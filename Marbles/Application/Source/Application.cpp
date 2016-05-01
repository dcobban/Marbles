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
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
struct application::implementation
{
	typedef std::shared_mutex				shared_mutex;
	typedef std::unique_lock<shared_mutex>	unique_lock;
	typedef std::shared_lock<shared_mutex>	shared_lock;
	typedef shared_service*					ActiveService;
	typedef application*					ActiveApplication;
	typedef std::vector<weak_service>		service_list;
	typedef std::vector<std::thread>		thread_list;

	implementation()
	: _next_service(0)
	, _run_result(0)
	{}

	template<typename T> static void        do_nothing(T*) {};

	shared_mutex                            _service_mutex;
	service_list                            _services;
	thread_list                             _threads;

	shared_task                             _choose_service;
	std::atomic<unsigned>                   _next_service;
	int                                     _run_result;

	thread_local static ActiveApplication	sApplication;
	thread_local static ActiveService		sActiveService;
};

// --------------------------------------------------------------------------------------------------------------------
thread_local application::implementation::ActiveApplication application::implementation::sApplication = NULL;
thread_local application::implementation::ActiveService application::implementation::sActiveService;

// --------------------------------------------------------------------------------------------------------------------
application::application()
: _implementation(std::make_unique<implementation>())
{
	if (_implementation)
	{
		_implementation->_choose_service = std::make_shared<task>([this]() { this->choose_service(); });
	}
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
	bool isRunning = NULL != implementation::sApplication;
	if (isRunning || _implementation->_services.empty())
	{
		return -1;
	}

	if (0 >= nu_threads)
	{
		nu_threads = num_hardware_threads();
	}

	_implementation->_next_service = 0; // or random
	{	// Initialize threads
		shared_service primary = _implementation->_services.front().lock();
		_implementation->_threads.resize(nu_threads - 1);
		for(size_t i = _implementation->_threads.size(); i--; )
		{
			auto action = std::make_shared<task>([this, i]()
			{
				std::thread worker([this](){ application::process_services(); });
				this->_implementation->_threads[i].swap(worker);
			});
			primary->post(action);
		}
	}

	application::process_services();

	for(unsigned i = 0; i < _implementation->_threads.size(); ++i)
	{
		_implementation->_threads[i].join();
	}
	_implementation->_threads.clear();
	_implementation->_services.clear();

	return _implementation->_run_result;
}

// --------------------------------------------------------------------------------------------------------------------
//bool application::post(const task& action)
//{
//	return post(std::make_shared<task>(action));
//}

// --------------------------------------------------------------------------------------------------------------------
bool application::post(const shared_task& action)
{
	unsigned int next = _implementation->_next_service.load();
	shared_service next_service = _implementation->_services[next].lock();
	return next_service && next_service->post(action);
}

// --------------------------------------------------------------------------------------------------------------------
application* application::get() 
{ 
	application* app = implementation::sApplication;
	ASSERT(app || !"You must call application::run before calling this std::function."); 
	return app; 
}

// --------------------------------------------------------------------------------------------------------------------
void application::yield()
{
	std::this_thread::yield();
}

// --------------------------------------------------------------------------------------------------------------------
void application::sleep(int milliseconds)
{
	std::chrono::milliseconds duration(milliseconds);
	std::this_thread::sleep_for(duration);
}

// --------------------------------------------------------------------------------------------------------------------
unsigned application::num_hardware_threads()
{
	return std::thread::hardware_concurrency();
}

// --------------------------------------------------------------------------------------------------------------------
size_t application::service_count() const
{
	implementation::shared_lock lock(_implementation->_service_mutex);
	return _implementation->_services.size();
}

// --------------------------------------------------------------------------------------------------------------------
shared_service application::service_at(size_t index)
{
	implementation::shared_lock lock(_implementation->_service_mutex);
	return _implementation->_services[index].lock();
}

// --------------------------------------------------------------------------------------------------------------------
shared_service application::active_service() const
{
	return *implementation::sActiveService;
}

// --------------------------------------------------------------------------------------------------------------------
shared_service application::create_service()
{
	shared_service service = service::create();
	_register(service);
	return std::move(service);
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
			const bool yield	= atEnd && isSelectionPossible;
			const bool exit		= atEnd && !isSelectionPossible;
			isSelectionPossible = (atEnd && accepted) || (!atEnd && (isSelectionPossible || accepted));
			selected = accepted || exit;
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
void application::_register(const shared_service& service)
{
	auto is_expired = [](const weak_service& srv)
	{
		return srv.expired();
	};
	implementation::service_list::iterator item;
	{
		implementation::shared_lock lock(_implementation->_service_mutex);
		item = remove_if(	_implementation->_services.begin(), 
								_implementation->_services.end(), 
								is_expired);
	}

	// Cleanup stopped services and push new service
	implementation::unique_lock lock(_implementation->_service_mutex);
	_implementation->_services.erase(item, _implementation->_services.end());
	_implementation->_services.push_back(service);
	service->_state = service::queued;
}

// --------------------------------------------------------------------------------------------------------------------
void application::unregister(const shared_service& service)
{
	service::execution_state service_state = service->_state.load();
	if (service_state != service::stopped)
	{
		implementation::shared_lock lock(_implementation->_service_mutex);
		auto item = find_if(_implementation->_services.begin(),
							_implementation->_services.end(),
							[&service](const weak_service& srv) 
		{
			return srv.lock() == service;
		});
		if (item != _implementation->_services.end())
		{	
			item->reset(); // Item is no longer a candidate
		}
		
		service->_state = service::stopped;
		service->_tasks.clear(); 
		service->post(_implementation->_choose_service);
	}
}

// --------------------------------------------------------------------------------------------------------------------
void application::process_services()
{
	shared_service choosen;
	implementation::sActiveService = &choosen;
	implementation::sApplication = this;
	choose_service();
	while(choosen) 
	{
		ASSERT(service::queued != choosen->state());
		ASSERT(!choosen->_tasks.empty());
		(*choosen->_tasks.pop())();
	}
	implementation::sActiveService->reset();
	implementation::sApplication = NULL;
}

// --------------------------------------------------------------------------------------------------------------------
void application::choose_service()
{
	shared_service& active = *implementation::sActiveService;
	if (active)
	{	// Requeue any service that has not been stopped
		service::execution_state state = active->_state.load();
		active->_state = service::stopped == state ? state : service::queued;
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
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
