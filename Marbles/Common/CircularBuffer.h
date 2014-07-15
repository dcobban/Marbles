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

namespace marbles
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
	CircularBuffer(const CircularBuffer<T, N>&) = delete;
	~CircularBuffer(){}
	inline unsigned size() const
	{
		const unsigned start = _start.load();
		const unsigned end = _end.load();
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
			reserved = _init.load();
			next = (reserved + 1) % (N + 1);
			const bool isFull = next == _clean.load();
			if (isFull)
				return false;
			// Try again if another thread has modified the _init value before me.
		} while (!_init.compare_exchange_strong(reserved, next));
		
		// Element reserved, assign the value
		_items[reserved] = value;

		// Syncronize the end position with the updated reserved position
		while (!_end.compare_exchange_strong(reserved, next))
		{	// wait for the other element to be completed.
			application::yield(); 
		}

		return true;
	}

	void push(const T& value)
	{
		while (!try_push(value))
		{
			application::yield();
		}
	}

	bool try_pop(T& out)
	{
		unsigned start;
		unsigned next;
		do 
		{
			start = _start.load();
			const bool isEmpty = start == _end.load();
			if (isEmpty)
				return false;
			next = (start + 1) % (N + 1);
		} while(!_start.compare_exchange_strong(start, next));
		
		out = T(_items[start]);
		//std::swap(_items[start], out);

		// Syncronize the clean position with the updated start position
		while (!_clean.compare_exchange_strong(start, next))
		{	// wait for the other element to be cleaned first
			application::yield(); 
		}
		
		return true;
	}

	T pop()
	{
		T tmp;
		while (!try_pop(tmp))
		{
			application::yield();
		}
		return tmp;
	}
private:
	// Todo: We really should control construction and destruction
	T						_items[N + 1]; 
	std::atomic<unsigned>	_start;
	std::atomic<unsigned>	_end;
	std::atomic<unsigned>	_init;
	std::atomic<unsigned>	_clean;
};

} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
