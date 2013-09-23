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

// --------------------------------------------------------------------------------------------------------------------
// REFLECT_TYPE(void,)
template<> struct ::Marbles::reflection::type_of_t<void> 
: public ::Marbles::reflection::instance_t< ::Marbles::reflection::type_of_t<void> > 
{
	typedef void type_info; 
	static ::Marbles::reflection::shared_type typeInfo() 
	{ 
		return ::Marbles::reflection::shared_type(); 
	} 
}; 

// --------------------------------------------------------------------------------------------------------------------
REFLECT_TYPE(Marbles::reflection::type_info, REFLECT_CREATOR())

// --------------------------------------------------------------------------------------------------------------------
REFLECT_TYPE(bool,				REFLECT_CREATOR())
REFLECT_TYPE(Marbles::uint8_t,	REFLECT_CREATOR())
REFLECT_TYPE(Marbles::uint16_t,	REFLECT_CREATOR())
REFLECT_TYPE(Marbles::uint32_t,	REFLECT_CREATOR())
REFLECT_TYPE(Marbles::uint64_t,	REFLECT_CREATOR())
REFLECT_TYPE(Marbles::int8_t,	REFLECT_CREATOR())
REFLECT_TYPE(Marbles::int16_t,	REFLECT_CREATOR())
REFLECT_TYPE(Marbles::int32_t,	REFLECT_CREATOR())
REFLECT_TYPE(Marbles::int64_t,	REFLECT_CREATOR())
REFLECT_TYPE(float,				REFLECT_CREATOR())
REFLECT_TYPE(double,			REFLECT_CREATOR())
REFLECT_TYPE(std::string,		REFLECT_CREATOR())
REFLECT_TYPE(Marbles::reflection::object,REFLECT_CREATOR())

// --------------------------------------------------------------------------------------------------------------------
REFLECT_TEMPLATE_TYPE(template<typename T>, std::char_traits<T>,)
REFLECT_TEMPLATE_TYPE(template<typename T>, std::allocator<T>,	REFLECT_CREATOR())
REFLECT_TEMPLATE_TYPE(template<typename T>, std::shared_ptr<T>,	REFLECT_CREATOR())
REFLECT_TEMPLATE_TYPE(template<typename T>, std::weak_ptr<T>,	REFLECT_CREATOR())

// --------------------------------------------------------------------------------------------------------------------
REFLECT_TEMPLATE_TYPE(template<typename T RFFLECT_COMMA typename A>, 
					  std::vector<T RFFLECT_COMMA A>, 
					  REFLECT_CREATOR())

// End of file --------------------------------------------------------------------------------------------------------
