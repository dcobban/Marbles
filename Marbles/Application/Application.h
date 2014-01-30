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

#include <memory>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
// --------------------------------------------------------------------------------------------------------------------
class task;
typedef std::shared_ptr<task> shared_task;
typedef std::weak_ptr<task> weak_task;

class service;
typedef std::shared_ptr<service> shared_service;
typedef std::weak_ptr<service> weak_service;

// --------------------------------------------------------------------------------------------------------------------
class application
{
public:
	struct user_settings
	{
		float fps;

		user_settings()
		: fps(60.0f)
		{
		}
	};
	static user_settings& settings();
	//static int CommandLineCount();
	//static char*[] CommandLine();
	static unsigned num_hardware_threads();

	static void sleep(int milliseconds);
	static application* get();

	application();
	~application();
	void stop(int run_result);
	int run(unsigned numThreads = 0); // The value given to application::stop() is returned by this function

	template<typename fn>
	shared_service start();
	template<typename fn, typename A0>
	shared_service start(A0& a0);
	template<typename fn, typename A0, typename A1>
	shared_service start(A0& a0, A1& a1);
	template<typename fn, typename A0, typename A1, typename A2>
	shared_service start(A0& a0, A1& a1, A2& a2);
	template<typename fn, typename A0, typename A1, typename A2, typename A3>
	shared_service start(A0& a0, A1& a1, A2& a2, A3& a3);
	template<typename fn, typename A0, typename A1, typename A2, typename A3, typename A4>
	shared_service start(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4);
	template<typename fn, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
	shared_service start(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5);

private:
	struct kernel;
	struct implementation;
	friend class service;

	shared_service activeService() const;
	void _register(shared_service service);
	void unregister(shared_service service);

	unique_ptr<implementation> _implementation;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename fn>
shared_service application::start()
{
	shared_service service = service::create();
	_register(service);
	if (service)
	{
		service->post(std::bind<void>(	&service::make_provider<fn>, 
										service.get()));
	}
	return service;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0>
shared_service application::start(A0& a0)
{
	shared_service service = service::create();
	_register(service);
	if (service)
	{
		service->post(std::bind<void>(	&service::make_provider<fn, A0>, 
										service.get(), a0));
	}
	
	return service;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0, typename A1>
shared_service application::start(A0& a0, A1& a1)
{
	shared_service service = service::create();
	_register(service);
	if (service)
	{
		service->post(std::bind<void>(	&service::make_provider<fn, A0, A1>, 
										service.get(), a0, a1));
	}
	return service;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0, typename A1, typename A2>
shared_service application::start(A0& a0, A1& a1, A2& a2)
{
	shared_service service = service::create();
	_register(service);
	if (service)
	{
		service->post(std::bind<void>(	&service::make_provider<fn, A0, A1, A2>, 
										service.get(), a0, a1, a2));
	}
	return service;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0, typename A1, typename A2, typename A3>
shared_service application::start(A0& a0, A1& a1, A2& a2, A3& a3)
{
	shared_service service = service::create();
	_register(service);
	if (service)
	{
		service->post(std::bind<void>(	&service::make_provider<fn, A0, A1, A2, A3>, 
										service.get(), a0, a1, a2, a3));
	}
	return service;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0, typename A1, typename A2, typename A3, typename A4>
shared_service application::start(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4)
{
	shared_service service = service::create();
	_register(service);
	if (service)
	{
		service->post(std::bind<void>(	&service::make_provider<fn, A0, A1, A2, A3, A4>, 
										service.get(), a0, a1, a2, a3, a4));
	}
	return service;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename fn, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
shared_service application::start(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
{
	shared_service service = service::create();
	_register(service);
	if (service)
	{
		service->post(std::bind<void>(	&service::make_provider<fn, A0, A1, A2, A3, A4, A5>, 
										service.get(), a0, a1, a2, a3, a4, a5));
	}
	return service;
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of File --------------------------------------------------------------------------------------------------------
