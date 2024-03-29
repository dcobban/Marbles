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

#include <common/function_traits.h>
#include <common/definitions.h>
#include <common/memory.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
template<int ...> struct seq {};
template<int N, int ...S> struct gens : gens<N - 1, N - 1, S...> {};
template<int ...S> struct gens<0, S...> { typedef seq<S...> type; };

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
template <typename A>
inline const A& Select(const bool condition, const A& a, const A& b) { return condition ? a : b; }

// --------------------------------------------------------------------------------------------------------------------
template<typename T> inline void Destruct(T* p) { p->~T(); }
template<typename T> inline void Delete(T* p) { delete p; }

template<typename T, typename... Args>
T* Construct(Args&&... args) { return new T(forward<Args>(args)...); }
template<typename T, typename... Args>
T* Construct(void* p, Args&&... args) { return new (p) T(forward<Args>(args)...); }

// --------------------------------------------------------------------------------------------------------------------
template<class _Ty>
bool atomic_swap(atomic<_Ty>& a, atomic<_Ty>& b)
{
    _Ty localA;
    _Ty localB;
    do {
        localA = a.load();
        localB = b.load();
        if (localA == atomic_exchange_strong(&a, &localA, localB))
        {
            if (localB == atomic_compare_exchange_strong(&b, &localB, localA))
            {
                break;
            }
            else if (localB == atomic_compare_exchange_strong(&a, &localB, localB))
            {

            }
        }
    } while (true);
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
