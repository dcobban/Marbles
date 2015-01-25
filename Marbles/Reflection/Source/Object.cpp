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

#include <reflection.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
namespace reflection
{

// --------------------------------------------------------------------------------------------------------------------
object::~object()
{
}

// --------------------------------------------------------------------------------------------------------------------
object::object() 
{}

// --------------------------------------------------------------------------------------------------------------------
object::object(const object& obj)
: mInfo(obj.mInfo)
, mPointee(obj.mPointee)
{}

// --------------------------------------------------------------------------------------------------------------------
object::object(object& obj)
: mInfo(obj.mInfo)
, mPointee(obj.mPointee) 
{}

// --------------------------------------------------------------------------------------------------------------------
object::object(const declaration& declaration, void* pointee)
: mInfo(declaration)
{
	_SetAddress(pointee);
}

// --------------------------------------------------------------------------------------------------------------------
object::object(const declaration& declaration, const std::shared_ptr<void>& pointee)
: mInfo(declaration)
, mPointee(pointee) 
{
}

// --------------------------------------------------------------------------------------------------------------------
object&	object::operator=(const object& rhs)
{
	// if rhs is not valid then lhs becomes not valid
	// if lhs is not valid then rhs is cloned
	// if rhs implements lhs it is valid to do a formal assignment
	// if rhs is zero then reset lhs approprately

	shared_type typeInfo = object::typeInfo();
	if (!isValid() || !rhs.isValid())
	{	// Clone the object
		object clone(rhs);
		swap(clone);
	}
	else
	{
		object rhsValue(rhs);
		if (rhsValue.isReference())
		{	// rhs must always be a value type_info
			object deref(*rhsValue);
			rhsValue.swap(deref);
		}

		const bool shared_assignment = mInfo.isShared() | mInfo.isWeak();
		if (shared_assignment)
		{	// Test against the type_info the managed reference looks at.
			typeInfo = (*(*this)).typeInfo();
		}

		if (rhsValue.typeInfo()->implements(typeInfo))
		{
			if (mInfo.isShared())
			{	
				std::shared_ptr<void>* self = To<std::shared_ptr<void>*>::from(*this);
				if (0 != rhsValue.mPointee.use_count())
				{
					*self = rhsValue.mPointee;
				}
				else 
				{
					self->reset(rhsValue.mPointee.get());
				}
			}
			else if (mInfo.isWeak())
			{	
				std::weak_ptr<void>* self = To<std::weak_ptr<void>*>::from(*this);
				if (0 != rhsValue.mPointee.use_count())
				{
					*self = rhsValue.mPointee;
				}
				else 
				{
					ASSERT(!"Illogical conversion, the raw pointer will be deleted immediately.");
				}
			}
			else if (mInfo.isReference())
			{	// assign the values reference address
				void**& self = To<void**>::from(*this);
				*self = rhsValue.address();
				assert(*To<void**>::from(*this) == rhsValue.address());
			}
			else
			{
				memberInfo()->assign(*this, rhsValue);
			}
		}
		else if (rhs._IsZero())
		{
			_AssignZero(rhs);
		}
		else
		{
			ASSERT(!"Unable to do assignment.");
		}
	}
	return *this;
}

// --------------------------------------------------------------------------------------------------------------------
hash_t object::hashName() const
{
	// Different members can have the same address, therefore combine with type_info to differenciate
	// References and values have the same type_info, therefore there are hash collisions

	hash_t hash[] = {	reinterpret_cast<hash_t>(address()), 
						reinterpret_cast<hash_t>(typeInfo().get()),
						static_cast<hash_t>(0 != mPointee.use_count())
					};
	return type_info::hash(&hash[0], sizeof(hash));
}

// --------------------------------------------------------------------------------------------------------------------
shared_member object::memberInfo(const path& route) const
{
	shared_member memberInfo;
	shared_type typeInfo = object::typeInfo();
	for (path::const_iterator i = route.begin(); typeInfo && i != route.end(); ++i)
	{
		type_info::member_list::size_type location = typeInfo->memberIndex(*i);
		if ((0 <= location) & (location < typeInfo->members().size()))
		{
			memberInfo = typeInfo->members()[location];
			typeInfo = memberInfo->typeInfo();
		}
		else
		{
			memberInfo.reset();
			typeInfo.reset();
		}
	}
	return memberInfo;
}

// --------------------------------------------------------------------------------------------------------------------
object object::at(const char* name) const
{
	return at(type_info::hash(name));
}

// --------------------------------------------------------------------------------------------------------------------
object object::at(const hash_t hashName) const
{
	object obj;
	if (isValid())
	{
		const type_info::member_list& members = object::members();
		const type_info::member_list::size_type index = typeInfo()->memberIndex(hashName);
		if (index < members.size())
		{
			object member(at(members[index]));
			obj.swap(member);
		}
		else if (isReference())
		{
			object deref(*(*this));
			if (deref.isValid())
			{
				object member(deref.at(hashName));
				obj.swap(member);
			}
		}
	}
	return obj;
}

// --------------------------------------------------------------------------------------------------------------------
object object::at(const shared_member& member) const
{
	ASSERT(member);
	return member->dereference(*this);
}

// --------------------------------------------------------------------------------------------------------------------
object object::append()
{
	return memberInfo()->append(*this);
}

// --------------------------------------------------------------------------------------------------------------------
object object::append(const object& obj)
{
	object appended = append();
	appended = obj;
	return appended;
}

// --------------------------------------------------------------------------------------------------------------------
bool object::equal(const object& obj) const
{
	// TODO: member by member comparison is required to determine if two objects are equivalant
	return identical(obj);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T>
object& object::_AssignReference(const object& rhs)
{
	T& self_ptr = *To<T*>::from(*this);
	if (rhs.mInfo.isShared())
	{
		self_ptr = *To<std::shared_ptr<void>*>::from(rhs);
	}
	else if (rhs.mInfo.isWeak())
	{
		self_ptr = To<std::weak_ptr<void>*>::from(rhs)->lock();
	}
	else if (rhs.mInfo.isReference())
	{   // not good wrong deleter!
		self_ptr = std::shared_ptr<void>(*reinterpret_cast<void**>(rhs.address()));
	}
	return *this;
}

// --------------------------------------------------------------------------------------------------------------------
inline object& object::_AssignZero(const object& zero)
{
	if (mInfo.isShared())
	{
		To<std::shared_ptr<void>*>::from(*this)->reset();
	}
	else if (mInfo.isWeak())
	{
		To<std::weak_ptr<void>*>::from(*this)->reset();
	}
	else if (mInfo.isReference())
	{
		*To<void**>::from(*this) = 0;
	}
	else
	{	// This case should never be executed, it will be treated as a normal assignment by operator=()
		ASSERT(zero.typeInfo()->implements(typeInfo()));
		memberInfo()->assign(*this, zero);
	}
	return *this;
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
