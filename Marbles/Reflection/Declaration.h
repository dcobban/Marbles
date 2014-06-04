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

#include <type_traits>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
namespace reflection
{

// --------------------------------------------------------------------------------------------------------------------
// review(danc): This description brakes down the equivalent of a typedef, should we rename it?
class declaration
{
public:
	shared_type		typeInfo() const;
	shared_member	memberInfo() const	{ return member; }
	inline bool		isValid() const		{ return !!member; }
	inline bool		isConstant() const	{ return is_constant; }
	inline bool		isValue() const		{ return Value == semantic; }
	inline bool		isCallable() const	{ return Function == semantic; }
	inline bool		isReference() const	{ return !isValue() & !isCallable(); }
	inline bool		isShared() const	{ return Shared == semantic; }
	inline bool		isWeak() const		{ return Weak == semantic; }

	declaration()
	: semantic(Value)
	, is_constant(false)
	, deref_count(0)
	//, reserved2(0)
	{
	}

	declaration(shared_member member, bool constant = false)
	: member(member)
	, semantic(Value)
	, is_constant(constant)
	, deref_count(0)
	//, reserved2(0)
	{
	}

	declaration(const declaration& declaration, bool constant)
	: member(declaration.member)
	, semantic(declaration.semantic)
	, is_constant(constant)
	, deref_count(0)
	//, reserved2(0)
	{
	}

	declaration(shared_member member, const declaration& declaration)
	: member(member)
	, semantic(declaration.semantic)
	, is_constant(declaration.is_constant)
	, deref_count(0)
	//, reserved2(0)
	{
	}

protected:
	enum reference_semantic
	{
		Value = 0,	
		Function,		// Callable function
		Reference,		// Reference to raw value
		Shared,			// Reference to shared value
		Weak,			// Reference to weak value
	};
	shared_member		member;
	uint8_t				semantic	: 3;
	bool				is_constant	: 1;
	uint8_t				deref_count	: 4;	// max of 2^4 - 1 or 15
	//uint8_t				deref_count;		// How many times must we dereference before finding the actual value
	//uint32_t			reserved2	: 16;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
class declarationT : public declaration
{
	template<typename T> struct get_semantic
	{ 
		static const reference_semantic value = 
			std::is_function<T>::value || std::is_member_function_pointer<T>::value ? Function : Value; 
	};
	template<typename T> struct get_semantic< T* > { static const reference_semantic value = Reference; };
	template<typename T> struct get_semantic< std::shared_ptr<T> > { static const reference_semantic value = Shared; };
	template<typename T> struct get_semantic< std::weak_ptr<T> > { static const reference_semantic value = Weak; };

	template<typename T> struct get_deref_count			{ static const int value = 0; };
	template<typename T> struct get_deref_count< T* >	{ static const int value = get_deref_count<T>::value + 1; };

public:
	declarationT()
	: declaration(type_of<T>()->valueDeclaration())
	{
		is_constant	= std::is_const<std::remove_reference<T>::type>::value;
		semantic = get_semantic<std::remove_const<T>::type>::value;
		deref_count = get_deref_count<std::remove_reference<T>::type>::value;
	}

	inline static void* store(T*& obj)
	{
		return &obj;
	}
	inline static void* store(T& obj)
	{
		typedef typename std::remove_const<T>::type* storeType;
		return const_cast<storeType>(&obj);
	}
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
