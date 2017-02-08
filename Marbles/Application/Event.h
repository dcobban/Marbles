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

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
class event_base
{
public:
	class callback;
	typedef function<void(Args...)> handler_t;
	typedef shared_ptr<callback> shared_handler;
    typedef weak_ptr<callback> weak_handler;

	shared_handler	onRaise(const handler_t& handler);
	shared_handler	onRaise(shared_handler& handler);
	shared_handler	clear(shared_handler& handler);
	void			clear();

	shared_handler	operator+=(const handler_t& handler);
	shared_handler	operator+=(shared_handler& handler);
	shared_handler	operator-=(shared_handler& handler);

protected:
    shared_handler  next_callback();
	atomic<callback*> _handlers;
	shared_handler _refHandlers;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
class event_base<Args...>::callback : public event_base<Args...>
{
public:
	void raise(const Args... args);
	void operator()(const Args... args);

protected:
	friend class event_base<Args...>; // Is this needed? ... remove it and find out!
	event_base<Args...>::handler_t _handler;
	weak_service _service;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
class event : public event_base<Args...>
{
public:
	event();

	void raise(const Args... args);
	void operator()(const Args... args);
};

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event_base<Args...>::shared_handler event_base<Args...>::onRaise(const handler_t& handler)
{
	auto new_handler = make_shared<callback>();
	new_handler->_handler = handler;
	new_handler->_service = service::active();
    return onRaise(new_handler);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event_base<Args...>::shared_handler event_base<Args...>::onRaise(shared_handler& handler)
{
	auto new_handler = handler;
	if (!new_handler->_handlers.load())
	{
		new_handler = make_shared<callback>();
		new_handler->_handler = handler->_handler;
		new_handler->_service = handler->_service;
	}

    callback* local = nullptr;
	do {
		new_handler->_handlers = _handlers.load();
		new_handler->_refHandlers = _refHandlers;
        _refHandlers = new_handler;
        local = _refHandlers.get();
	} while (!_handlers.compare_exchange_weak(local, _handlers.load()));

	return new_handler;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event_base<Args...>::shared_handler event_base<Args...>::operator+=(const handler_t& handler)
{
	return onRaise(handler);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event_base<Args...>::shared_handler event_base<Args...>::operator+=(shared_handler& handler)
{
	return onRaise(handler);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event_base<Args...>::shared_handler event_base<Args...>::operator-=(shared_handler& callback)
{	
	return move(clear(callback));
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event_base<Args...>::shared_handler event_base<Args...>::clear(shared_handler& handler)
{
    (void)handler;
    // remove from list
    //handler_t* handler;

}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event_base<Args...>::shared_handler event_base<Args...>::next_callback()
{
    handler_t* handler;
    shared_handler callback;
    do {
        handler = _handlers.load();
        callback = _refHandlers;
    } while (_handlers.compare_exchange_weak(handler, handler));

    return move(callback);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event_base<Args...>::callback::raise(const Args... args)
{
    shared_handler handler = next_callback();
    if (handler)
    {
        shared_service service = handler->_service.lock();
        if (service)
        {
            weak_handler trigger = handler;
            service->post([trigger, args...]() {
                shared_handler myHandler = trigger.lock();
                if (myHandler)
                {
                    myHandler->callback(args...);
                }
            });
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event_base<Args...>::callback::operator()(const Args... args) 
{
	raise(forward<Args>(args)...);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event_base<Args...>::clear()
{ 
    shared_handler handler;
    _handlers.exchange(nullptr);
    handler.swap(_refHandlers);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
event<Args...>::event()
{
    event_base<Args...>::clear();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event<Args...>::raise(const Args... args)
{
    shared_handler callback = next_callback();
	callback->raise(args...);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
void event<Args...>::operator()(const Args... args)
{
    raise(args...);
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
