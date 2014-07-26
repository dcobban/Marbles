// This source file is part of marbles library.
//
// Copyright (c) 2013 Dan Cobban
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

#include <application\task.h>
#include <application\service.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

template<typename sig>
class event;

// --------------------------------------------------------------------------------------------------------------------
template<typename _T>
class event_base
{
public:
	typedef typename _T				derived;
	typedef typename event_base<_T>	base;

	inline void clear()
	{
		m_multiCast.clear();
	}

	inline task operator+=(const task& task)
	{
		event_task local;
		local.action = task;
		local.service = marbles::service::active();
		m_multiCast.push_back(local);
		return task;
	}

	inline task operator-=(const task& task)
	{
		m_multiCast.erase(	std::remove(m_multiCast.begin(), 
										m_multiCast.end(), 
										task), 
							m_multiCast.end());
		return task;
	}

	inline void operator()()
	{
		m_multiCast.erase(	std::remove_if(	m_multiCast.begin(), 
											m_multiCast.end(), 
											PostMessage), 
							m_multiCast.end());
	}

	inline void operator()() const
	{
		std::for_each(m_multiCast.begin(), m_multiCast.end(), PostMessage);
	}

private:
	struct event_task
	{
		task action;
		weak_service service;
		const bool operator ==(const task& rhs)
		{
			return action == rhs;
		}
	};

	static const bool PostMessage(const event_task& task)
	{
		shared_service provider = task.service.lock();
		if (provider)
		{
			provider->post(task.action);
			return false;
		}

		return true;
	}

	std::vector<event_task> m_multiCast;
};

// --------------------------------------------------------------------------------------------------------------------
template<>
class event<void ()> : public event_base< event<void ()> >
{
public:
	task operator+=(const std::function<void()>& fn) 
	{
		task action;
		action._fn = std::make_shared<task::fn>(fn);
		return base::operator+=(action); 
	}
};


// How will this work without allocating?  Tasks have a life span and 
// parameters must last that life span.  

// Maybe just allocate a parameter list, eg struct callback { std::funcition<sig> fn; shared_parameters params; }
// therefore the final task is a callback containing the fn and ... no not going to work needs 2 allocs
template<typename P0>
class event<void (P0)> : public event_base< event<void (P0)> >
{
public:
	shared_task operator += (const std::function<void(P0)>& fn) 
	{ 
		shared_task pTask = std::make_shared<task>(fn, service::active());
		return base::operator+=(pTask); 
	}

	inline void operator()()
	{
		for (std::vector<shared_task>::iterator i = m_multiCast.begin();
			 i != m_multiCast.end();
			 ++i)
		{
			shared_service provider = (*i)->service.lock();
			while (!provider && i != end)
			{
				--end;
				std::swap(*end, *i);
				provider = (*i)->service.lock();
			}
			if (provider)
			{
				provider->post(*i);
			}
			++i;
		}
		m_multiCast.erase(end, m_multiCast.end());
	}

	inline void operator()() const
	{
		for(std::vector<shared_task>::iterator i = m_multiCast.begin();
			i != m_multiCast.end();
			++i)
		{
			shared_service provider = (*i)->service.lock();
			if (provider)
			{
				provider->post(*i);
			}
		}
	}
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
