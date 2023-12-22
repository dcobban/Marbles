// This source file is part of marbles library.
//
// Copyright (c) 2023 Dan Cobban
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
#include <Common/Allocator.h>
#include <Common/AtomicBuffer.h>
#include <Common/AtomicList.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
	template<typename T, int block_size = 64>
	class atomic_queue
	{
	public:
		atomic_queue()
		{
			clear();
		}

		~atomic_queue()
		{
			// Disable queue
			buffer_node* head = _head.exchange(nullptr); 
			_tail.exchange(nullptr);

			// Clear outstanding buffers
			buffer_node* release_node = nullptr;
			do 
			{
				release_node = head;
				head = head->next();
				_pool.free(release_node);
			} while (nullptr != head);

			// Release all reserved blocks not actively in use
			_pool.release();
		}

		void enqueue(const T& item)
		{
			buffer_node* tail = _tail.load();
			while (!tail->get()->try_push(item))
			{
				if (!_pool.can_allocate())
				{
					_pool.reserve(1);
				}

				buffer_node* queue = _pool.allocate<buffer_list::node>();
				if (nullptr != queue && _tail.compare_exchange_strong(tail, queue))
				{
					tail->set_next(queue); // point previous tail to queue, the new tail
				}
				else if (nullptr != queue)
				{
					_pool.free(queue);
				}
				tail = _tail.load();
			}
		}

		bool dequeue(T& item)
		{
			buffer_node* head = _head.load();

			while (!head->get()->try_pop(item))
			{
				buffer_node* next = head->next();
				if (nullptr == next)
				{
					return false;
				}

				assert(next != nullptr); // _head cannot be nullptr must be valid at all times.
				if (_head.compare_exchange_strong(head, next))
				{
					// ********************************
					// Freeing 'head' could have multiple pop requests fail
					// ********************************
					_pool.free(head); 
				}
				head = _head.load();
			}

			return true;
		}

		bool empty() const
		{
			const buffer_node* tail = _tail.load();
			return nullptr == tail || tail->get()->empty();
		}

		void clear()
		{
			buffer_node* queue = nullptr;
			buffer_node* tail = nullptr;
			// Replace head/tail with new empty buffer to clear the queue
			do {
				tail = _tail.load();
				queue = _pool.allocate<buffer_list::node>();
				if (nullptr != queue && _tail.compare_exchange_strong(tail, queue))
				{	// Success! _tail points to an allocated buffer, queue is not considered empty
					_head.exchange(queue); // Set _head, queue can now dequeue correctly
				}
				else if (nullptr != queue)
				{	// Failed to set head, free the candidate
					_pool.free(queue);
				}
				else if (!_pool.can_allocate())
				{	// Failed to allocate reserve a buffer for allocation
					_pool.reserve(1);
				}
			} while (nullptr == queue);

			// Tail points previous buffer_node list free them all!
			while (nullptr != tail)
			{
				queue = tail;
				tail = tail->next();

				_pool.free(queue);
			}
		}

	private:
		typedef atomic_buffer<T, block_size> queue_buffer;
		typedef atomic_list<queue_buffer> buffer_list;
		typedef buffer_list::node buffer_node;
		typedef block_allocator<bit_ceil(sizeof(buffer_list::node))> pool_allocator;

		static pool_allocator _pool;
		atomic<typename buffer_node*> _tail = nullptr;
		atomic<typename buffer_node*> _head = nullptr;
	};

	// ----------------------------------------------------------------------------------------------------------------
	template<typename T, int block_size> atomic_queue<T, block_size>::pool_allocator atomic_queue<T, block_size>::_pool;

} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
