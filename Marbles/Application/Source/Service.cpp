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

#include <application\service.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
service::service()
: _state(service::uninitialized)
{
}

// --------------------------------------------------------------------------------------------------------------------
service::execution_state service::state() const
{
	return _state.load();
}

// --------------------------------------------------------------------------------------------------------------------
void service::stop()
{
	shared_service self = _self.lock();
	post([self](){ application::get()->unregister(self); });
}

// --------------------------------------------------------------------------------------------------------------------
//bool service::wait(execution_state state)
//{
//	const bool isSelf = active().get() == this;
//	ASSERT(!isSelf); // We cannot wait for ourself!
//	if (!isSelf)
//	{
//		mutex_lock lock(_stateMutex);
//		while (state > _state.get())
//		{
//			_stateChanged.wait(lock);
//		}
//	}
//	return !isSelf;
//}

// --------------------------------------------------------------------------------------------------------------------
bool service::post(task action)
{
	return _tasks.try_push(action);
}

// --------------------------------------------------------------------------------------------------------------------
size_t service::count()
{
	return application::get()->service_count();
}

// --------------------------------------------------------------------------------------------------------------------
shared_service service::at(unsigned int i)
{
	return application::get()->service_at(i);
}

// --------------------------------------------------------------------------------------------------------------------
shared_service service::active()
{
	return application::get()->active_service();
}

// --------------------------------------------------------------------------------------------------------------------
shared_service service::create()
{
	shared_service service(new service()); // would like to use make_shared<service>() but service::service() is private
	service->_self = service;
	return std::move(service);
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
