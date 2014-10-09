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

#include <memory>
#include <functional>
#include <algorithm>
#include <vector>
#include <atomic>
#include <array>
#include <limits>
#include <cassert>
#include <type_traits>
#include <tuple>

// We should not make these global includes if possible
#include <cstdint>

#include <Common/Function_Traits.h>
#include <Common/Definitions.h>
#include <Common/Memory.h>

#define TO_STRING2(constant) #constant
#define TO_STRING(constant) TO_STRING2(constant)
#define Location(label) __FILE__ "(" TO_STRING(__LINE__) ") " label ": "
#define TODO(txt) Location("Todo") txt
#define Message(label, msg) message (Location(label) msg)

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
// --------------------------------------------------------------------------------------------------------------------
template<int ...> struct seq {};
template<int N, int ...S> struct gens : gens<N - 1, N - 1, S...> {};
template<int ...S> struct gens<0, S...> { typedef seq<S...> type; };

// --------------------------------------------------------------------------------------------------------------------
#define STATIC_ASSERT BOOST_STATIC_ASSERT
#define ASSERT assert

// --------------------------------------------------------------------------------------------------------------------
template <typename T> 
inline size_t AlignTo(size_t value) 
{ 
	return ((value + (AlignOf<T>() - 1)) & ~AlignOf<T>()); 
}

// --------------------------------------------------------------------------------------------------------------------
template <typename A>
inline const A& Max(const A& a, const A& b) { return a > b ? a : b; }
template <typename A>
inline const A& Min(const A& a, const A& b) { return a < b ? a : b; }

// --------------------------------------------------------------------------------------------------------------------
template<typename T> inline void Destruct(T* p) { p->~T(); }
template<typename T> inline void Delete(T* p) { delete p; }

template<typename T, typename... Args>
T* Construct(Args&&... args) { return new T(forward<Args>(args)...)}
template<typename T, typename... Args>
T* Construct(void* p, Args&&... args) { return new (p) T(forward<Args>(args)...)}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
