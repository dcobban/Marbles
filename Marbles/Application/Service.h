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

#include <application/task.h>
#include <Common/CircularBuffer.h>
#include <Common/Common.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/any.hpp>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

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
		// ShutdownPending,
		shutdown,
		stopped,
	};
	execution_state			state() const;
	bool					hasStopped() const;
	void					stop(bool block = false);
	//bool					wait(float timeout = infinity);
	//bool					wait(execution_state state/*, float timeout = infinity*/);

	template<typename T>
	T*						provider();
	template<typename Fn>
	bool					post(Fn fn);
	// bool					post(task::fn& fn);
	bool					post(shared_task task);

	bool					operator==(const service& rhs);

	~service() {}

	// service API
	static size_t			count();
	static shared_service	at(unsigned int i);
	static shared_service	active();
private:
	friend class application;
	typedef std::shared_ptr<boost::any> shared_provider;
	typedef std::weak_ptr<boost::any> weak_provider;

							service();

	static shared_service	create();
	template<typename fn>
	void					make_provider();
	template<typename fn, typename A0>
	void					make_provider(A0& a0);
	template<typename fn, typename A0, typename A1>
	void					make_provider(A0& a0, A1& a1);
	template<typename fn, typename A0, typename A1, typename A2>
	void					make_provider(A0& a0, A1& a1, A2& a2);
	template<typename fn, typename A0, typename A1, typename A2, typename A3>
	void					make_provider(A0& a0, A1& a1, A2& a2, A3& a3);
	template<typename fn, typename A0, typename A1, typename A2, typename A3, typename A4>
	void					make_provider(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4);
	template<typename fn, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
	void					make_provider(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5);

	typedef CircularBuffer<shared_task, 128> task_queue;
	typedef boost::mutex					mutex;
	typedef boost::unique_lock<mutex>		mutex_lock;
	typedef boost::condition_variable		condition;

	task_queue				_taskQueue;

	atomic<execution_state>	_state;
	// mutable mutex			_stateMutex; // We can remove this by not allowing resizing  
	// mutable condition		_stateChanged;

	boost::any				_provider;
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
	return stopped == _state.get();
}

//// --------------------------------------------------------------------------------------------------------------------
//inline void	service::SetState(execution_state state)
//{
//	_state = state;
//	_stateChanged.notify_all();
//}

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
T* service::provider()
{
	return boost::any_cast< std::shared_ptr<T> >(_provider).get();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename FN> 
bool service::post(FN taskFn)
{
	return post(std::make_shared< task >(taskFn));
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn> 
void service::make_provider()
{
	_provider = std::make_shared<fn>();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0> 
void service::make_provider(A0& a0)
{
	_provider = std::make_shared<fn>(a0);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0, typename A1> 
void service::make_provider(A0& a0, A1& a1)
{
	_provider = std::make_shared<fn>(a0, a1);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0, typename A1, typename A2> 
void service::make_provider(A0& a0, A1& a1, A2& a2)
{
	_provider = std::make_shared<fn>(a0, a1, a2);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0, typename A1, typename A2, typename A3> 
void service::make_provider(A0& a0, A1& a1, A2& a2, A3& a3)
{
	_provider = std::make_shared<fn>(a0, a1, a2, a3);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0, typename A1, typename A2, typename A3, typename A4> 
void service::make_provider(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4)
{
	_provider = std::make_shared<fn>(a0, a1, a2, a3, a4);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5> 
void service::make_provider(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
{
	_provider = std::make_shared<fn>(a0, a1, a2, a3, a4, a5);
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
