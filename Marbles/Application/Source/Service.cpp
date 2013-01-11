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

#include <Application\Service.h>

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{

// --------------------------------------------------------------------------------------------------------------------
Service::Service()
: mState(Service::Uninitialized)
{
}

// --------------------------------------------------------------------------------------------------------------------
Service::ExecutionState Service::State() const
{
	return mState.get();
}

// --------------------------------------------------------------------------------------------------------------------
void Service::Stop(bool /*block*/)
{
	Post(std::bind<void>(&Service::AllStop, mSelf.lock()));
	//if (block)
	//{
	//	Wait(Stopped);
	//}
}

// --------------------------------------------------------------------------------------------------------------------
//bool Service::Wait(ExecutionState state)
//{
//	const bool isSelf = Active().get() == this;
//	ASSERT(!isSelf); // We cannot wait for ourself!
//	if (!isSelf)
//	{
//		MutexLock lock(mStateMutex);
//		while (state > mState.get())
//		{
//			mStateChanged.wait(lock);
//		}
//	}
//	return !isSelf;
//}

// --------------------------------------------------------------------------------------------------------------------
bool Service::Post(Task::Fn& fn)
{	
	shared_service service = mSelf.lock();
	shared_task task(new Task(fn, service));
	return Post(task);
}

// --------------------------------------------------------------------------------------------------------------------
bool Service::Post(shared_task task)
{
	return mTaskQueue.try_push(task);
}

// --------------------------------------------------------------------------------------------------------------------
void Service::AllStop()
{
	Application* application = Application::Get();
	Application::SharedLock lock(application->mServiceMutex);

	shared_service self = mSelf.lock();
	Application::ServiceList::iterator item = application->mServices.begin();
	while(item->lock() != self && item != application->mServices.end())
	{
		++item;
	}
	if (item != application->mServices.end())
	{	
		item->reset(); // Item is no longer a candidate
	}
	mState = Stopped;
}

// --------------------------------------------------------------------------------------------------------------------
shared_service Service::Active()
{
	return *Application::Get()->mActiveService;
}

// --------------------------------------------------------------------------------------------------------------------
shared_service Service::Create()
{
	shared_service service(new Service());
	service->mSelf = service;
	return service;
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
