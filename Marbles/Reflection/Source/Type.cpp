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
#include <Common/Common.h>
#include <Common/hash.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
namespace reflection
{
namespace 
{
// --------------------------------------------------------------------------------------------------------------------
} // namespace <>

// --------------------------------------------------------------------------------------------------------------------
std::map<hash_t, shared_type> type_info::sRegistrar;

// --------------------------------------------------------------------------------------------------------------------
type_info::type_info()
: mByValue()
, mCreateFn(NULL)
{
}

// --------------------------------------------------------------------------------------------------------------------
type_info::~type_info() 
{
}

// --------------------------------------------------------------------------------------------------------------------
const std::string& type_info::name() const
{ 
	return mByValue.memberInfo()->name(); 
}

// --------------------------------------------------------------------------------------------------------------------
hash_t type_info::hashname() const
{ 
	return mByValue.memberInfo()->hashname(); 
}

// --------------------------------------------------------------------------------------------------------------------
const bool type_info::implements(const shared_type& type_info) const
{
	bool implements = type_info && *this == *type_info;
	for(type_list::const_iterator base = mImplements.begin(); 
		!implements && base != mImplements.end(); 
		++base)
	{
		implements = (*base)->implements(type_info);
	}
	return implements;
}

// --------------------------------------------------------------------------------------------------------------------
type_info::member_list::size_type type_info::memberIndex(hash_t hashName) const
{	// A binary search would be good!
	member_list::size_type i = 0; 
	while (i < mMembers.size() && hashName != mMembers[i]->hashname())
	{
		++i;
	}
	return i;
}

// --------------------------------------------------------------------------------------------------------------------
object type_info::create(const char* name) const
{
	(void)name;
	ASSERT(NULL == name); // usage of name not implemented
	object obj;
	if (mCreateFn)
	{
		(*mCreateFn)(obj);
	}
	return obj;
}

// --------------------------------------------------------------------------------------------------------------------
const bool type_info::operator==(const type_info& type_info) const
{
	return hashname() == type_info.hashname();
}

// --------------------------------------------------------------------------------------------------------------------
hash_t type_info::hash(const char* str)
{
	return hash::djb2(str);
}

// --------------------------------------------------------------------------------------------------------------------
hash_t type_info::hash(const void* obj, size_t size)
{
	return hash::djb2(obj, size);
}

// --------------------------------------------------------------------------------------------------------------------
shared_type	type_info::find(const char* name)
{
	return find(hash(name));
}

// --------------------------------------------------------------------------------------------------------------------
shared_type	type_info::find(hash_t hashName)
{
	std::map<hash_t, shared_type>::iterator i = sRegistrar.find(hashName);
	return i != sRegistrar.end() ? i->second : shared_type();
}

// --------------------------------------------------------------------------------------------------------------------
void type_info::clear_registrar()
{
	// sRegistrar.clear();
	while (!sRegistrar.empty())
	{
		TypeMap::iterator i = sRegistrar.begin();
		sRegistrar.erase(i);
	}
}

// --------------------------------------------------------------------------------------------------------------------
bool type_info::_register(shared_type type_info)
{
	const bool registered = sRegistrar.end() != sRegistrar.find(type_info->hashname());
	if (!registered)
	{
		sRegistrar[type_info->hashname()] = type_info;
	}
	return !registered;
}

// --------------------------------------------------------------------------------------------------------------------
type_info::builder::builder()
{
}

// --------------------------------------------------------------------------------------------------------------------
shared_type type_info::builder::create(const char* name)
{
	std::shared_ptr<type_info> candidate = std::shared_ptr<type_info>(new type_info());
	shared_type type = std::const_pointer_cast<const type_info>(candidate);

	// Creates a circular reference type_info->member->type_info
	std::shared_ptr<member> mem = std::make_shared<member>(name, type, "Default by value type_info member");
	candidate->mByValue = std::const_pointer_cast<const member>(mem); 

	if (type_info::_register(type))
	{
		mBuild.swap(candidate);
	}
	else
	{
		type.reset();
	}

	return type;
}

// --------------------------------------------------------------------------------------------------------------------
void type_info::builder::setCreator(type_info::CreateFn fn)
{
	ASSERT(mBuild);
	mBuild->mCreateFn = fn;
}

// --------------------------------------------------------------------------------------------------------------------
void type_info::builder::setSize(size_t size)
{
	ASSERT(mBuild);
	mBuild->mSize = size;
}

// --------------------------------------------------------------------------------------------------------------------
void type_info::builder::setAlignment(size_t alignment)
{
	ASSERT(mBuild);
	mBuild->mAlignment = 0;
	// Their is some bit twiddling way to do this
	while (1 < alignment)
	{	
		alignment >>= 1;
		++mBuild->mAlignment;
	}
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
