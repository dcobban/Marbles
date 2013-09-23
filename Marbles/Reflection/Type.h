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

#include <Common/Function_traits.h> 
#include <Common/hash.h> 

#include <sstream>
#include <map>

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{
namespace reflection
{
class object;

// --------------------------------------------------------------------------------------------------------------------
class type_info
{
public:
	typedef std::vector<shared_type>	type_list;
	typedef std::vector<shared_member>	member_list;
	typedef std::vector<declaration>	declaration_list;
	class builder;

	type_info();
	~type_info();

	const std::string&		name() const;
	hash_t					hashname() const;
	size_t					size() const						{ return mSize; }
	size_t					alignment() const					{ return static_cast<size_t>(1) << mAlignment; }
	const declaration&		valueDeclaration() const			{ return mByValue; }
	const declaration_list&	parameters() const					{ return mParameters; }
	const member_list&		members() const						{ return mMembers; }
	member_list::size_type	memberIndex(const char* name) const	{ return memberIndex(hash(name)); }
	member_list::size_type	memberIndex(hash_t hashname) const;

	const bool				implements(const shared_type& type) const;
	object					create(const char* name = NULL) const;

	const bool				operator==(const type_info& type) const;

	static hash_t			hash(const char* str);
	static hash_t			hash(const void* obj, size_t size);
	static shared_type		find(const char* name);
	static shared_type		find(hash_t hashname);
	static void				clear();

private:
	static bool				_register(shared_type type);

	declaration				mByValue;
	member_list				mMembers;
	declaration_list		mParameters;
	type_list				mImplements;
	size_t					mSize;
	unsigned char			mAlignment; // Stored as an exponent to a power of two

	typedef void (*CreateFn)(object& );

	// AssignFn
	// DestoryFn
	CreateFn mCreateFn;

	// ConvertFn
	// IndexFn
	// EnumeratorFn
	// AppendFn

	static std::map<hash_t, shared_type> sRegistrar;
};

// --------------------------------------------------------------------------------------------------------------------
class type_info::builder
{
public:
	builder();
	shared_type typeInfo() const { return mBuild; }
	shared_type create(const char* name);
	
	void setCreator(type_info::CreateFn fn);
	// void setAppend(type_info::AppendFn fn);
	// void setEnumerator();

	template<typename T> shared_type create(const char* name);
	template<typename T> void addMember(const char* name, const char* description = NULL);
	template<typename T> void addMember(const char* name, T member, const char* description = NULL);
	void addMember(const char* name, shared_type type_info, const char* description = NULL);

private:
	void setAlignment(size_t alignment);
	void setSize(size_t size);

	template<typename T> struct template_traits;
	template<typename T> friend struct type_of_t;

	std::shared_ptr<type_info> mBuild;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> struct type_info::builder::template_traits
{
	static unsigned NumberOfParameters()				{ return 0; }
	static declaration ParameterAt(unsigned /*index*/)	{ return declaration(); }
	static std::string Typename(const char* name)		{ return std::string(name); }
};

// Review(danc): This only works for type_info parameters not numeric template parameters
#define BUILD_TEMPLATE_TRAITS_PREFIX ::Marbles::reflection::type_of<A
#define BUILD_TEMPLATE_TRAITS_POSTFIX >()
#define BUILD_TEMPLATE_TRAITS_TYPENAME ->name()
#define BUILD_TEMPLATE_TRAITS_SEP << ',' <<
#define BUILD_TEMPLATE_TRAITS_LIST(N) FN_LIST(N,BUILD_TEMPLATE_TRAITS_PREFIX,BUILD_TEMPLATE_TRAITS_POSTFIX BUILD_TEMPLATE_TRAITS_TYPENAME,BUILD_TEMPLATE_TRAITS_SEP)
#define BUILD_TEMPLATE_PARAMETERS_PREFIX ::Marbles::reflection::declarationT<A
#define BUILD_TEMPLATE_PARAMETERS_POSTFIX >()
#define BUILD_PARAMETER_TYPE_LIST(N) FN_LIST(N,BUILD_TEMPLATE_PARAMETERS_PREFIX,BUILD_TEMPLATE_PARAMETERS_POSTFIX,FN_COMMA)
#define BUILD_TEMPLATE_TRAITS(N) \
	template<template<FN_LIST(N,typename B,,FN_COMMA)> class T, FN_TYPENAME(N)> \
	struct type_info::builder::template_traits< T<FN_TYPES(N)> > \
	{ \
		static unsigned NumberOfParameters() { return N; } \
		static declaration ParameterAt(unsigned index) \
		{ \
			declaration parameters[] = { BUILD_PARAMETER_TYPE_LIST(N) }; \
			return (0 <= index && index < N) ? parameters[index] : declaration(); \
		} \
		static std::string Typename(const char* name) \
		{ \
			std::stringstream ss; \
			int max_chars = 256; \
			while ('\0' != *name && '<' != *name && max_chars--) \
			{ \
				ss << *(name++); \
			} \
			ss << '<' << BUILD_TEMPLATE_TRAITS_LIST(N) << '>'; \
			return ss.str(); \
		} \
	}

BUILD_TEMPLATE_TRAITS(1);
BUILD_TEMPLATE_TRAITS(2);
BUILD_TEMPLATE_TRAITS(3);
BUILD_TEMPLATE_TRAITS(4);
BUILD_TEMPLATE_TRAITS(5);
BUILD_TEMPLATE_TRAITS(6);
BUILD_TEMPLATE_TRAITS(7);
BUILD_TEMPLATE_TRAITS(8);
BUILD_TEMPLATE_TRAITS(9);
BUILD_TEMPLATE_TRAITS(10);
BUILD_TEMPLATE_TRAITS(11);
BUILD_TEMPLATE_TRAITS(12);

#undef BUILD_TEMPLATE_TRAITS_PREFIX 
#undef BUILD_TEMPLATE_TRAITS_POSTFIX
#undef BUILD_TEMPLATE_TRAITS_TYPENAME
#undef BUILD_TEMPLATE_TRAITS_SEP
#undef BUILD_TEMPLATE_TRAITS_LIST
#undef BUILD_PARAMETER_TYPE_LIST
#undef BUILD_TEMPLATE_TRAITS

// --------------------------------------------------------------------------------------------------------------------
template<typename T> 
shared_type type_info::builder::create(const char* name)
{	
	std::shared_ptr<type_info> candidate = std::shared_ptr<type_info>(new type_info());
	shared_type type = std::const_pointer_cast<const type_info>(candidate);

	unsigned numberOfParameters = template_traits<typename by_value<T>::type>::NumberOfParameters(); 
	candidate->mParameters.reserve(numberOfParameters);
	for(unsigned i = 0; i < numberOfParameters; ++i)
	{
		candidate->mParameters.push_back(template_traits<typename by_value<T>::type>::ParameterAt(i)); 
	}
	std::string fullname = template_traits<typename by_value<T>::type>::Typename(name);
	std::shared_ptr< memberT<T> > mem = std::make_shared< memberT<T> >(fullname, type, "Default value type_info member.");
	candidate->mByValue = declaration(std::static_pointer_cast<member>(mem)); 

	if (type_info::_register(type))
	{
		mBuild.swap(candidate);
	}
	else
	{
		type.reset();
	}

	if (mBuild)
	{
		setCreator(&object::create<void>);
		setAlignment(std::alignment_of<T>::value);
		setSize(sizeof(T));
	}

	return type;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T> void type_info::builder::addMember(const char* name, const char* description)
{
	addMember(name, type_of<T>(), description);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T> void type_info::builder::addMember(const char* name, T member, const char* description)
{
	shared_member memberInfo = std::make_shared< memberT<T> >(name, member, description);
	if (memberInfo)
	{
		mBuild->mMembers.push_back(memberInfo);
	}
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
