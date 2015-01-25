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

#include <Common/hash.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
namespace reflection
{
// --------------------------------------------------------------------------------------------------------------------
class type_info;
typedef std::shared_ptr<const type_info> shared_type;
typedef std::weak_ptr<const type_info> weak_type;

// --------------------------------------------------------------------------------------------------------------------
class member;
typedef std::shared_ptr<const member> shared_member;
typedef std::weak_ptr<const member> weak_member;

// --------------------------------------------------------------------------------------------------------------------
template<typename T> struct instance_t 
{ 
	static T& get() { static T s_instance; return s_instance; } 
private:
	static const T& s_reference; // Forces a 'get' call during static initialization
};

template<typename T> const T& instance_t<T>::s_reference = instance_t<T>::get();

// --------------------------------------------------------------------------------------------------------------------
template<typename T> struct type_of_t
{ 
	static shared_type typeInfo(); 
};
template<typename T, bool hasTypeInfo = HasTypeInfo<T>::value> struct resolveT
{ 
	inline static shared_type typeInfo()
	{
		return type_of_t<T>::typeInfo();
	}
	inline static shared_type typeInfo(T*) 
	{ 
		return type_of_t<T>::typeInfo(); 
	} 
};
template<typename T> struct resolveT<T, true> 
{
	inline static shared_type typeInfo()
	{
		return type_of_t<T>::typeInfo();
	}
	inline static shared_type typeInfo(T* p)
	{
		return NULL == p ? type_of_t<T>::typeInfo() : p->_type_info();
	}
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> shared_type type_of()	
{ 
	return resolveT<typename by_value<T>::type>::typeInfo(); 
}
template<typename T> shared_type type_of(T& obj)
{
	return resolveT<typename by_value<T>::type>::typeInfo(&obj);
}
template<typename T> shared_type type_of(T* obj)
{
	return resolveT<typename by_value<T>::type>::typeInfo(obj);
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace marbles

// --------------------------------------------------------------------------------------------------------------------
#define REFLECT_TYPE(T,REFLECT_DEFINITION) \
	REFLECT_COMMON_TYPE(template<>,T,REFLECT_DEFINITION)

#define REFLECT_COMMA ,
#define REFLECT_TEMPLATE_TYPE(_template,T,REFLECT_DEFINITION) \
	REFLECT_COMMON_TYPE(_template,T,REFLECT_DEFINITION)

// Todo: Ensure that builders are the only object that internally references a std::shared_ptr to a type_info!
#define REFLECT_COMMON_TYPE(_template,T,REFLECT_DEFINITION) \
	_template struct ::marbles::reflection::type_of_t<T> \
	: public ::marbles::reflection::instance_t< ::marbles::reflection::type_of_t<T> > \
	{	\
		static ::marbles::reflection::shared_type typeInfo() \
		{ \
			::marbles::reflection::shared_type typeInfo = get().reflect_type.lock(); \
			if (!typeInfo) \
				typeInfo = get().create(); \
			return typeInfo; \
		} \
	private: \
		::marbles::reflection::weak_type reflect_type; \
	public: \
		::marbles::reflection::shared_type create() \
		{ \
			typedef T self_type; \
			::marbles::reflection::type_info::builder build; \
			reflect_type = build.create<T>(#T); \
			REFLECT_DEFINITION \
			return build.typeInfo(); \
		} \
	}; \

#define REFLECT_CREATOR() \
			build.setCreator(&object::create<self_type>); \

#define REFLECT_MEMBER(...) \
			build.addMember(__VA_ARGS__); \

// End of file --------------------------------------------------------------------------------------------------------
