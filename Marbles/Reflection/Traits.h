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
#include <algorithm>
#include <type_traits>
#include <Common/Function_Traits.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
namespace reflection
{
class type_info;
typedef shared_ptr<const type_info> shared_type;
typedef weak_ptr<const type_info> weak_type;
typedef uint32_t hash;

// --------------------------------------------------------------------------------------------------------------------
// Review(danc): should we create a 'Common\type_traits' for these?
template<typename T> struct remove_all_ptrs						{ typedef T type; };
template<typename T> struct remove_all_ptrs<T*>					{ typedef typename remove_all_ptrs<T>::type type; };
template<typename T> struct remove_all_ptrs<T* const>			{ typedef typename remove_all_ptrs<T>::type type; };
template<typename T> struct remove_all_ptrs<T* volatile>		{ typedef typename remove_all_ptrs<T>::type type; };
template<typename T> struct remove_all_ptrs<T* const volatile>	{ typedef typename remove_all_ptrs<T>::type type; };

// --------------------------------------------------------------------------------------------------------------------
// Review(danc): should we create a 'Common\type_traits' for these?
template<typename T> struct remove_all_cv						{ typedef typename remove_cv<T>::type type; };
template<typename T> struct remove_all_cv<T*>					{ typedef typename remove_all_cv<T>::type* type; };
template<typename T> struct remove_all_cv<T* const>				{ typedef typename remove_all_cv<T>::type* type; };
template<typename T> struct remove_all_cv<T* volatile>			{ typedef typename remove_all_cv<T>::type* type; };
template<typename T> struct remove_all_cv<T* const volatile>	{ typedef typename remove_all_cv<T>::type* type; };

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
struct by_value
{
private:
	typedef typename remove_reference<T>::type					ref_removed;
	typedef typename remove_all_ptrs<typename ref_removed>::type	ref_ptr_removed;
	typedef typename remove_cv<typename ref_ptr_removed>::type	ref_ptr_cv_removed;
public:
	typedef typename ref_ptr_cv_removed type;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
struct HasTypeInfo
{
	template<typename U, shared_type (U::*)() const> struct signature {};
	template<typename U> static int HasMethod(signature<U, &U::_type_info>*);
	template<typename U> static char HasMethod(...);
	static const bool value = sizeof(HasMethod<T>(0)) != sizeof(char);
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
struct HasIndexer
{
	template<typename U, typename V, typename K, V (U::*)(K)> struct signature
	{
		typedef typename U container_type;
		typedef typename V value_type;
		typedef typename K key_type;
	};
	struct void_signature
	{
		typedef void container_type;
		typedef void value_type;
		typedef void key_type;
	};
	template<typename U, typename V, typename K> static signature<U, V, K, &U::operator[]>* IndexMethod(signature<U, V, K, &U::operator[]>*);
	template<typename U> static char IndexMethod(...);
	static const bool value = sizeof(IndexMethod<T>(0)) != sizeof(char);

	// C++11 required
	//typedef typename result_of< IndexMethod<T>(0) >::type result_type
	//typedef typename conditional<value, void_signature, typename result_type>::type signature_traits;
	//typedef typename signature_traits::container_type container_type;
	//typedef typename signature_traits::value_type value_type;
	//typedef typename signature_traits::key_type key_type;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
struct HasEnumerator
{
	template<typename U, typename U::iterator (U::*)(), typename U::iterator (U::*)()> struct signature
	{
		typedef U container_type;
		typedef typename U::iterator iterator;
	};
	template<typename U> static signature<U, &U::begin, &U::end>* BeginEnd(signature<U, &U::begin, &U::end>*);
	template<typename U> static char BeginEnd(...);
	static const bool value = sizeof(BeginEnd<T>(0)) != sizeof(char);

	// C++11 required
	//typedef decltype(HasMethod<T>(0)) signature_type;
	//typedef signature_type::container_type container_type;
	//typedef signature_type::value_type value_type;
	//typedef signature_type::key_type key_type;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
struct SupportTraits
{
	static const bool isExternal	= HasTypeInfo<T>::value;
	static const bool canEnumerate	= HasEnumerator<T>::value;	// supports begin/end iteration
	static const bool canIndex		= HasIndexer<T>::value;		// can be indexed by a key operator[]
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
