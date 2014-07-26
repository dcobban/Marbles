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
	typedef std::vector<std::thread>			thread_list;

	implementation()
	: _active_service(&do_nothing<shared_service>)
	, _next_service(0)
	, _run_result(0)
	{
		_choose_service._fn = std::make_shared<task::fn>([]() { application::get()->choose_service(); });
	}

	~implementation()
	{
	}

	template<typename T> static void do_nothing(T*) {};

	shared_mutex			_service_mutex;
	service_list			_services;
	thread_list				_threads;

	task					_choose_service;
	ActiveService			_active_service;
	std::atomic<unsigned>	_next_service;
	int						_run_result;

	static ActiveApplication sApplication;
};

// --------------------------------------------------------------------------------------------------------------------
void application::process_services()
{
	shared_service choosen;
	application::implementation::sApplication.reset(this);
	_implementation->_active_service.reset(&choosen);
	choose_service();
	while(choosen) 
	{
		ASSERT(service::queued != choosen->state());
		ASSERT(!choosen->_taskQueue.empty());
		choosen->_taskQueue.pop().run();
	}
	_implementation->_active_service.reset();
}

// --------------------------------------------------------------------------------------------------------------------
void application::choose_service()
{
	shared_service& active = *_implementation->_active_service;
	if (active)
	{	// Queue current service
		service::execution_state state = active->_state.load();
		active->_state = service::queued;
		if (service::stopped == state)
		{	// Stopped _services require thier task queue to be emptied
			active->_taskQueue.clear();
			active->_state = service::stopped;
		}
		active.reset();
	}

	// Select the next service
	active = select_service();
	if (active)
	{
		active->post(_implementation->_choose_service); // Schedule the next attempt to switch _services
	}
}

// --------------------------------------------------------------------------------------------------------------------
shared_service application::select_service()
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

// --------------------------------------------------------------------------------------------------------------------
int application::run(unsigned nu_threads)
{
	bool isRunning = NULL != implementation::sApplication.get();
	if (isRunning || _implementation->_services.empty())
	{
		return -1;
	}

	if (0 >= nu_threads)
	{
		nu_threads = num_hardware_threads();
	}


	// The _services to initialize first are the _kernels.
	_implementation->_next_service = 0; // or random

	implementation::sApplication.reset(this);
	{	// Initialize threads
		shared_service primary = _implementation->_services.front().lock();
		_implementation->_threads.resize(nu_threads - 1);
		for(int i = _implementation->_threads.size(); i--; )
		{
			auto thread_start = [this, i]() 
			{
				std::thread worker([this](){ application::process_services(); });
				this->_implementation->_threads[i].swap(worker);
			};
			primary->post(thread_start);
		}
	}

	application::process_services();

	for(unsigned i = 0; i < _implementation->_threads.size(); ++i)
	{
		_implementation->_threads[i].join();
	}
	_implementation->_threads.clear();
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
shared_service application::active_service() const
{
	return NULL == _implementation.get() ? shared_service() : *_implementation->_active_service;
}

// --------------------------------------------------------------------------------------------------------------------
shared_service application::create_service()
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
