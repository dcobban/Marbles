// This source file is part of marbles library.
//
// Copyright (c) 2014 Dan Cobban
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

#include <Common/Allocator.h>

// --------------------------------------------------------------------------------------------------------------------
TEST(block_allocator, basic_operations)
{
	static const int32_t block_size = 16;
	marbles::block_allocator<block_size> blocksOf16;

	EXPECT_EQ(0, blocksOf16.num_available());
	EXPECT_FALSE(blocksOf16.can_allocate());
	EXPECT_EQ(nullptr, blocksOf16.allocate<int32_t>());

	blocksOf16.reserve(1);
	EXPECT_TRUE(blocksOf16.can_allocate());
	EXPECT_EQ(1, blocksOf16.num_available());
	EXPECT_EQ(1, blocksOf16.num_reserved());

	int32_t* block = blocksOf16.allocate<int32_t>();
	EXPECT_TRUE(nullptr != block);
	EXPECT_FALSE(blocksOf16.can_allocate());
	EXPECT_EQ(0, blocksOf16.num_available());
	EXPECT_EQ(1, blocksOf16.num_reserved());

	int32_t* block2 = blocksOf16.allocate<int32_t>();
	EXPECT_EQ(nullptr, block2);
	EXPECT_FALSE(blocksOf16.can_allocate());
	EXPECT_EQ(0, blocksOf16.num_available());
	EXPECT_EQ(1, blocksOf16.num_reserved());

	blocksOf16.free(block);
	EXPECT_TRUE(blocksOf16.can_allocate());
	EXPECT_EQ(1, blocksOf16.num_available());
	EXPECT_EQ(1, blocksOf16.num_reserved());

	struct badBlock { int32_t mem[block_size]; }; // larger than block size
	//badBlock* block3 = blocksOf16.allocate<badBlock>(); // uncomment to cause static_assert

	int32_t* block4 = blocksOf16.allocate<int32_t>();
	EXPECT_TRUE(nullptr != block4);
	EXPECT_FALSE(blocksOf16.can_allocate());
	EXPECT_EQ(0, blocksOf16.num_available());
	EXPECT_EQ(1, blocksOf16.num_reserved());

	EXPECT_EQ(0, blocksOf16.release());
	EXPECT_FALSE(blocksOf16.can_allocate());
	EXPECT_EQ(0, blocksOf16.num_available());
	EXPECT_EQ(1, blocksOf16.num_reserved());

	EXPECT_TRUE(blocksOf16.reserve(2));
	EXPECT_TRUE(blocksOf16.can_allocate());
	EXPECT_EQ(2, blocksOf16.num_available());
	EXPECT_EQ(3, blocksOf16.num_reserved());

	EXPECT_EQ(1, blocksOf16.release(1));
	EXPECT_TRUE(blocksOf16.can_allocate());
	EXPECT_EQ(1, blocksOf16.num_available());
	EXPECT_EQ(2, blocksOf16.num_reserved());

	EXPECT_TRUE(blocksOf16.free(block4));
	EXPECT_TRUE(blocksOf16.can_allocate());
	EXPECT_EQ(2, blocksOf16.num_available());
	EXPECT_EQ(2, blocksOf16.num_reserved());

	EXPECT_EQ(2, blocksOf16.release());
	EXPECT_FALSE(blocksOf16.can_allocate());
	EXPECT_EQ(0, blocksOf16.num_available());
	EXPECT_EQ(0, blocksOf16.num_reserved());
}

// End of file --------------------------------------------------------------------------------------------------------
