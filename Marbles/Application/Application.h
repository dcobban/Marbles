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

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
class service;
typedef function<void __cdecl ()> task;
typedef shared_ptr<task> shared_task;
typedef weak_ptr<task> weak_task;
typedef shared_ptr<service> shared_service;
typedef weak_ptr<service> weak_service;

// --------------------------------------------------------------------------------------------------------------------
class application
{
public:
	application();
	~application();

	template<typename T, typename... ARG> 
	shared_service		start(ARG&&... args);
	void				stop(int run_result);
	// bool				post(const task& action);
	bool				post(const shared_task& action);
	int					run(unsigned numThreads = 0); // The value given to application::stop() is returned by this function

	static application*	get();
	static void			yield(); // Why do users need this?
	static void			sleep(int milliseconds); // Why do users need this?
	static unsigned		num_hardware_threads(); // Why is this needed?

private:
	struct implementation;
	friend class service;

	size_t service_count() const;
	shared_service service_at(size_t index);
	shared_service active_service() const;
	shared_service create_service();
	shared_service select_service();

	void _register(const shared_service& service);
	void unregister(const shared_service& service);
	void choose_service();
	void process_services();

	unique_ptr<implementation> _implementation;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T, typename... ARG> 
inline shared_service application::start(ARG&&... args)
{
	shared_service srv = create_service();
	if (srv)
	{
		service* ptr = srv.get();
		srv->post(make_shared<task>([ptr, args...]() 
		{ 
			ptr->make_provider<T>(forward<ARG>(args)...); 
		}));
	}
	return srv;
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of File --------------------------------------------------------------------------------------------------------
