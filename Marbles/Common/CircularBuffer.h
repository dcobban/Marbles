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

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{

// --------------------------------------------------------------------------------------------------------------------
// Lock-free circular buffer
template<typename T, size_t N> 
class CircularBuffer
{
public:
	// ----------------------------------------------------------------------------------------------------------------
	CircularBuffer()
	: mStart(0)
	, mEnd(0)
	, mInit(0)
	, mClean(0)
	{
	}

	// ----------------------------------------------------------------------------------------------------------------
	~CircularBuffer(){}

	// ----------------------------------------------------------------------------------------------------------------
	inline unsigned size() const
	{
		const unsigned start = mStart.get();
		const unsigned end = mEnd.get();
		return start <= end ? end - start : end + N + 1 - start;
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline bool empty() const
	{
		return 0 == size();
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline bool full() const
	{
		return N == size();
	}

	// ----------------------------------------------------------------------------------------------------------------
	void clear()
	{
		while (!empty())
		{
			pop();
		}
	}

	// ----------------------------------------------------------------------------------------------------------------
	bool try_push(const T& value)
	{
		unsigned reserved;
		unsigned next;
		do
		{	// Reserve an element to be created
			reserved = mInit.get();
			next = (reserved + 1) % (N + 1);
			if (next == mClean.get())
				return false;
			// Try again if another thread has modified the mInit value before me.
		} while(reserved != mInit.compare_exchange(next, reserved));
		
		// Element reserved, assign the value
		mItems[reserved] = value;

		// Syncronize the end position with the updated reserved position
		while (reserved != mEnd.compare_exchange(next, reserved))
		{	// Wait for the other element to be completed.
			Application::Sleep(1); 
		}

		return true;
	}

	// ----------------------------------------------------------------------------------------------------------------
	void push(const T& value)
	{
		while (!try_push(value))
		{
			Application::Sleep(1);
		}
	}

	// ----------------------------------------------------------------------------------------------------------------
	bool try_pop(T& out)
	{
		unsigned start;
		unsigned next;
		do 
		{
			start = mStart.get();
			if (start == mEnd.get())
				return false;
			next = (start + 1) % (N + 1);
		} while(start != mStart.compare_exchange(next, start));
		
		out = T();
		std::swap(mItems[start], out);

		// Syncronize the clean position with the updated start position
		while (start != mClean.compare_exchange(next, start))
		{	// Wait for the other element to be cleaned first
			Application::Sleep(1); 
		}
		
		return true;
	}

	// ----------------------------------------------------------------------------------------------------------------
	T pop()
	{
		T tmp;
		while (!try_pop(tmp))
		{
			Application::Sleep(1);
		}
		return tmp;
	}

private:
	// Todo: should control construction and destruction
	T						mItems[N + 1]; 
	atomic<unsigned>		mStart;
	atomic<unsigned>		mEnd;
	atomic<unsigned>		mInit;
	atomic<unsigned>		mClean;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
