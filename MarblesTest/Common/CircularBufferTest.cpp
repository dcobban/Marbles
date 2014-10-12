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
#include <thread>

BOOST_AUTO_TEST_SUITE( circular_buffer )

BOOST_AUTO_TEST_CASE( basic_operations )
{
	BOOST_MESSAGE( "circular_buffer.basic_operations" );

	const int size = 10;
	marbles::circular_buffer<int, size> buffer;

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

BOOST_AUTO_TEST_CASE(multi_threaded_push_pop)
{
	BOOST_MESSAGE("circular_buffer.multi_threaded_push");
	const int quantity = 150;
	const int numProducers = 15;

	marbles::circular_buffer<int, numProducers*quantity> data;
	marbles::circular_buffer<int, numProducers*quantity> consumerData;
	std::array<std::thread, numProducers> producerThreads;
	for (auto id = numProducers; id--;)
	{
		std::thread producer([&data, id, quantity]()
		{
			for (int i = quantity; i--;)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				data.push(id);
			}
		});
		producerThreads[id].swap(producer);
	}

	for (auto j = producerThreads.size(); j--;)
	{
		producerThreads[j].join();
	}

	typedef std::array<int, numProducers> tally_t;
	tally_t tally;
	for (auto& sum : tally)
	{
		sum = 0;
	}

	while (!data.empty())
	{
		int value = data.pop();
		consumerData.push(value);
		++tally[value];
	}

	for (auto sum : tally)
	{
		BOOST_CHECK_EQUAL(sum, quantity);
	}

	BOOST_MESSAGE("circular_buffer.multi_threaded_pop");
	const int numConsumers = 6;
	std::array<tally_t, numConsumers> tallySheet;
	for (auto& tally : tallySheet)
	{
		for (auto& value : tally)
		{
			value = 0;
		}
	}

	std::array<std::thread, numConsumers> consumerThreads;
	for (auto id = numConsumers; id--;)
	{
		tally_t& tally = tallySheet[id];
		std::thread consumer([&tally, &consumerData]()
		{
			int value = 0;
			while (consumerData.try_pop(value))
			{
				++tally[value];
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		});
		consumerThreads[id].swap(consumer);
	}

	for (auto j = consumerThreads.size(); j--;)
	{
		consumerThreads[j].join();
	}

	tally_t finalTally;
	for (auto& value : finalTally)
	{
		value = 0;
	}

	for (auto& tally : tallySheet)
	{
		for (int i = tally.size(); i--;)
		{
			finalTally[i] += tally[i];
		}
	}

	for (auto& sum : finalTally)
	{
		BOOST_CHECK_EQUAL(sum, quantity);
	}
}

BOOST_AUTO_TEST_CASE(multi_thread_usage)
{
	BOOST_MESSAGE( "circular_buffer.multi_thread_usage" );
	const int quantity = 150;
	const int bufferSize = 10;
	const int numProducers = 15;
	const int numBuffers = numProducers >> 1;

	typedef marbles::circular_buffer<int, bufferSize> Buffer;
	typedef std::array<Buffer, numProducers> BufferArray;
	typedef std::array<std::thread, numProducers> ThreadArray;
	typedef std::array<int, numProducers> TallyArray;

	TallyArray tally;
	ThreadArray threads;
	BufferArray buffers;
	Buffer accumulation;

	for (auto& count : tally)
	{
		count = 0;
	}

	auto accumulator = [&buffers, &accumulation](int)
	{
		int value = 0;
		unsigned index = 0;
		do
		{
			++index;
			index %= buffers.size();
		
			if (buffers[index].try_pop(value) && 0 <= value)
			{
				accumulation.push(value);
			}
			std::this_thread::yield();
		} while (0 <= value);
	};

	std::thread accumulatorThread([accumulator]{ accumulator(1); });
	std::thread accumulator2Thread([accumulator]{ accumulator(2); });
	for (unsigned id = 0; id < numProducers; ++id)
	{
		Buffer& buffer = buffers[id%numBuffers];
		std::thread producer([&buffer, id, quantity]()
		{
			for (int i = quantity; i--;)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				buffer.push(id);
			}
		});
		threads[id].swap(producer);
	}

	int sum = 0;
	do
	{
		int value;
		if (accumulation.try_pop(value))
		{
			++tally[value];
			++sum;
		}
	} while (sum < numProducers*quantity);
	
	int k = 0;
	for(auto amount : tally)
	{
		BOOST_CHECK_EQUAL(amount, quantity);
		threads[k++].join();
	}

	// Push an exit for each accumulator std::thread
	const int exit = -1;
	for (unsigned j = 0; j < buffers.size(); ++j)
	{
		buffers[j].push(exit);
	}

	// Clean up
	accumulatorThread.join();
	accumulator2Thread.join();

	accumulation.clear();
}

BOOST_AUTO_TEST_SUITE_END()