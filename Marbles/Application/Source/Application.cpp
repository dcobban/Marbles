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
#include <chrono>
#include <boost\thread.hpp>
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
	: activeService(&Empty<shared_service>)
	, next_service(0)
	, run_result(0)
	{
		unregister_task._fn = std::make_shared<task::fn>(&unregister);
	}

	~implementation()
	{
	}

	shared_mutex			kernelMutex;
	kernel_list				kernels;

	shared_mutex			service_mutex;
	service_list			services;

	ActiveService			activeService;
	std::atomic<unsigned>	next_service;
	int						run_result;

	static void				unregister(const task::shared_param& param) { sApplication->unregister(*reinterpret_cast<shared_service*>(param.get())); };
	task					unregister_task;
	static ActiveApplication sApplication;
};

// --------------------------------------------------------------------------------------------------------------------
struct application::kernel
{
	std::thread		thread;
	shared_service	activeService;
	shared_service	nextService;
	size_t			id;

	task			chooseNextService;

	kernel();
	~kernel();

	inline bool running() const { return NULL != activeService; }

	void Main(application* application);
	void startThread();
	void stopThread();
	void chooseService();
	shared_service selectService();
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T, typename R, R (T::*member)()>
void VoidMember(T* self, const task::shared_param&)
{
	if (self)
	{
		(self->*member)();
	}
}

// --------------------------------------------------------------------------------------------------------------------
application::kernel::kernel()
{
	chooseNextService._fn = std::make_shared<task::fn>();
	if (chooseNextService._fn)
	{
		*chooseNextService._fn = std::bind<void>(&VoidMember<application::kernel, void, &application::kernel::chooseService>, this, std::_1);
	}
	activeService.reset();
}

// --------------------------------------------------------------------------------------------------------------------
application::kernel::~kernel()
{
	stopThread();
	if (thread.joinable())
	{
		thread.join();
	}
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::Main(application* application)
{
	application::implementation::sApplication.reset(application);
	application->_implementation->activeService.reset(&activeService);
	ASSERT(!activeService);
	chooseService();
	while(activeService && !activeService->_taskQueue.empty()) 
	{
		ASSERT(service::queued != activeService->state());
		task run = activeService->_taskQueue.pop();
		run();
	}
	activeService.reset();
	application->_implementation->activeService.reset();
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::startThread()
{	// What if we are already running?
	application* application = application::get();
	std::thread kernel(std::bind(&application::kernel::Main, this, application));
	thread.swap(kernel);
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::stopThread()
{	// Without an active service the thread will exit
	activeService.reset();
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::chooseService()
{
	shared_service selected;
	if (activeService)
	{	// Queue current service
		service::execution_state state = activeService->_state.load();
		activeService->_state = service::queued;
		if (service::stopped == state)
		{	// Stopped services require thier task queue to be emptied
			activeService->_taskQueue.clear();
			activeService->_state = service::stopped;
		}
		activeService.reset();
	}

	// Select the next service
	if (nextService)
	{
		nextService->_state = service::running;
		activeService = nextService;
		nextService.reset();
	}
	else
	{
		activeService = selectService();
	}

	if (activeService)
	{
		activeService->post(chooseNextService); // Schedule the next attempt to switch services
	}
}

// --------------------------------------------------------------------------------------------------------------------
shared_service application::kernel::selectService()
{	
	shared_service candidate;
	application::implementation* const application = application::get()->_implementation.get();
	unsigned int start = application->next_service.load();
	bool isSelectionPossible = false;
	bool selected = false;

	implementation::shared_lock lock(application->service_mutex);
	const size_t size = application->services.size();
	if (0 != size)
	{
		do
		{
			unsigned int next = 0;
			unsigned int index = 0;
			do 
			{	
				index = application->next_service.load();
				next = (index + 1) % size;
				ASSERT(index != next); // Ensures that 2 <= mServiceList.size()
				// try again if another thread has changed the value before me 
			} while (!application->next_service.compare_exchange_strong(index, next));

			service::execution_state state = service::queued;
			candidate = application->services[index].lock();
			if (!candidate || !candidate->_state.compare_exchange_strong(state, service::running))
			{
				candidate.reset();
			}

			// If we have looped over all services and their is no possible selection then exit loop.
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
application::implementation::ActiveApplication application::implementation::sApplication(&Empty<application>);

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
	_implementation->run_result = run_result;

	{
		implementation::shared_lock lock(_implementation->service_mutex);
		for(implementation::service_list::iterator i = _implementation->services.begin(); 
			i != _implementation->services.end(); 
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
int application::run(unsigned nuthreads)
{
	bool isRunning = NULL != implementation::sApplication.get();
	if (isRunning)
	{
		return -1;
	}

	implementation::sApplication.reset(this);

	if (0 >= nuthreads)
	{
		nuthreads = boost::thread::hardware_concurrency();
	}

	// The services to initialize first are the kernels.
	_implementation->next_service = _implementation->services.size();

	{	// create the requested kernel
		application::implementation::unique_lock lock(_implementation->kernelMutex);
		_implementation->kernels.reserve(nuthreads);
		for(int i = nuthreads; i--; )
		{
			shared_service service = service::create();
			_register(service);
			service->make_provider<kernel>();
			application::kernel* provider = service->provider<kernel>();
			if (!_implementation->kernels.empty())
			{	// Launch a thread for all kernels except the current thread.
				service->post(std::bind<void>(&application::kernel::startThread, provider));
			}
			provider->id = _implementation->kernels.size();
			_implementation->kernels.push_back(service);
		}
	}

	shared_service primary = _implementation->services[_implementation->next_service.load()].lock();
	primary->provider<kernel>()->Main(this);

	for(implementation::kernel_list::iterator service = _implementation->kernels.begin();
		service != _implementation->kernels.end();
		++service)
	{
		if (primary != (*service))
		{
			kernel* provider = (*service)->provider<kernel>();
			if (provider->thread.joinable())
			{
				provider->thread.join();
			}
		}
	}
	_implementation->kernels.clear();
	_implementation->services.clear();
	
	implementation::sApplication.reset();
	return _implementation->run_result;
}

// --------------------------------------------------------------------------------------------------------------------
task application::task_unregister() const
{
	return _implementation->unregister_task;
}

// --------------------------------------------------------------------------------------------------------------------
unsigned application::num_hardware_threads()
{
	return boost::thread::hardware_concurrency();
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
	return NULL == _implementation.get() ? shared_service() : *_implementation->activeService;
}

// --------------------------------------------------------------------------------------------------------------------
void application::_register(const shared_service& service)
{
	weak_service serviceItem;
	{	// If something has expired then reuse the slot
		implementation::shared_lock lock(_implementation->service_mutex);
		implementation::service_list::iterator item = _implementation->services.begin();
		while(item != _implementation->services.end() && !item->expired())
		{
			++item;
		}
		if (item != _implementation->services.end())
		{
			*item = serviceItem = service;
		}
	}

	if (serviceItem.expired())
	{
		implementation::unique_lock lock(_implementation->service_mutex);
		_implementation->services.erase(
			std::remove_if(	_implementation->services.begin(), 
							_implementation->services.end(), 
							expired<marbles::service>()), 
			_implementation->services.end());
		_implementation->services.push_back(service);
	}

	service->_state = service::queued;
}

// --------------------------------------------------------------------------------------------------------------------
void application::unregister(const shared_service& service)
{
	service::execution_state service_state = service->_state.load();
	if (service_state != service::stopped)
	{
		implementation::shared_lock lock(_implementation->service_mutex);
		implementation::service_list::iterator item = _implementation->services.begin();
		while(item != _implementation->services.end() && item->lock() != service)
		{
			++item;
		}
		if (item != _implementation->services.end())
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
