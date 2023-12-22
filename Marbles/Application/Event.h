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
#include <application\application.h>
#include <common\atomiclist.h>
#include <algorithm>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
class event
{
public:
    class       handler;
    typedef     function<void(Args...)> callback_t;

    void        clear();
    void        clear(handler* callback);
    handler*    onRaise(callback_t callback);
    handler*    onRaise(callback_t callback, shared_service service);
    void        raise(Args... args);

    void        operator()(Args... args);
    event&      operator+=(callback_t callback);

protected:
    typedef atomic_list<handler> handlers_t;
    handlers_t _handlers;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
class event<Args...>::handler
{
public:
    handler() = default;
    handler(callback_t callback, shared_service service);

    const callback_t& callback() const;
    const callback_t& callback(callback_t fn);
    shared_service service();
    shared_service service(shared_service service);

private:
    callback_t _callback;
    weak_service _service;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
event<Args...>::handler::handler(callback_t fn, shared_service service)
: _callback(move(fn))
, _service(move(service))
{
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline const typename event<Args...>::callback_t& event<Args...>::handler::callback() const
{
    return _callback;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline const typename event<Args...>::callback_t& event<Args...>::handler::callback(typename event<Args...>::callback_t fn)
{
    _callback = move(fn);
    return _callback;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline shared_service event<Args...>::handler::service()
{
    return _service.lock();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline shared_service event<Args...>::handler::service(shared_service srvc)
{
    _service = srvc;
    return _service.lock();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event<Args...>::clear()
{
    _handlers.clear();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event<Args...>::clear(event<Args...>::handler* callback)
{
    _handlers.remove(callback);

    return callback;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event<Args...>::handler* event<Args...>::onRaise(event<Args...>::callback_t callback)
{
    return onRaise(forward<callback_t>(callback), service::active());
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline typename event<Args...>::handler* event<Args...>::onRaise(event<Args...>::callback_t callback, shared_service service)
{
    handler* result = nullptr;
    typename handlers_t::node* element = new handlers_t::node();
    if (element)
    {
        element->get()->callback(forward<callback_t>(callback));
        element->get()->service(forward<shared_service>(service));
        _handlers.insert_next(element);
        result = element->get();
    }
    return result;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event<Args...>::operator()(Args... args)
{
    raise(forward<Args>(args)...);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline event<Args...>& event<Args...>::operator+=(callback_t callback)
{
    onRaise(callback);
    return *this; 
}

// --------------------------------------------------------------------------------------------------------------------
template<typename... Args>
inline void event<Args...>::raise(Args... args)
{
    for (handler& callback : _handlers)
    {
        application* app = application::get();
        shared_service service = callback.service();
        if (service)
        {
            service->post(callback.callback(), forward<Args>(args)...);
        }
        else if (app)
        {
            app->post(callback.callback(), forward<Args>(args)...);
        }
        else
        {
            callback.callback()(forward<Args>(args)...);
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
