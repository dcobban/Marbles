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
template<size_t BLOCK_SIZE>
class block_allocator : public allocator<void>
{
public:
    block_allocator() : _alloc(nullptr)
    {}
    ~block_allocator()
    {
        ::free(_alloc);
    }

    size_t capacity()
    {
        return _blocks; 
    }

    bool reserve_blocks(size_t count)
    {
        if (nullptr == _alloc)
        {
            _alloc = ::malloc(size);
        }
    }

protected:
    static const size_t page_size = 4*kb;
    static_assert(BLOCK_SIZE <= (page_size >> 1), "Blocks must be smaller than the page size. (BLOCK_SIZE <= (page_size >> 1)");
    static_assert(0 == page_size % BLOCK_SIZE, "page_size must be be divisable by BLOCK_SIZE (0 == page_size % BLOCK_SIZE)");
    union block_t
    {
        atomic<block_t*> _next;
        ubyte_t _block[BLOCK_SIZE];
    };
    atomic<block_t*> _free;
    atomic<int32_t> _outstanding;
    ubyte_t* _alloc;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
