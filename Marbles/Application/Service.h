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

#pragma once

#include <Common/AtomicBuffer.h>
#include <Common/Common.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
class service;
typedef future<int> task;
typedef shared_ptr<task> shared_task;
typedef weak_ptr<task> weak_task;
typedef shared_ptr<service> shared_service;
typedef weak_ptr<service> weak_service;

// --------------------------------------------------------------------------------------------------------------------
class service
{
public:
	enum execution_state
	{
		uninitialized = -1,
		startup,
		queued,
		running,
		stopped,
	};

	execution_state			state() const;
	bool					hasStopped() const;
	void					stop();
	//bool					wait(float timeout = infinity);
	//bool					wait(execution_state state/*, float timeout = infinity*/);

	template<typename T>
	T*						provider();
    template< class Function, class... Args>
    bool					post(Function&& f, Args&&... args);

	bool					operator==(const service& rhs);

							~service() {}

	// service API
	static size_t			count();
	static shared_service	at(unsigned int i);
	static shared_service	active();
private:
	friend class application;
	typedef shared_ptr<void> shared_provider;
	typedef weak_ptr<void> weak_provider;

							explicit service();

	template<typename T, typename... Args>
	void					make_provider(Args&&... args);
	static shared_service	create();

	typedef atomic_buffer<task, 128>	task_queue;

	task_queue				_tasks;
	atomic<execution_state>	_state;
	shared_provider			_provider;
	weak_service			_self;
};

// --------------------------------------------------------------------------------------------------------------------
inline bool service::operator==(const service& rhs)
{
	return this == &rhs;
}

// --------------------------------------------------------------------------------------------------------------------
inline bool	service::hasStopped() const
{
	return stopped == _state.load();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
inline T* service::provider()
{
	return static_pointer_cast<T>(_provider).get();
}

// --------------------------------------------------------------------------------------------------------------------
template< class Function, class... Args>
inline bool service::post(Function&& f, Args&&... args)
{
    return _tasks.try_push(async(launch::deferred, 
                                 [f](Args&&... fargs) { f(forward<Args>(fargs)...); return 0; }, 
                                 forward<Args>(args)...));
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T, typename... Args>
inline void service::make_provider(Args&&... args)
{
	_provider = static_pointer_cast<void>(make_shared<T>(forward<Args>(args)...));
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
