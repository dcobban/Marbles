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

#include <Common/CircularBuffer.h>
//#include <boost/thread.hpp>
#include <thread>

BOOST_AUTO_TEST_SUITE( CircularBuffer )

BOOST_AUTO_TEST_CASE( basic_operations )
{
	BOOST_MESSAGE( "CircularBuffer.basic_operations" );

	const int size = 10;
	marbles::CircularBuffer<int, size> buffer;

	int pop = 0;
	int push = 0;

	BOOST_CHECK(buffer.empty());

	while (buffer.try_push(push))
	{
		++push;
		BOOST_CHECK_EQUAL(push, buffer.size());
	}

	BOOST_CHECK(buffer.full());
	BOOST_CHECK_EQUAL(push, size);

	while (buffer.try_pop(pop))
	{
		BOOST_CHECK_EQUAL(pop, size - buffer.size() - 1);
	}

	BOOST_CHECK(buffer.empty());

	while (buffer.try_push(push))
	{
		++push;
		if (push % 2)
		{
			BOOST_CHECK_EQUAL(++pop, buffer.pop());
		}
	}

	BOOST_CHECK(buffer.full());

	int dummy;
	while (buffer.try_pop(dummy))
	{
		BOOST_CHECK_EQUAL(++pop, dummy);
	}

	BOOST_CHECK(buffer.empty());
}

namespace 
{
	static const int numProducers = 15;
	static const int numBuffers = (numProducers + 1) / 2;
	static const int bufferSize = 15;
	typedef marbles::CircularBuffer<int, bufferSize> Buffer_t;
	static Buffer_t buffers[numBuffers];
	static Buffer_t accumulation;
}

void marshaller()
{
	int value;
	int index = 0;
	for (int i = numBuffers; i--;)
	{
		value = buffers[index].pop();
		accumulation.push(value);
	}

	int timeout;
	do 
	{
		timeout = 100 * numBuffers;
		do
		{
			index += 1;
			index %= numBuffers;
			--timeout;
		} while(buffers[index].empty() && 0 < timeout);

		if (buffers[index].try_pop(value))
		{
			accumulation.push(value);
		}
	} while(0 <= value);
}

void producer(Buffer_t* buffer, int value, int amount)
{
	for(int i = amount; i--; )
	{
		buffer->push(value);
	}
}

BOOST_AUTO_TEST_CASE( multi_thread_usage )
{
	BOOST_MESSAGE( "CircularBuffer.multi_thread_usage" );
	const int amountProduced = bufferSize * 10;
	typedef std::array<std::thread, numProducers> ThreadArray;
	typedef std::array<int, numProducers> AmountArray;

	AmountArray amounts;
	ThreadArray threads;

	AmountArray::iterator amount = amounts.begin();
	while(amount != amounts.end())
	{
		*amount++ = 0;
	}

	std::thread marshallerThread(marshaller);
	std::thread marshaller2Thread(marshaller);
	for(unsigned i = numProducers; i--;)
	{
		std::thread action(std::bind(producer, &buffers[i%numBuffers], i, amountProduced));
		threads[i].swap(action);
	}

	int sum = 0;
	do
	{
		int value;
		if (accumulation.try_pop(value))
		{
			++amounts[value];
			++sum;
		}
	} while(sum < numProducers*amountProduced);

	// Push an exit for each marchal thread
	const int exit = -1;
	for (int i = 0; i < numBuffers;++i)
		buffers[i].push(exit);

	amount = amounts.begin();
	while(amount != amounts.end())
	{
		BOOST_CHECK_EQUAL(*amount++, amountProduced);
	}

	// Clean up
	marshallerThread.join();
	marshaller2Thread.join();

	accumulation.clear();
}

BOOST_AUTO_TEST_SUITE_END()