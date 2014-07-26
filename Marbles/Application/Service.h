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

#include <memory>

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
	bool					post(task task);

	bool					operator==(const service& rhs);

	~service() {}

	// service API
	static size_t			count();
	static shared_service	at(unsigned int i);
	static shared_service	active();
private:
	friend class application;
//	template<> friend shared_service std::make_shared<service>();
	typedef std::shared_ptr<boost::any> shared_provider;
	typedef std::weak_ptr<boost::any> weak_provider;

							service();

	template<typename T, typename... ARGS>
	void					make_provider(ARGS&&... args);
	static shared_service	create();


	typedef CircularBuffer<task, 128>	task_queue;
	typedef boost::mutex				mutex;
	typedef boost::unique_lock<mutex>	mutex_lock;
	typedef boost::condition_variable	condition;

	task_queue						_taskQueue;

	std::atomic<execution_state>	_state;

	boost::any						_provider;
	weak_service					_self;
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
bool service::post(FN action_fn)
{
	task msg;
	msg._fn = std::make_shared<task::fn>(action_fn); // Need to remove this allocation
	return post(msg);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T, typename... ARGS>
inline void service::make_provider(ARGS&&... args)
{
	_provider = std::make_shared<T>(std::forward<ARGS>(args)...);
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
