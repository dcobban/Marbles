// This source file is part of marbles library.
//
// Copyright (c) 2012 Dan Cobban
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// --------------------------------------------------------------------------------------------------------------------

#include <application\application.h>
#include <application\service.h>
#include <common\common.h>
#include <boost\any.hpp>
#include <boost\thread.hpp>
#include <chrono>
#include <thread>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
struct application::implementation
{
	typedef boost::shared_mutex					shared_mutex;
	typedef boost::unique_lock<shared_mutex>	unique_lock;
	typedef boost::shared_lock<shared_mutex>	shared_lock;
	typedef boost::thread_specific_ptr<shared_service> ActiveService;
	typedef boost::thread_specific_ptr<application> ActiveApplication;
	typedef std::vector<weak_service>			service_list;
	typedef std::vector<shared_service>			kernel_list;

	implementation()
	: _active_service(&do_nothing<shared_service>)
	, _next_service(0)
	, _run_result(0)
	{
	}

	~implementation()
	{
	}

	template<typename T> static void do_nothing(T*) {};

	shared_mutex			_kernel_mutex;
	kernel_list				_kernels;

	shared_mutex			_service_mutex;
	service_list			_services;

	ActiveService			_active_service;
	std::atomic<unsigned>	_next_service;
	int						_run_result;

	static ActiveApplication sApplication;
};

// --------------------------------------------------------------------------------------------------------------------
struct application::kernel
{
	std::thread		_thread;
	shared_service	_active_service;
	shared_service	_next_service;
	size_t			_id;

	task			_choose_next_service;

	kernel();
	~kernel();

	inline bool running() const { return NULL != _active_service; }

	void Main(application* application);
	void start_thread();
	void stop_thread();
	void choose_service();
	shared_service select_service();
};

// --------------------------------------------------------------------------------------------------------------------
application::kernel::kernel()
{
	_choose_next_service._fn = std::make_shared<task::fn>();
	if (_choose_next_service._fn)
	{
		*_choose_next_service._fn = [this]() { this->choose_service(); };
	}
	_active_service.reset();
}

