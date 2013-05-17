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

#include <Reflection.h>
#include <Common/Common.h>

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{
namespace Reflection
{
// --------------------------------------------------------------------------------------------------------------------
shared_type	Declaration::TypeInfo() const 
{ 
	return member->TypeInfo(); 
}

// --------------------------------------------------------------------------------------------------------------------
Member::Member(const std::string& name, const Declaration& declaration, const char* usage)
: mName(name)
, mType(declaration.TypeInfo())
, mUsage(usage)
, mDeclaration(declaration)
{
	mHashName = Type::Hash(mName.c_str());
}

// --------------------------------------------------------------------------------------------------------------------
Member::Member(const std::string& name, const shared_type& type, const char* usage)
: mName(name)
, mType(type)
, mUsage(usage)
{
	mHashName = Type::Hash(mName.c_str());
}

// --------------------------------------------------------------------------------------------------------------------
Object Member::Assign(Object self, const Object& rhs) const
{
	ASSERT(self.IsValid());
	ASSERT(rhs.TypeInfo()->Implements(self.TypeInfo()));
	const Type::MemberList& members = self.TypeInfo()->Members();
	for(Type::MemberList::const_iterator i = members.begin(); i < members.end(); ++i)
	{
		self.At(*i) = rhs.At(*i);
	}
	return self;
}

// --------------------------------------------------------------------------------------------------------------------
Object Member::Dereference(const Object& /*self*/) const
{
	return Object();
}

// --------------------------------------------------------------------------------------------------------------------
Object Member::Append(Object& /*self*/) const
{
	return Object();
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace Reflection
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
