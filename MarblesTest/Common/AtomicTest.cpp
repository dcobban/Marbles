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

#include <Common/AtomicBuffer.h>
#include <Common/AtomicList.h>
#include <thread>

void rest_thread(int value)
{
    // yield and sleep_for to encourage thread contention.
    if (value % 10)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    else
    {
        std::this_thread::yield();
    }
}

template<typename T>
class wrap
{
public:
    wrap(T val = 0) : _value(val) {}
    wrap<T>& set(const T& val) { _value = val; return *this; }
    T& get() { return _value; }
    operator T() { return _value; }
private:
    T _value;
};

TEST(atomic_buffer, basic_operations)
{
	const int size = 10;
	marbles::atomic_buffer<int, size> buffer;

	int pop = 0;
	int push = 0;

	EXPECT_EQ(true, buffer.empty());

	while (buffer.try_push(push))
	{
		++push;
		EXPECT_EQ(push, buffer.size());
	}

	EXPECT_EQ(true, buffer.full());
	EXPECT_EQ(push, size);

	while (buffer.try_pop(pop))
	{
		EXPECT_EQ(pop, size - buffer.size() - 1);
	}

	EXPECT_EQ(true, buffer.empty());

	while (buffer.try_push(push))
	{
		++push;
		if (push % 2)
		{
			EXPECT_EQ(++pop, buffer.pop());
		}
	}

	EXPECT_EQ(true, buffer.full());

	int dummy;
	while (buffer.try_pop(dummy))
	{
		EXPECT_EQ(++pop, dummy);
	}

	EXPECT_EQ(true, buffer.empty());
}

TEST(atomic_buffer, multi_threaded_push_pop)
{
	const int32_t quantity = 150;
	const int32_t numProducers = 15;

	marbles::atomic_buffer<wrap<int32_t>, numProducers*quantity> data;
	marbles::atomic_buffer<int32_t, numProducers*quantity> consumerData;
    marbles::array<std::thread, numProducers> producerThreads;
	for (auto id = numProducers; id--;)
	{
        std::thread producer([&data, id, quantity]()
		{
			for (auto i = quantity; i--;)
			{
                rest_thread(i);
                data.push(id);
			}
		});
		producerThreads[id].swap(producer);
	}

	for (auto j = producerThreads.size(); j--;)
	{
		producerThreads[j].join();
	}

	typedef marbles::array<int, numProducers> tally_t;
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
		EXPECT_EQ(sum, quantity);
	}

	const size_t numConsumers = 6;
	marbles::array<tally_t, numConsumers> tallySheet;
	for (auto& count : tallySheet)
	{
		for (auto& value : count)
		{
			value = 0;
		}
	}

    marbles::array<std::thread, numConsumers> consumerThreads;
	for (auto id = numConsumers; id--;)
	{
		tally_t& count = tallySheet[id];
		std::thread consumer([&count, &consumerData]()
		{
			int value = 0;
			while (consumerData.try_pop(value))
			{
				++count[value];
                rest_thread(value);
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

	for (auto& count : tallySheet)
	{
		for (auto i = count.size(); i--;)
		{
			finalTally[i] += count[i];
		}
	}

	for (auto& sum : finalTally)
	{
		EXPECT_EQ(sum, quantity);
	}
}

TEST(atomic_buffer, multi_thread_usage)
{
	const int quantity = 150;
	const int bufferSize = 10;
	const int numProducers = 10;
	const int numBuffers = numProducers >> 1;

	typedef marbles::atomic_buffer<wrap<int>, bufferSize> Buffer;
	typedef marbles::array<Buffer, numProducers> BufferArray;
	typedef marbles::array<std::thread, numProducers> ThreadArray;
	typedef marbles::array<int, numProducers> TallyArray;

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
		wrap<int> value = 0;
		unsigned index = 0;
		do
		{
			++index;
			index %= buffers.size();
		
			if (buffers[index].try_pop(value) && 0 <= value)
			{
				accumulation.push(value);
			}

            rest_thread(value);
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
                rest_thread(i);
                buffer.push(id);
			}
		});
		threads[id].swap(producer);
	}

	int sum = 0;
	do
	{
		wrap<int> value(0);
		if (accumulation.try_pop(value))
		{
			++tally[value];
			++sum;
		}
	} while (sum < numProducers*quantity);
	
	int k = 0;
	for(auto amount : tally)
	{
		EXPECT_EQ(amount, quantity);
		threads[k++].join();
	}

	// Push an exit for each accumulator thread
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

TEST(atomic_list, basic_operations)
{
    typedef marbles::atomic_list<int> list_t;
    const int size = 100;
    std::vector<list_t::node_type> nodes;
    list_t list;
    int count;

    nodes.reserve(size);
    EXPECT_EQ(true, list.empty());
    while (nodes.size() != nodes.capacity())
    {
        list_t::node_type next(static_cast<int>(nodes.capacity() - nodes.size()));
        nodes.push_back(next);
        list.insert_next(&nodes[nodes.size() - 1]);
        EXPECT_EQ(false, list.empty());
    }

    count = 1;
    for (auto i = list.begin(); i != list.end(); ++i, ++count)
    {
        EXPECT_EQ(count, *i);
    }

    auto* node = list.next();
    list.set_next(nullptr);
    EXPECT_EQ(true, list.empty());
    list.set_next(node);

    count = 1;
    for (auto i = list.begin(); i != list.end(); ++i, count += 2)
    {
        i.remove_next(&node);
        EXPECT_EQ(count, node->get());
        EXPECT_EQ(count + 1, *i);
    }

    //count = 1;
    //for (auto i = list.begin(); i != list.end(); ++i, count += 2)
    //{
    //    i.remove_next(&node);
    //    EXPECT_EQ(count, node->get());
    //    EXPECT_EQ(count + 1, *i);
    //}

    //EXPECT_EQ(true, buffer.full());
    //EXPECT_EQ(push, size);

    //while (buffer.try_pop(pop))
    //{
    //    EXPECT_EQ(pop, size - buffer.size() - 1);
    //}

    //EXPECT_EQ(true, buffer.empty());

    //while (buffer.try_push(push))
    //{
    //    ++push;
    //    if (push % 2)
    //    {
    //        EXPECT_EQ(++pop, buffer.pop());
    //    }
    //}

    //EXPECT_EQ(true, buffer.full());

    //int dummy;
    //while (buffer.try_pop(dummy))
    //{
    //    EXPECT_EQ(++pop, dummy);
    //}

    //EXPECT_EQ(true, buffer.empty());
}