// --------------------------------------------------------------------------------------------------------------------
application::kernel::~kernel()
{
	stop_thread();
	if (_thread.joinable())
	{
		_thread.join();
	}
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::Main(application* application)
{
	application::implementation::sApplication.reset(application);
	application->_implementation->_active_service.reset(&_active_service);
	ASSERT(!_active_service);
	choose_service();
	while(_active_service) 
	{
		ASSERT(service::queued != _active_service->state());
		ASSERT(!_active_service->_taskQueue.empty());
		task run = _active_service->_taskQueue.pop();
		run();
	}
	_active_service.reset();
	application->_implementation->_active_service.reset();
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::start_thread()
{	// What if we are already running?
	application* application = application::get();
	std::thread kernel([=]() { this->Main(application); });
	_thread.swap(kernel);
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::stop_thread()
{	// Without an active service the _thread will exit
	_active_service.reset();
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::choose_service()
{
	shared_service selected;
	if (_active_service)
	{	// Queue current service
		service::execution_state state = _active_service->_state.load();
		_active_service->_state = service::queued;
		if (service::stopped == state)
		{	// Stopped _services require thier task queue to be emptied
			_active_service->_taskQueue.clear();
			_active_service->_state = service::stopped;
		}
		_active_service.reset();
	}

	// Select the next service
	if (_next_service)
	{
		_next_service->_state = service::running;
		_active_service = _next_service;
		_next_service.reset();
	}
	else
	{
		_active_service = select_service();
	}

	if (_active_service)
	{
		_active_service->post(_choose_next_service); // Schedule the next attempt to switch _services
	}
}

// --------------------------------------------------------------------------------------------------------------------
shared_service application::kernel::select_service()
{	
	shared_service candidate;
	application::implementation* const application = application::get()->_implementation.get();
	unsigned int start = application->_next_service.load();
	bool isSelectionPossible = false;
	bool selected = false;

	implementation::shared_lock lock(application->_service_mutex);
	const size_t size = application->_services.size();
	if (0 != size)
	{
		do
		{
			unsigned int next = 0;
			unsigned int index = 0;
			do 
			{	
				index = application->_next_service.load();
				next = (index + 1) % size;
				ASSERT(index != next); // Ensures that 2 <= mServiceList.size()
				// try again if another _thread has changed the value before me 
			} while (!application->_next_service.compare_exchange_strong(index, next));

			service::execution_state state = service::queued;
			candidate = application->_services[index].lock();
			if (!candidate || !candidate->_state.compare_exchange_strong(state, service::running))
			{
				candidate.reset();
			}

			// If we have looped over all _services and their is no possible selection then exit loop.
			const bool accepted = NULL != candidate;
			const bool atEnd	= next == start;
			const bool yield	= atEnd & isSelectionPossible;
			const bool exit		= atEnd & !isSelectionPossible;
			isSelectionPossible = (atEnd & accepted) | (!atEnd & (isSelectionPossible | accepted));
			selected = accepted | exit;
			if (yield)
			{
				application::yield();
			}
			// If the service is valid and is being processed then pick another.
		} while(!selected);
	}
	ASSERT(!candidate || service::queued != candidate->_state.load());
	return candidate;
}

// --------------------------------------------------------------------------------------------------------------------
application::implementation::ActiveApplication application::implementation::sApplication(&application::implementation::do_nothing<application>);

// --------------------------------------------------------------------------------------------------------------------
application::application()
: _implementation(new implementation())
{
}

// --------------------------------------------------------------------------------------------------------------------
application::~application()
{
}

// --------------------------------------------------------------------------------------------------------------------
void application::stop(int run_result)
{
	_implementation->_run_result = run_result;

	{
		implementation::shared_lock lock(_implementation->_service_mutex);
		for(implementation::service_list::iterator i = _implementation->_services.begin(); 
			i != _implementation->_services.end(); 
			++i)
		{
			shared_service service = (*i).lock();
			if (service)
			{
				service->stop();
			}
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------
int application::run(unsigned nu_threads)
{
	bool isRunning = NULL != implementation::sApplication.get();
	if (isRunning)
	{
		return -1;
	}

	implementation::sApplication.reset(this);

	if (0 >= nu_threads)
	{
		nu_threads = num_hardware_threads();
	}

	// The _services to initialize first are the _kernels.
	_implementation->_next_service = _implementation->_services.size();

	{	// create the requested kernel
		application::implementation::unique_lock lock(_implementation->_kernel_mutex);
		_implementation->_kernels.reserve(nu_threads);
		for(int i = nu_threads; i--; )
		{
			shared_service service = service::create();
			_register(service);
			service->make_provider<kernel>();
			application::kernel* provider = service->provider<kernel>();
			if (!_implementation->_kernels.empty())
			{	// Launch a _thread for all _kernels except the current _thread.
				service->post(std::bind<void>(&application::kernel::start_thread, provider));
			}
			provider->_id = _implementation->_kernels.size();
			_implementation->_kernels.push_back(service);
		}
	}

	shared_service primary = _implementation->_services[_implementation->_next_service.load()].lock();
	primary->provider<kernel>()->Main(this);

	for(implementation::kernel_list::iterator service = _implementation->_kernels.begin();
		service != _implementation->_kernels.end();
		++service)
	{
		if (primary != (*service))
		{
			kernel* provider = (*service)->provider<kernel>();
			if (provider->_thread.joinable())
			{
				provider->_thread.join();
			}
		}
	}
	_implementation->_kernels.clear();
	_implementation->_services.clear();
	
	implementation::sApplication.reset();
	return _implementation->_run_result;
}

// --------------------------------------------------------------------------------------------------------------------
unsigned application::num_hardware_threads()
{
	return std::thread::hardware_concurrency();
}

// --------------------------------------------------------------------------------------------------------------------
void application::sleep(int milliseconds)
{
	std::chrono::milliseconds duration(milliseconds);
	std::this_thread::sleep_for(duration);
}

// --------------------------------------------------------------------------------------------------------------------
void application::yield()
{
	std::this_thread::yield();
}

// --------------------------------------------------------------------------------------------------------------------
shared_service application::activeService() const
{
	return NULL == _implementation.get() ? shared_service() : *_implementation->_active_service;
}

// --------------------------------------------------------------------------------------------------------------------
shared_service application::createService()
{
	shared_service service = service::create();
	_register(service);
	return std::move(service);
}

// --------------------------------------------------------------------------------------------------------------------
void application::_register(const shared_service& service)
{
	weak_service serviceItem;
	{	// If something has expired then reuse the slot
		implementation::shared_lock lock(_implementation->_service_mutex);
		implementation::service_list::iterator item = _implementation->_services.begin();
		while(item != _implementation->_services.end() && !item->expired())
		{
			++item;
		}
		if (item != _implementation->_services.end())
		{
			*item = serviceItem = service;
		}
	}

	if (serviceItem.expired())
	{
		implementation::unique_lock lock(_implementation->_service_mutex);
		_implementation->_services.erase(
			std::remove_if(	_implementation->_services.begin(), 
							_implementation->_services.end(), 
							expired<marbles::service>()), 
			_implementation->_services.end());
		_implementation->_services.push_back(service);
	}

	service->_state = service::queued;
}

// --------------------------------------------------------------------------------------------------------------------
void application::unregister(const shared_service& service)
{
	service::execution_state service_state = service->_state.load();
	if (service_state != service::stopped)
	{
		implementation::shared_lock lock(_implementation->_service_mutex);
		implementation::service_list::iterator item = _implementation->_services.begin();
		while(item != _implementation->_services.end() && item->lock() != service)
		{
			++item;
		}
		if (item != _implementation->_services.end())
		{	
			item->reset(); // Item is no longer a candidate
		}

		service->_state = service::stopped;
	}
}

// --------------------------------------------------------------------------------------------------------------------
application* application::get() 
{ 
	application* app = implementation::sApplication.get();
	ASSERT(app || !"You must call application::run before calling this function."); 
	return app; 
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
