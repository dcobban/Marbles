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

#include <application\service.h>
#include <algorithm>
#include <tuple>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
class event
{
public:
	typedef std::function<void (Args...)> handler_fn;
	typedef std::shared_ptr<handler_fn> shared_handler;
	typedef std::weak_ptr<handler_fn> weak_handler;

	weak_handler operator+=(const handler_fn& handler);
	weak_handler operator+=(shared_handler handler);
	shared_handler operator-=(const shared_handler& handler);
	void operator()(const Args&&... args);
	void clear();

private:
	struct receiver
	{
		shared_handler _handler;
		weak_service _service;
	};
	typedef std::tuple<Args...> params;
	typedef std::shared_ptr<params> shared_params;
	std::vector<shared_params> _params;
	std::vector<receiver> _handlers;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event<Args...>::weak_handler event<Args...>::operator+=(const typename event<Args...>::handler_fn& handler) 
{ 
	return operator+=(std::make_shared<handler_fn>(handler));
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event<Args...>::weak_handler event<Args...>::operator+=(typename event<Args...>::shared_handler handler) 
{
	receiver box = { handler, service::active() };
	auto result = std::find_if(_handlers.begin(), _handlers.end(), [](const receiver& box)
	{
		return box._service.expired();
	});
	if (result != _handlers.end())
	{
		*result = std::move(box);
	}
	else
	{
		_handlers.push_back(std::move(box));
	}

	return handler; 
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event<Args...>::shared_handler event<Args...>::operator-=(const typename event<Args...>::shared_handler& handler) 
{
	_handlers.erase(
		std::remove_if(_handlers.begin(), _handlers.end(), [&handler](const receiver& box)
		{
			return handler == box._handler; 
		}), 
		_handlers.end());
	return handler;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event<Args...>::operator()(const Args&&... args) 
{
	for (auto& msg : _handlers)
	{
		shared_service service = msg._service.lock();
		if (service)
		{
			// TODO move make_shared<> call to operator+=
			shared_handler handler = msg._handler;
			service->post(std::make_shared<task>([handler, args...]()
			{ 
				(*handler)(std::forward<Args>(args)...);
			}));
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event<Args...>::clear() 
{ 
	_handlers.clear(); 
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
