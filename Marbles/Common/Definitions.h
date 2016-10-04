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
#include <memory>
#include <functional>
#include <algorithm>
#include <vector>
#include <atomic>
#include <array>
#include <limits>
#include <cassert>
#include <type_traits>
#include <future>

// --------------------------------------------------------------------------------------------------------------------
#define TO_STRING2(label) #label
#define TO_STRING(label) TO_STRING2(label)
#define SOURCE_LOCATION(label) __FILE__ "(" TO_STRING(__LINE__) "): " label "- "
#define TODO(txt) SOURCE_LOCATION("Todo") txt
#define MESSAGE(label, msg) message (SOURCE_LOCATION(label) msg)
#define TRACE_INFO(label, x, ...) printf("[%s] " x, label, __VA_ARGS__)

// --------------------------------------------------------------------------------------------------------------------
#define STATIC_ASSERT static_assert
#if defined NDEBUG
#define ASSERT
#define VERIFY(x) (x)
#else
#define ASSERT assert
#define VERIFY(x) ASSERT(x)
#endif 

// --------------------------------------------------------------------------------------------------------------------
#define kb 1024
#define mb (kb*kb)

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
// --------------------------------------------------------------------------------------------------------------------
using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

typedef char	char_t;
typedef char	byte_t;
typedef uint8_t	ubyte_t;
typedef float	float32_t;
typedef double	float64_t;
typedef void*	handle;

// --------------------------------------------------------------------------------------------------------------------
using std::array;
using std::async;
using std::atomic;
using std::alignment_of;
using std::allocator;
using std::char_traits;
using std::const_pointer_cast;
namespace chrono = std::chrono;
using std::ctype;
using std::conditional;
using std::enable_shared_from_this;
using std::endl;
using std::forward;
using std::function;
using std::future;
using std::ios;
using std::is_class;
using std::is_const;
using std::is_function;
using std::is_member_function_pointer;
using std::istream;
using std::launch;
using std::locale;
using std::make_shared;
using std::make_unique;
using std::move;
using std::numeric_limits;
using std::ostream;
using std::remove_cv;
using std::remove_reference;
using std::remove_const;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::stringstream;
using std::swap;
namespace this_thread = std::this_thread;
using std::unique_ptr;
using std::use_facet;
using std::vector;
using std::weak_ptr;

} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
