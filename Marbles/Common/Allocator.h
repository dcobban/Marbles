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

#include <Common/Common.h>
#include <Common/AtomicList.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
template<size_t block_size = 64>
class block_allocator
{
public:
    block_allocator() 
    : _free(nullptr)
    , _outstanding(0)
    {
    }

    ~block_allocator()
    {
        ASSERT(_outstanding == 0);
        release();
    }

    // Can a block be allocated
    bool can_allocate() const 
    {
        return nullptr != _free.load();
    }

	// Total number of blocks available for allocation
    int32_t num_available() const
    {
		int32_t count = 0;
		block_t* head = nullptr;
		do {
			count = 0;
			head = _free.load();
			block_t* next = head;
			while (nullptr != next)
			{
                ++count;
				next = next->_next.load();
			}
		} while (head != _free.load()); // Another thread changed the free list

		return count;
    }

    // Total number of blocks managed by this allocator
    int32_t num_reserved() const
    {
        return num_available() + _outstanding.load();
    }

    // Increases the number of blocks available for allocation
    bool reserve(int32_t count)
    {
		allocator<int8_t> blockAllocator;
        block_t* block = nullptr;
        do { 
			block = reinterpret_cast<block_t*>(blockAllocator.allocate(sizeof(block_t)));

            push_block(block);
            if (nullptr != block)
            {
                count = count - 1;
            }
        } while (nullptr != block && 0 != count);

        return nullptr != block;
    }

    // Release the number of given blocks or release all blocks in reserve
    // @param count     release on 'count' number of buffers. if count <= 0 all buffers are removed.
    // @return          number of buffers released from the pool
    int32_t release(int32_t count = 0) 
    {
		allocator<int8_t> blockAllocator;
        int32_t pop_count = 0;
        block_t* head = nullptr;
        do 
        {
            head = pop_block();
            if (nullptr != head)
            {
                --count;
				++pop_count;
				blockAllocator.deallocate(reinterpret_cast<int8_t*>(head), sizeof(block_t));
            }
        } while (nullptr != head && 0 != count);

        return pop_count;
    }

    // Allocate a new object from the pool 
    template<typename T, typename ...args> T* allocate(args...)
    {
        STATIC_ASSERT(sizeof(T) <= block_size);
        block_t* head = pop_block();
        if (nullptr != head)
        {
            ++_outstanding;
			allocator<T> tAllocator;
            T* out = reinterpret_cast<T*>(head);
            allocator_traits<allocator<T>>::construct(tAllocator, out, args...);
            return out;
        }
        return nullptr;
    }

    // Free the given pointer to the pool
    template<typename T> bool free(T* item)
    {
        if (nullptr != item)
        {
			allocator<T> tAllocator;
            allocator_traits<allocator<T>>::destroy(tAllocator, item);
			--_outstanding;

            // TODO: Validate that this block was previously owned by the allocator?
            block_t* blockItem = reinterpret_cast<block_t*>(item);
            push_block(blockItem);
        }
	    return nullptr != item;
    }

protected:

	static const size_t page_size = 4 * kb;
	static_assert(block_size <= (page_size >> 1), "Blocks must be smaller than the page size. (BLOCK_SIZE <= (page_size >> 1)");
	static_assert(0 == page_size % block_size, "page_size must be be divisable by BLOCK_SIZE (0 == page_size % BLOCK_SIZE)");
	union block_t
	{
		atomic<block_t*> _next;
		ubyte_t _block[block_size];
	};

    block_t* pop_block()
    {
		block_t* head = nullptr;
		do {
			head = _free.load();
		} while (nullptr != head && !_free.compare_exchange_weak(head, head->_next.load()));
        return head;
    }

    void push_block(block_t* block)
    {
        if (nullptr != block)
        {
		    block_t* head = nullptr;
            do {
                head = _free.load();
                block->_next = head;
            } while (!_free.compare_exchange_weak(head, block));
		}
    }

    atomic<block_t*> _free;
    atomic<int32_t> _outstanding;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
