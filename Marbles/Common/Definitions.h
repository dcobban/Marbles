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

#include <cstdint>

// --------------------------------------------------------------------------------------------------------------------
#define TO_STRING2(label) #label
#define TO_STRING(label) TO_STRING2(label)
#define SOURCE_LOCATION(label) __FILE__ "(" TO_STRING(__LINE__) "): " label "- "
#define TODO(txt) SOURCE_LOCATION("Todo") txt
#define MESSAGE(label, msg) message (SOURCE_LOCATION(label) msg)
#define TRACE_INFO(label, x, ...) printf("[%s] " x, label, __VA_ARGS__)

// --------------------------------------------------------------------------------------------------------------------
#define STATIC_ASSERT BOOST_STATIC_ASSERT
#if defined NDEBUG
#define ASSERT
#define VERIFY(x) (x)
#else
#define ASSERT assert
#define VERIFY(x) ASSERT(x)
#endif 

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
// --------------------------------------------------------------------------------------------------------------------
using	std::int8_t;
using	std::int16_t;
using	std::int32_t;
using	std::int64_t;
using	std::uint8_t;
using	std::uint16_t;
using	std::uint32_t;
using	std::uint64_t;

typedef char	byte_t;
typedef uint8_t	ubyte_t;
typedef float	float32_t;
typedef double	float64_t;
typedef void*	handle;

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
