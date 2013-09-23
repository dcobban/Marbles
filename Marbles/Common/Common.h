// This source file is part of Marbles library.
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
#include <array>
#include <limits>
#include <cassert>
#include <type_traits>

// We should not make these global includes if possible
#include <boost/cstdint.hpp>
#include <boost/interprocess/detail/atomic.hpp>
namespace boost
{
	using namespace interprocess::ipcdetail;
}

#include <Common/Function_Traits.h>
#include <Common/Definitions.h>

#define TO_STRING2(constant) #constant
#define TO_STRING(constant) TO_STRING2(constant)
#define Location(label) __FILE__ "(" TO_STRING(__LINE__) ") " label ": "
#define TODO(txt) Location("Todo") txt
#define Message(label, msg) message (Location(label) msg)

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{

// --------------------------------------------------------------------------------------------------------------------
#define STATIC_ASSERT BOOST_STATIC_ASSERT
#define ASSERT assert

// --------------------------------------------------------------------------------------------------------------------
template <typename T> 
inline size_t AlignOf() 
{ 
	return __alignof(T); // This is win32 specific
}

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
template<typename T> struct expired 
{ inline bool operator()(const std::tr1::weak_ptr<T>& p) const { return p.expired(); } };

// --------------------------------------------------------------------------------------------------------------------
inline void NoDelete(void*) {}
template<typename T> inline void Destruct(void* p) { p->~T(); }
template<typename T> inline void Delete(void* p) { delete reinterpret_cast<T*>(p); }
template<typename T> inline void Empty(T*) { }

#define CONSTRUCT_WITH_ARGS(N) \
	template<typename T, FN_TYPENAME(N)> \
	T* Construct(FN_PARAMETER(N)) { return new T(FN_ARGUMENTS(N)); } \
	template<typename T, FN_TYPENAME(N)> \
	T* Construct(void* p, FN_PARAMETER(N)) { return new (p) T(FN_ARGUMENTS(N)); } 

// CONSTRUCT_WITH_ARGS(0)
template<typename T>
T* Construct() { return new T(); }
template<typename T>
T* Construct(void* p) { return new (p) T(); }

CONSTRUCT_WITH_ARGS(1)
CONSTRUCT_WITH_ARGS(2)
CONSTRUCT_WITH_ARGS(3)
CONSTRUCT_WITH_ARGS(4)
CONSTRUCT_WITH_ARGS(5)
CONSTRUCT_WITH_ARGS(6)
CONSTRUCT_WITH_ARGS(7)
CONSTRUCT_WITH_ARGS(8)
CONSTRUCT_WITH_ARGS(9)
CONSTRUCT_WITH_ARGS(10)
CONSTRUCT_WITH_ARGS(11)
CONSTRUCT_WITH_ARGS(12)

#undef CONSTRUCT_WITH_ARGS

// --------------------------------------------------------------------------------------------------------------------
// to be replaced by std::unique_ptr<>
template<typename T> class unique_ptr : public std::auto_ptr<T>
{
public:
	explicit unique_ptr(T* p = 0) 
		:  std::auto_ptr<T>(p)
		{	// construct from object pointer
		}

	unique_ptr(const unique_ptr<T>& rhs) 
		:  std::auto_ptr<T>(const_cast<unique_ptr<T>&>(rhs))
		{	
		}

	unique_ptr(const std::auto_ptr_ref<T> rhs) 
		: std::auto_ptr<T>(rhs)
		{	
		}
};

// --------------------------------------------------------------------------------------------------------------------
// to be replaced by std::atomic<>
template<typename T>
class atomic
{
	typedef T					value_type;
	typedef atomic<value_type>	self_type;
	typedef value_type&			reference;
public:
	atomic() {}
	atomic(value_type value) : mValue(value) {}
	atomic(const self_type& rhs) : mValue(boost::atomic_read32(&rhs.mValue)) {}

	inline self_type& operator=(value_type value) 
	{ boost::atomic_write32(&mValue, static_cast<boost::uint32_t>(value)); return *this; }
	inline self_type& operator=(const self_type& rhs) 
	{ boost::atomic_write32(&mValue, boost::atomic_read32(&rhs.mValue)); return *this; }

	inline value_type get() 
	{ return static_cast<value_type>(boost::atomic_read32(&mValue)); }
	inline value_type get() const 
	{ return const_cast<self_type*>(this)->get(); }

	inline value_type compare_exchange(value_type value, value_type compare) 
	{ return static_cast<value_type>(boost::atomic_cas32(&mValue, static_cast<boost::uint32_t>(value), static_cast<boost::uint32_t>(compare))); }

	// value_type operator++() { return boost::atomic_increment(&mValue); }
private:
	mutable volatile boost::uint32_t mValue;
};

template<typename T>
class atomic<T*>
{
	typedef T*					value_type;
	typedef atomic<value_type>	self_type;
	typedef value_type&			reference;
public:
	atomic() {}
	atomic(value_type value) : mValue(reinterpret_cast<boost::uint32_t>(value)) {}
	atomic(const self_type& rhs) : mValue(boost::atomic_read32(&rhs.mValue)) {}
	
	inline self_type& operator=(value_type value) 
	{ Set(value); return *this; }
	inline self_type& operator=(const self_type& rhs) 
	{ Set(rhs); return *this; }

	// Set the value and return the previous value
	inline value_type& set(value_type value) 
	{ return reinterpret_cast<value_type>(boost::atomic_write32(&mValue, reinterpret_cast<boost::uint32_t>(value))); }
	inline value_type& set(const self_type& rhs) 
	{ return reinterpret_cast<value_type>(boost::atomic_write32(&mValue, boost::atomic_read32(&rhs.mValue))); }

	// get the value 
	inline value_type get() 
	{ return reinterpret_cast<value_type>(boost::atomic_read32(&mValue)); }
	inline value_type get() const 
	{ return const_cast<self_type*>(this)->get(); }

	// if value equal to 'compare' then set to 'newValue', always returning the previous value
	inline value_type compare_exchange(value_type newValue, value_type compare) 
	{ 
		return reinterpret_cast<value_type>(boost::atomic_cas32(
			&mValue, 
			reinterpret_cast<boost::uint32_t>(newValue), 
			reinterpret_cast<boost::uint32_t>(compare))); 
	}

	// Increment/Decrement operators
	inline value_type operator++() 
	{ return boost::atomic_inc32(&mValue); }
	inline value_type operator--() 
	{ return boost::atomic_dec32(&mValue); }
	inline value_type operator++(int) 
	{ return boost::atomic_inc32(&mValue) + 1; }
	inline value_type operator--(int) 
	{ return boost::atomic_dec32(&mValue) - 1; }
private:
	volatile boost::uint32_t mValue;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace Marbles

// --------------------------------------------------------------------------------------------------------------------
namespace std 
{ 
using namespace std::tr1; 

// --------------------------------------------------------------------------------------------------------------------
template<int condition, typename Then, typename Else> struct conditional { typedef Then type_info; };
template<typename Then, typename Else> struct conditional<0, Then, Else> { typedef Else type_info; };

// --------------------------------------------------------------------------------------------------------------------
#define MAKE_SHARED(N) \
	template<typename T, FN_TYPENAME(N)> \
	shared_ptr<T> make_shared(FN_PARAMETER(N)) { return shared_ptr<T>(new T(FN_ARGUMENTS(N))); } \

// MAKE_SHARED(0)
template<typename T>
shared_ptr<T> make_shared() { return shared_ptr<T>(new T()); }

MAKE_SHARED(1)
MAKE_SHARED(2)
MAKE_SHARED(3)
MAKE_SHARED(4)
MAKE_SHARED(5)
MAKE_SHARED(6)
MAKE_SHARED(7)
MAKE_SHARED(8)
MAKE_SHARED(9)
MAKE_SHARED(10)
MAKE_SHARED(11)
MAKE_SHARED(12)
	
#undef MAKE_SHARED
}

// End of file --------------------------------------------------------------------------------------------------------
