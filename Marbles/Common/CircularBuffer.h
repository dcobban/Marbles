// This source file is part of Marbles library.
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

namespace Marbles
{

// Lock-free circular buffer
template<typename T, size_t N> class CircularBuffer
{
public:
	CircularBuffer()
	: _start(0)
	, _end(0)
	, _init(0)
	, _clean(0)
	{
	}
	~CircularBuffer(){}
	inline unsigned size() const
	{
		const unsigned start = _start.get();
		const unsigned end = _end.get();
		return start <= end ? end - start : end + N + 1 - start;
	}
	inline bool empty() const
	{
		return 0 == size();
	}
	inline bool full() const
	{
		return N == size();
	}
	void clear()
	{
		while (!empty())
		{
			pop();
		}
	}
	bool try_push(const T& value)
	{
		unsigned reserved;
		unsigned next;
		do
		{	// Reserve an element to be created
			reserved = _init.get();
			next = (reserved + 1) % (N + 1);
			if (next == _clean.get())
				return false;
			// Try again if another thread has modified the _init value before me.
		} while(reserved != _init.compare_exchange(next, reserved));
		
		// Element reserved, assign the value
		_items[reserved] = value;

		// Syncronize the end position with the updated reserved position
		while (reserved != _end.compare_exchange(next, reserved))
		{	// wait for the other element to be completed.
			application::sleep(1); 
		}

		return true;
	}

	void push(const T& value)
	{
		while (!try_push(value))
		{
			application::sleep(1);
		}
	}

	bool try_pop(T& out)
	{
		unsigned start;
		unsigned next;
		do 
		{
			start = _start.get();
			if (start == _end.get())
				return false;
			next = (start + 1) % (N + 1);
		} while(start != _start.compare_exchange(next, start));
		
		out = T();
		std::swap(_items[start], out);

		// Syncronize the clean position with the updated start position
		while (start != _clean.compare_exchange(next, start))
		{	// wait for the other element to be cleaned first
			application::sleep(1); 
		}
		
		return true;
	}

	T pop()
	{
		T tmp;
		while (!try_pop(tmp))
		{
			application::sleep(1);
		}
		return tmp;
	}
private:
	// Todo: We really should control construction and destruction
	T						_items[N + 1]; 
	atomic<unsigned>		_start;
	atomic<unsigned>		_end;
	atomic<unsigned>		_init;
	atomic<unsigned>		_clean;
};

} // namespace Marbles
