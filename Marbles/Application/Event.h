// This source file is part of Marbles library.
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
namespace Marbles
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

	inline base& operator+=(task::fn& fn)
	{
		return operator+=(std::make_shared<task>(fn, service::active()));
	}
	inline base& operator+=(shared_task& task)
	{
		m_multiCast.push_back(task);
		return *this;
	}

	inline base& operator-=(task::fn& fn);
	inline base& operator-=(shared_task& task);

	inline void operator()()
	{
		std::vector<shared_task>::iterator i = m_multiCast.begin();
		std::vector<shared_task>::iterator end = m_multiCast.end();
		while(i != end)
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
private:
	std::vector<shared_task> m_multiCast;
};

// --------------------------------------------------------------------------------------------------------------------
template<>
class event<void ()> : public event_base< event<void ()> >
{
public:
	base& operator += (const std::function<void()>& fn) 
	{ 
		shared_task pTask = std::make_shared<task>(fn, service::active());
		return base::operator+=(pTask); 
	}

	template<typename Fn>
	base& operator -= (const Fn& fn); // Unknown how to implement this, std::function cannot be compaired. 
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
