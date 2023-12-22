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

#include <Common/AtomicList.h>
#include <Common/AtomicBuffer.h>
#include <Common/AtomicQueue.h>
#include <thread>

void rest_thread(int value)
{
	(void)value;
    std::this_thread::sleep_for(std::chrono::microseconds(1));
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

TEST(atomic_test, list_operations)
{
    typedef marbles::atomic_list<int> list_t;
    const int size = 25;
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
    list_t odd;
    list_t even;
    while (!list.empty())
    {
        list.remove_next(&node);
        EXPECT_EQ(count++, *node->get());

        if (*node->get() % 2)
        {
            odd.append(node);
        }
        else
        {
            even.append(node);
        }
    }

    count = 0;
    for (auto i = odd.begin(); i != odd.end(); ++i)
    {
        EXPECT_LT(count, *i);
        EXPECT_EQ(1, *i % 2);
        count = *i;
    }

    count = 0;
    for (auto i = even.begin(); i != even.end(); ++i)
    {
        EXPECT_LT(count, *i);
        EXPECT_EQ(0, *i % 2);
        count = *i;
    }
}

TEST(atomic_test, buffer_operations)
{
	const int size = 9;
	marbles::atomic_buffer<int, size> buffer;

	int pop = 0;
	int push = 0;

	EXPECT_EQ(true, buffer.empty());

	while (buffer.try_push(push))
	{
		++push;
		EXPECT_EQ(push, (int)buffer.size());
	}

	EXPECT_EQ(true, buffer.full());
	EXPECT_EQ(push, size - 1);

	for (int i = 0; buffer.try_pop(pop); ++i)
	{
		EXPECT_EQ(pop, i);
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

TEST(atomic_test, queue_operations)
{
	const int size = 16;
	marbles::atomic_queue<int, size> queue;

	int pop = 0;
	int push = 0;

	EXPECT_TRUE(queue.empty());

	while (size != push)
	{
		queue.enqueue(push++);
	}

	EXPECT_FALSE(queue.empty());

	while (!queue.empty())
	{
		int value = 0;

		EXPECT_TRUE(queue.dequeue(value));
		EXPECT_EQ(pop++, value);
	}

	EXPECT_TRUE(queue.empty());

	for (int i = 3*size; --i;)
	{
		queue.enqueue(push++);
		queue.enqueue(push++);

		int value = 0;
		EXPECT_TRUE(queue.dequeue(value));
		EXPECT_EQ(pop++, value);
	}

	while (!queue.empty())
	{
		queue.enqueue(push++);

		int value = 0;
		if (queue.dequeue(value))
		{
			EXPECT_EQ(pop++, value);
		}
		if (queue.dequeue(value))
		{
			EXPECT_EQ(pop++, value);
		}
	}

	EXPECT_TRUE(queue.empty());
}

template<typename queue_t> void push_pop_multi()
{
	static const int32_t num_consumers = 3;
	static const int32_t num_producers = 10;
	static const int32_t num_produce = 1000;
	marbles::atomic<int32_t> consumed[num_producers];
	marbles::atomic<int32_t> consumer_end;
	marbles::atomic<int32_t> producer_end;
	std::thread workers[num_consumers + num_producers];

	marbles::atomic_queue<int32_t, 32> id_queue;
	marbles::atomic<int32_t> start = 0;

	int32_t num_workers = 0;
	for (int32_t i = num_consumers; i--;)
	{
		consumer_end.fetch_add(1);
		std::thread consumer([&]()
		{
			int32_t id = 0;
			while (0 <= producer_end.load() || !id_queue.empty())
			{
				if (id_queue.dequeue(id))
				{
					consumed[id].fetch_add(1);
				}
				else
				{
					rest_thread(id);
				}
			}
			consumer_end.fetch_add(-1);
		});
		workers[num_workers++].swap(consumer);
	}
	for (int32_t i = num_producers; i--;)
	{
		consumed[i].store(0);
		producer_end.fetch_add(1);

		std::thread producer([&]()
		{
			while (0 == start.load())
			{
				std::this_thread::sleep_for(std::chrono::microseconds(1));
			}

			int32_t id = 0; // select id
			do {
				id = start.load();
			} while (!start.compare_exchange_weak(id, id + 1));
			rest_thread(id);

			--id; // id should be in range [0 (num_producers - 1)]
			for (int32_t i = num_produce; i--;)
			{
				id_queue.enqueue(id);
				if (i % 5)
				{
					rest_thread(i);
				}
			}
			producer_end.fetch_add(-1);
		});
		workers[num_workers++].swap(producer);
	}

	start.store(1);

	for (int32_t i = 0; i < num_workers; ++i)
	{
		workers[i].join();
	}

	for (int32_t i = num_producers; i--;)
	{
		EXPECT_EQ(num_produce, consumed[i].load());
	}
}

TEST(atomic_test, buffer_push_pop_multi)
{
	const int32_t quantity = 150;
	const int32_t numProducers = 15;

	typedef marbles::array<int32_t, numProducers> tally_t;
	tally_t tally;
	for (auto& sum : tally)
	{
		sum = 0;
	}

	const size_t numConsumers = 6;
	marbles::array<tally_t, numConsumers> tallySheet;
	for (auto& count : tallySheet)
	{
		count = tally;
	}

	marbles::atomic<int> producerCount = numProducers;
	marbles::atomic_buffer<int32_t, quantity> data;

	marbles::array<std::thread, numProducers> producerThreads;
	for (auto id = numProducers; id--;)
	{
        std::thread producer([&producerCount, &data, id, quantity]()
		{
			for (auto i = quantity; i--;)
			{
                rest_thread(i);
                data.push(id);
			}
			producerCount--;
		});
		producerThreads[id].swap(producer);
	}

	marbles::array<std::thread, numConsumers> consumerThreads;
	for (auto id = numConsumers; id--;)
	{
		tally_t& count = tallySheet[id];
		std::thread consumer([&producerCount, &count, &data]()
		{
			int value = 0;
			while (0 != producerCount.load() || !data.empty())
			{
				rest_thread(value);
				if (data.try_pop(value))
				{
					++count[value];
				}
			}
		});
		consumerThreads[id].swap(consumer);
	}

	for (auto j = producerThreads.size(); j--;)
	{
		producerThreads[j].join();
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

TEST(atomic_test, queue_push_pop_multi)
{
	const int32_t quantity = 150;
	const int32_t numProducers = 15;

	typedef marbles::array<int32_t, numProducers> tally_t;
	tally_t tally;
	for (auto& sum : tally)
	{
		sum = 0;
	}

	const size_t numConsumers = 6;
	marbles::array<tally_t, numConsumers> tallySheet;
	for (auto& count : tallySheet)
	{
		count = tally;
	}

	marbles::atomic<int> producerCount = numProducers;
	marbles::atomic_queue<int32_t, 16> data;

	marbles::array<std::thread, numProducers> producerThreads;
	for (auto id = numProducers; id--;)
	{
		std::thread producer([&producerCount, &data, id, quantity]()
		{
			for (auto i = quantity; i--;)
			{
				rest_thread(i);
				data.enqueue(id);
			}
			producerCount--;
		});
		producerThreads[id].swap(producer);
	}

	marbles::array<std::thread, numConsumers> consumerThreads;
	for (auto id = numConsumers; id--;)
	{
		tally_t& count = tallySheet[id];
		std::thread consumer([&producerCount, &count, &data]()
		{
			int value = 0;
			while (0 != producerCount.load() || !data.empty())
			{
				rest_thread(value);
				if (data.dequeue(value))
				{
					++count[value];
				}
			}
		});
		consumerThreads[id].swap(consumer);
	}

	for (auto j = producerThreads.size(); j--;)
	{
		producerThreads[j].join();
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

TEST(atomic_test, multi_thread_usage)
{
	const int quantity = 150;
	const int bufferSize = 10;
	const int numProducers = 10;
	const int numBuffers = numProducers >> 1;

    typedef marbles::atomic_list<wrap<int>> List;
    typedef marbles::array<List::node_type, quantity*numProducers> NodeArray;

    typedef marbles::atomic_buffer<wrap<int>, bufferSize> Buffer;
	typedef marbles::array<Buffer, numProducers> BufferArray;
	typedef marbles::array<std::thread, numProducers> ThreadArray;
	typedef marbles::array<int, numProducers> TallyArray;

    List pool;
    NodeArray poolBuffer;
	TallyArray tally;
	ThreadArray threads;
	BufferArray buffers;
    List accumulation;

    {
        int id = 0;
        for (auto& node : poolBuffer)
        {
            *node.get() = id++;
            pool.insert_next(&node);
        }
    }

	for (auto& count : tally)
	{
		count = 0;
	}

	auto bufferAccumulator = [&buffers, &pool, &accumulation](int)
	{
		wrap<int> value = 0;
		unsigned index = 0;
		do
		{
			++index;
			index %= buffers.size();
		
            List::node_type* node;
            if (pool.remove_next(&node))
            {
                if (buffers[index].try_pop(value) && 0 <= value)
			    {
                    *node->get() = value;
				    accumulation.insert_next(node);
			    }
                else
                {
                    pool.insert_next(node);
                }
            }

            rest_thread(value);
		} while (0 <= value);
	};

    std::thread accumulatorThread([bufferAccumulator]{ bufferAccumulator(1); });
    std::thread accumulator2Thread([bufferAccumulator]{ bufferAccumulator(2); });
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
        List::node_type* node;
        if (accumulation.remove_next(&node))
		{
			++tally[*node->get()];
			++sum;

            pool.insert_next(node);
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
}

