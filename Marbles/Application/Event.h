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
//#include <tuple>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
class event
{
public:
	event();

	typedef std::function<void (Args...)> handler_fn;
	typedef std::shared_ptr<handler_fn> shared_handler;

	shared_handler operator+=(handler_fn handler);
	shared_handler operator+=(shared_handler handler);
	shared_handler operator-=(shared_handler& handler);
	void operator()(const Args&&... args);
	void clear();

private:
	struct event_info;
	struct message_info
	{
		std::shared_ptr<event_info> _event;
		//std::tuple<Args...> _params;
		shared_task _process_event;
		
		message_info();
		template<int ...S> void message_task(seq<S...>);
	};
	struct handler_info
	{
		shared_handler _handler;
		weak_service _service;
	};
	struct event_info
	{	// std::thread safety?  I don't see any.
		std::vector<handler_info> _handlers;
	};
	std::vector<std::shared_ptr<message_info>> _pool; // pool could work for any event with these params
	std::vector<weak_service> _services;
	std::shared_ptr<event_info> _event;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
event<Args...>::event()
{
	clear();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event<Args...>::shared_handler event<Args...>::operator+=(typename event<Args...>::handler_fn handler)
{ 
	return operator+=(std::make_shared<handler_fn>(std::forward<handler_fn>(handler)));
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event<Args...>::shared_handler event<Args...>::operator+=(typename event<Args...>::shared_handler handler)
{
	auto active = service::active();
	auto& handlers = _event->_handlers;

	// Clear invalidated handlers
	_services.push_back(active);
	_services.erase(
		unique(_services.begin(), _services.end(), [](weak_service& e1, weak_service& e2)
		{   // We should be able to compare std::weak_ptr's?
			return e1.lock() == e2.lock();
		}),
		_services.end());
	handlers.erase(
		remove_if(handlers.begin(), handlers.end(), [](const handler_info& info)
		{
			return info._service.expired();
		}),
		handlers.end());

	// Add new handler
	handler_info new_handler;
	new_handler._handler = handler;
	new_handler._service = active;
	_event->_handlers.push_back(std::move(new_handler));

	return handler; 
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event<Args...>::shared_handler event<Args...>::operator-=(typename event<Args...>::shared_handler& handler)
{	
	auto& handlers = _event->_handlers;

	handlers.erase(
		remove_if(handlers.begin(), handlers.end(), [&handler](const handler_info& info)
		{
			return handler == info._handler; 
		}), 
		handlers.end());

	// Push all dependent services
	for (auto& info : handlers)
	{
		_services.push_back(info._service);
	}
	
	// Only unique services are allowed
	_services.erase(
		unique(_services.begin(), _services.end(), [](weak_service& e1, weak_service& e2)
		{	// We should be able to compare std::weak_ptr's?
			return e1.lock() == e2.lock();
		}),
		_services.end());
	return handler;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event<Args...>::operator()(const Args&&... args) 
{
	// Find usable item from pool
	auto param = find_if(_pool.begin(), _pool.end(), [](const std::shared_ptr<message_info>& info)
	{
		return !info->_event;
	});
	if (param == _pool.end())
	{
		auto info = std::make_shared<message_info>();
		info->_event = _event;
		_pool.push_back(std::move(info));
		param = _pool.begin() + _pool.size() - 1;
	}
	else
	{
		(*param)->_event = _event;
	}
	//(*param)->_params = std::make_tuple(args...);

	for (auto& srv : _services)
	{
		auto receiver = srv.lock();
		if (receiver)
		{
			receiver->post((*param)->_process_event);
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event<Args...>::clear() 
{ 
	_event = std::make_shared<event_info>();
	_services.clear();
	_pool.clear();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
event<Args...>::message_info::message_info()
{
	auto fn = [this]() { this->message_task(gens<sizeof...(Args)>::type()); };
	_process_event = std::make_shared<task>(std::move(fn));
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
template<int ...S>
void event<Args...>::message_info::message_task(seq<S...>)
{
	if (_event)
	{
		shared_service active = service::active();
		for (auto& handler : _event->_handlers)
		{
			if (active == handler._service.lock() && handler._handler)
			{
				(*handler._handler)(get<S>(_params) ...);
			}
		}
		_event.reset();
	}
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
