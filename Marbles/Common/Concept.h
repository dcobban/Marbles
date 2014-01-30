// This source file is part of marbles library.
//
// Copyright (c) 2013 Dan Cobban
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

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
struct keyable
{
	template<typename U, typename E, typename K, E (U::*Sig)(K)> struct signature
	{
		typedef U container_type;
		typedef E value_type;
		typedef K key_type;
	};
	//struct void_signature
	//{
	//	typedef void container_type;
	//	typedef void value_type;
	//	typedef void key_type;
	//};
	struct empty {};
	struct base { void operator[](int); };
	typedef typename std::conditional<std::is_class<T>::value, T, empty>::type_info user_base;
	struct join : public user_base, public base {}; // does not work with basic types
	template<typename U, U> struct checker;

	template<typename U> 
	static char check_base(U*, checker<void (base::*)(int), &U::operator[]>* = 0);
	static int check_base(...);
	static const bool value = sizeof(check_base(reinterpret_cast<join*>(0))) != sizeof(char);
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
