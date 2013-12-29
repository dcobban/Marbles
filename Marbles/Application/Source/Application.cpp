// This source file is part of Marbles library.
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
#include <boost\any.hpp>
#include <boost\thread.hpp>

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
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
	{}

	~implementation()
	{
	}

	shared_mutex				kernelMutex;
	kernel_list				kernels;

	shared_mutex				service_mutex;
	service_list			services;

	ActiveService			activeService;
	atomic<unsigned int>	next_service;
	int						run_result;

	static ActiveApplication sApplication;
};

// --------------------------------------------------------------------------------------------------------------------
struct application::kernel
{
	boost::thread	mThread;
	shared_service	mActive;
	shared_service	mNext;
	size_t			mId;

	task::fn		mChooseService;

	kernel();
	~kernel();

	inline bool running() const { return mActive; }

	void Main(application* application);
	void startThread();
	void stopThread();
	void chooseService();
	shared_service selectService();
};

// --------------------------------------------------------------------------------------------------------------------
application::kernel::kernel()
{
	mChooseService = std::bind<void>(&application::kernel::chooseService, this);
	mActive.reset();
}

// --------------------------------------------------------------------------------------------------------------------
application::kernel::~kernel()
{
	stopThread();
	if (mThread.joinable())
	{
		mThread.join();
	}
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::Main(application* application)
{
	application::implementation::sApplication.reset(application);
	application->_implementation->activeService.reset(&mActive);
	ASSERT(!mActive);
	chooseService();
	while(mActive && !mActive->_taskQueue.empty()) 
	{
		ASSERT(service::queued != mActive->state());
		shared_task task = mActive->_taskQueue.pop();
		task->task_fn();
	}
	mActive.reset();
	application->_implementation->activeService.reset();
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::startThread()
{	// What if we are already running?
	application* application = application::get();
	boost::thread kernel(std::bind(&application::kernel::Main, this, application));
	mThread.swap(kernel);
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::stopThread()
{	// Without an active service the thread will exit
	mActive.reset();
}

// --------------------------------------------------------------------------------------------------------------------
void application::kernel::chooseService()
{
	shared_service selected;
	if (mActive)
	{	// Queue current service
		service::execution_state state = mActive->_state.get();
		mActive->_state = service::queued;
		if (service::stopped == state)
		{	// Stopped services require thier task queue to be emptied
			mActive->_taskQueue.clear();
			mActive->_state = service::stopped;
		}
		mActive.reset();
	}

	// Select the next service
	if (mNext)
	{
		mNext->_state = service::running;
		mActive = mNext;
		mNext.reset();
	}
	else
	{
		mActive = selectService();
	}

	if (mActive)
	{
		mActive->post(mChooseService); // Schedule the next attempt to switch services
	}
}

// --------------------------------------------------------------------------------------------------------------------
shared_service application::kernel::selectService()
{	
	shared_service candidate;
	application::implementation* const application = application::get()->_implementation.get();
	unsigned int start = application->next_service.get();
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
				index = application->next_service.get();
				next = (index + 1) % size;
				ASSERT(index != next); // Ensures that 2 <= mServiceList.size()
				// try again if another thread has changed the value before me 
			} while(index != application->next_service.compare_exchange(next, index));

			candidate = application->services[index].lock();
			if (!candidate || service::queued != candidate->_state.compare_exchange(service::running, service::queued))
			{
				candidate.reset();
			}

			// If we have looped over all services and their is no possible selection then exit loop.
			const bool accepted = candidate;
			const bool atEnd	= next == start;
			const bool sleep	= atEnd & isSelectionPossible;
			const bool exit		= atEnd & !isSelectionPossible;
			isSelectionPossible = (atEnd & accepted) | (!atEnd & (isSelectionPossible | accepted));
			selected = accepted | exit;
			if (sleep)
			{
				application::sleep(1);
			}
			// If the service is valid and is being processed then pick another.
		} while(!selected);
	}
	ASSERT(!candidate || service::queued != candidate->_state.get());
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
int application::run(unsigned numThreads)
{
	bool isRunning = NULL != implementation::sApplication.get();
	if (isRunning)
	{
		return -1;
	}

	implementation::sApplication.reset(this);

	if (0 >= numThreads)
	{
		numThreads = boost::thread::hardware_concurrency();
	}

	// The services to initialize first are the kernels.
	_implementation->next_service = _implementation->services.size();

	{	// create the requested kernel
		application::implementation::unique_lock lock(_implementation->kernelMutex);
		_implementation->kernels.reserve(numThreads);
		for(int i = numThreads; i--; )
		{
			shared_service service = service::create();
			_register(service);
			service->make_provider<kernel>();
			application::kernel* provider = service->provider<kernel>();
			if (!_implementation->kernels.empty())
			{	// Launch a thread for all kernels except the current thread.
				service->post(std::bind<void>(&application::kernel::startThread, provider));
			}
			provider->mId = _implementation->kernels.size();
			_implementation->kernels.push_back(service);
		}
	}

	shared_service primary = _implementation->services[_implementation->next_service.get()].lock();
	primary->provider<kernel>()->Main(this);

	for(implementation::kernel_list::iterator service = _implementation->kernels.begin();
		service != _implementation->kernels.end();
		++service)
	{
		if (primary != (*service))
		{
			kernel* provider = (*service)->provider<kernel>();
			if (provider->mThread.joinable())
			{
				provider->mThread.join();
			}
		}
	}
	_implementation->kernels.clear();
	_implementation->services.clear();
	
	implementation::sApplication.reset();
	return _implementation->run_result;
}

// --------------------------------------------------------------------------------------------------------------------
unsigned application::num_hardware_threads()
{
	return boost::thread::hardware_concurrency();
}

// --------------------------------------------------------------------------------------------------------------------
void application::sleep(int milliseconds)
{
	boost::chrono::milliseconds duration(milliseconds);
	boost::this_thread::sleep_for(duration);
}

// --------------------------------------------------------------------------------------------------------------------
shared_service application::activeService() const
{
	return NULL == _implementation.get() ? shared_service() : *_implementation->activeService;
}

// --------------------------------------------------------------------------------------------------------------------
void application::_register(shared_service service)
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
							expired<Marbles::service>()), 
			_implementation->services.end());
		_implementation->services.push_back(service);
	}

	service->_state = service::queued;
}

// --------------------------------------------------------------------------------------------------------------------
void application::unregister(shared_service service)
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

// --------------------------------------------------------------------------------------------------------------------
application* application::get() 
{ 
	application* app = implementation::sApplication.get();
	ASSERT(app || !"You must call application::run before calling this function."); 
	return app; 
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
