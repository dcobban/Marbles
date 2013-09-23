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

#include <type_traits>
#include <ios>

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{
namespace reflection
{

// --------------------------------------------------------------------------------------------------------------------
struct type_def_base
{
	virtual bool			valid() const							{ return 0 != size(); }
	virtual size_t			number_of_parameters() const			{ return 0; }
	virtual type_def_base	parameter_at(size_t /*index*/) const	{ return type_def_base; }
	virtual std::ostream&	signature(std::ostream& name) const		{ return name; }

	virtual shared_type		type_info() const	{ return shared_type(); }
	virtual const bool		is_constant() const	{ return false; } 
	virtual const bool		is_volatile() const	{ return false; } 
	virtual const bool		is_function() const	{ return false; }
	virtual const size_t	alignment() const	{ return std::alignment_of<type_def_base>::value; }
	virtual const size_t	size() const		{ return 0; }
	virtual const size_t	rank() const		{ return 0; }
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> struct type_def : public type_def_base
{
	std::ostream&	signature(std::ostream& name) const { return name << type_info()->name(); }

	shared_type		type_info() const	{ return type_of<T>(); }
	const size_t	alignment() const	{ return std::alignment_of<std::remove_reference<T>::value>::value; }
	const size_t	size() const		{ return sizeof(T); }

	const bool		is_constant() const	{ return std::is_const<T>::value; }
private:
	static const size_t rank_value = 0;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> struct type_def<T*> : public type_def_base
{
	std::ostream&	signature(std::ostream& os) const { return os << type_info()->name() << "*"; }

	shared_type		type_info() const	{ return type_of<T>(); }
	const size_t	alignment() const	{ return std::alignment_of<T*>::value; }
	const size_t	size() const		{ return sizeof(T*); }
	const size_t	rank() const		{ return rank_value; }

private:
	static const size_t rank_value = type_def<T>::rank_value + 1;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> struct type_def<T* const> : public type_def_base
{
	std::ostream&	signature(std::ostream& os) const { return os << type_info()->name() << "* const"; }

	shared_type		type_info() const	{ return type_of<T>(); }
	const size_t	alignment() const	{ return std::alignment_of<T* const>::value; }
	const size_t	size() const		{ return sizeof(T* const); }
	const size_t	rank() const		{ return rank_value; }

	const bool		is_constant() const	{ return true; }
private:
	static const size_t rank_value = type_def<T>::rank_value + 1;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> struct type_def<T* volatile> : public type_def_base
{
	std::ostream&	signature(std::ostream& os) const { return os << type_info()->name() << "* volatile"; }

	shared_type		type_info() const	{ return type_of<T>(); }
	const size_t	alignment() const	{ return std::alignment_of<T* volatile>::value; }
	const size_t	size() const		{ return sizeof(T* volatile); }
	const size_t	rank() const		{ return rank_value; }

	const bool		is_volatile() const	{ return true; }
private:
	static const size_t rank_value = type_def<std::remove_pointer<T* volatile>::value>::rank_value + 1;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> struct type_def<T* const volatile> : public type_def_base
{
	std::ostream&	signature(std::ostream& os) const { return os << type_info()->name() << "* const volatile"; }

	shared_type		type_info() const	{ return type_of<T>(); }
	const size_t	alignment() const	{ return std::alignment_of<T* const volatile>::value; }
	const size_t	size() const		{ return sizeof(T* const volatile); }
	const size_t	rank() const		{ return rank_value; }

	const bool		is_volatile() const	{ return true; }
	const bool		is_constant() const	{ return true; }
private:
	static const size_t rank_value = type_def<std::remove_pointer<T* volatile>::value>::rank_value + 1;
};

// --------------------------------------------------------------------------------------------------------------------
// Review(danc): This only works for type_info parameters not numeric template parameters
#define BUILD_TYPE_DEF_PREFIX ::Marbles::reflection::type_of<A
#define BUILD_TYPE_DEF_POSTFIX >()
#define BUILD_TYPE_DEF_TYPENAME ->name()
#define BUILD_TYPE_DEF_SEP << ',' <<
#define BUILD_TYPE_DEF_LIST(N) FN_LIST(N,BUILD_TYPE_DEF_PREFIX,BUILD_TYPE_DEF_POSTFIX BUILD_TYPE_DEF_TYPENAME,BUILD_TYPE_DEF_SEP)
#define BUILD_PARAMETERS_PREFIX ::Marbles::reflection::type_def<A
#define BUILD_PARAMETERS_POSTFIX >()
#define BUILD_PARAMETER_LIST(N) FN_LIST(N,BUILD_PARAMETERS_PREFIX,BUILD_PARAMETERS_POSTFIX,FN_COMMA)
#define BUILD_TYPE_DEF(N) \
	template<template<FN_LIST(N,typename B,,FN_COMMA)> class T, FN_TYPENAME(N)> \
	struct type_def< T<FN_TYPES(N)> > \
	{ \
		size_t			number_of_parameters() const		{ return N; } \
		type_def_base	parameter_at(size_t index) const	{ return type_def_base; } \
		{ \
			type_def_base parameters[N] = { BUILD_PARAMETER_LIST(N) }; \
			return (0 <= index && index < N) ? parameters[index] : type_def_base(); \
		} \
		std::ostream&	signature(std::ostream& os) const \
		{ \
			return os << sName << '<' << BUILD_TYPE_DEF_LIST(N) << '>'; \
		} \
	private:
		static const char* sName; \
		//
		//static void type_name(const char* name) \
		//{ \
		//	static const char* sName = name; \
		//	ss << name << '<' << BUILD_TYPE_DEF_LIST(N) << '>'; \
		//	return ss.str(); \
		//} \
	}

BUILD_TYPE_DEF(1);
BUILD_TYPE_DEF(2);
BUILD_TYPE_DEF(3);
BUILD_TYPE_DEF(4);
BUILD_TYPE_DEF(5);
BUILD_TYPE_DEF(6);
BUILD_TYPE_DEF(7);
BUILD_TYPE_DEF(8);
BUILD_TYPE_DEF(9);
BUILD_TYPE_DEF(10);
BUILD_TYPE_DEF(11);
BUILD_TYPE_DEF(12);

#undef BUILD_TYPE_DEF_PREFIX 
#undef BUILD_TYPE_DEF_POSTFIX
#undef BUILD_TYPE_DEF_TYPENAME
#undef BUILD_TYPE_DEF_SEP
#undef BUILD_TYPE_DEF_LIST
#undef BUILD_PARAMETER_LIST
#undef BUILD_TYPE_DEF

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
