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

#include <type_traits> 
#include "definitions.h"

namespace marbles
{

// Lock-free circular buffer
template<typename T, size_t N> 
class alignas(alignment_of<T>::value) atomic_buffer final
{
public:
	atomic_buffer()
	: _start(0)
	, _end(0)
	, _init(0)
	, _clean(0)
	{
	}

	~atomic_buffer()
	{ 
		clear();
	}

    atomic_buffer(const atomic_buffer<T, N>&) = delete;
    atomic_buffer<T, N>& operator=(const atomic_buffer<T, N>&) = delete;

	inline unsigned size() const
	{
		const unsigned start = _start.load();
		const unsigned end = _end.load();
		const unsigned inclusive_range = end - start;
		const unsigned exclusive_range = end + N - start;
		return start <= end ? inclusive_range : exclusive_range;
	}

	inline unsigned capacity() const
	{
		return N;
	}

	inline bool empty() const
	{
		return 0 == size();
	}

	inline bool full() const
	{
		return (N - 1) == size();
	}

	void clear()
	{
		T out; // Review: Causes all template types to require a default constructor, this API may not be appropriate
		while (try_pop(out)) 
		{
			this_thread::yield();
		}
	}

	bool try_push(T value)
	{
		unsigned reserved;
		unsigned next;
        do
        {	// Reserve an element to be created
            reserved = _init.load();
            next = (reserved + 1) % N;
            const bool isFull = next == _clean.load();
            if (isFull)
            {
                return false;
            }
			// Try again if another thread has modified the _init value before me.
		} while (!_init.compare_exchange_weak(reserved, next));
		
		// Element reserved, assign the value
		new (items() + reserved) T(forward<T>(value));

		// Synchronize the end position with the updated reserved position
		const unsigned persist = reserved;
		while (!_end.compare_exchange_weak(reserved, next))
		{	// wait for the other element to be completed.
			reserved = persist;
			this_thread::yield(); 
		}

		return true;
	}

	void push(T value)
	{
		while (!try_push(forward<T>(value)))
		{
			this_thread::yield();
		}
	}

	bool try_pop(T& out)
	{
		unsigned start;
		unsigned next;
		do 
		{
			start = _start.load();
			const bool is_empty = start == _end.load();
			if (is_empty)
            {
				return false;
            }
			next = (start + 1) % N;
		} while(!_start.compare_exchange_weak(start, next));
		
		out = move(*(items() + start));
		(items() + start)->~T();

		// Syncronize the clean position with the updated start position
		const unsigned persist = start;
		while (!_clean.compare_exchange_weak(start, next))
		{	// wait for the other element to be cleaned first
			start = persist;
			this_thread::yield();
		}
		
		return true;
	}

	T pop()
	{
		T tmp; // Review: Causes all template types to require a default constructor, this API may not be appropriate
		while (!try_pop(tmp))
		{
			this_thread::yield();
		}
		return move(tmp);
	}

    const T& operator[](int index) const
    {
        assert(index < size());
        const int position = (_start + index) % N;
        return *(items() + position);
    }

private:
	T*                  items()       { return reinterpret_cast<T*>(&_reserve[0]); }
	const T*            items() const { return reinterpret_cast<const T*>(&_reserve[0]); }

	uint8_t				_reserve[sizeof(T) * N];
	atomic<unsigned>	_start;
	atomic<unsigned>	_end;
	atomic<unsigned>	_init;
	atomic<unsigned>	_clean;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
