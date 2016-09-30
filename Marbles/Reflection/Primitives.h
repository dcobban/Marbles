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

// --------------------------------------------------------------------------------------------------------------------
// REFLECT_TYPE(void,)
template<> struct ::marbles::reflection::type_of_t<void> 
: public ::marbles::reflection::instance_t< ::marbles::reflection::type_of_t<void> > 
{
	typedef void type_info; 
	static ::marbles::reflection::shared_type typeInfo() 
	{ 
		return ::marbles::reflection::shared_type(); 
	} 
}; 

// --------------------------------------------------------------------------------------------------------------------
REFLECT_TYPE(marbles::reflection::type_info, REFLECT_CREATOR())

// --------------------------------------------------------------------------------------------------------------------
REFLECT_TYPE(bool,				REFLECT_CREATOR())
REFLECT_TYPE(marbles::byte_t,	REFLECT_CREATOR())
REFLECT_TYPE(marbles::int8_t,	REFLECT_CREATOR())
REFLECT_TYPE(marbles::int16_t,	REFLECT_CREATOR())
REFLECT_TYPE(marbles::int32_t,	REFLECT_CREATOR())
REFLECT_TYPE(marbles::int64_t,	REFLECT_CREATOR())
REFLECT_TYPE(marbles::uint8_t,	REFLECT_CREATOR())
REFLECT_TYPE(marbles::uint16_t, REFLECT_CREATOR())
REFLECT_TYPE(marbles::uint32_t, REFLECT_CREATOR())
REFLECT_TYPE(marbles::uint64_t, REFLECT_CREATOR())
REFLECT_TYPE(marbles::float32_t, REFLECT_CREATOR())
REFLECT_TYPE(marbles::float64_t, REFLECT_CREATOR())
REFLECT_TYPE(marbles::string,	REFLECT_CREATOR())
REFLECT_TYPE(marbles::reflection::object,REFLECT_CREATOR())

// --------------------------------------------------------------------------------------------------------------------
REFLECT_TEMPLATE_TYPE(template<typename T>, marbles::char_traits<T>, )
REFLECT_TEMPLATE_TYPE(template<typename T>, marbles::allocator<T>, REFLECT_CREATOR())
REFLECT_TEMPLATE_TYPE(template<typename T>, marbles::unique_ptr<T>, REFLECT_CREATOR())
REFLECT_TEMPLATE_TYPE(template<typename T>, marbles::shared_ptr<T>, REFLECT_CREATOR())
REFLECT_TEMPLATE_TYPE(template<typename T>, marbles::weak_ptr<T>, REFLECT_CREATOR())
//REFLECT_TEMPLATE_TYPE(template<int N>, char[N], )
template<int N>
struct ::marbles::reflection::type_of_t<char[N]> 
: public ::marbles::reflection::instance_t< ::marbles::reflection::type_of_t<char[N]> > 
{	
	static ::marbles::reflection::shared_type typeInfo() 
	{ 
		::marbles::reflection::shared_type typeInfo = get().reflect_type.lock(); 
		if (!typeInfo) 
			typeInfo = get().create(); 
		return typeInfo; 
	} 
private: 
	::marbles::reflection::weak_type reflect_type; 
public: 
	::marbles::reflection::shared_type create() 
	{ 
		typedef char self_type[N];
		::marbles::reflection::type_info::builder build; 
		reflect_type = build.create<char[N]>("char[" TO_STRING(N) "]");
		// REFLECT_DEFINITION 
		return build.typeInfo(); 
	} 
}; 

// --------------------------------------------------------------------------------------------------------------------
REFLECT_TEMPLATE_TYPE(template<typename T REFLECT_COMMA typename A>,
                      marbles::vector<T REFLECT_COMMA A>,
					  REFLECT_CREATOR())

// End of file --------------------------------------------------------------------------------------------------------
