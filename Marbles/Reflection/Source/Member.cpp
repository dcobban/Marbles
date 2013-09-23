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

#include <reflection.h>
#include <Common/Common.h>

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{
namespace reflection
{
// --------------------------------------------------------------------------------------------------------------------
shared_type	declaration::typeInfo() const 
{ 
	return member->typeInfo(); 
}

// --------------------------------------------------------------------------------------------------------------------
member::member(const std::string& name, const declaration& declaration, const char* usage)
: mName(name)
, mType(declaration.typeInfo())
, mUsage(usage)
, mDeclaration(declaration)
{
	mHashName = type_info::hash(mName.c_str());
}

// --------------------------------------------------------------------------------------------------------------------
member::member(const std::string& name, const shared_type& type_info, const char* usage)
: mName(name)
, mType(type_info)
, mUsage(usage)
{
	mHashName = type_info::hash(mName.c_str());
}

// --------------------------------------------------------------------------------------------------------------------
object member::Assign(object self, const object& rhs) const
{
	ASSERT(self.isValid());
	ASSERT(rhs.typeInfo()->implements(self.typeInfo()));
	const type_info::member_list& members = self.typeInfo()->members();
	for(type_info::member_list::const_iterator i = members.begin(); i < members.end(); ++i)
	{
		self.at(*i) = rhs.at(*i);
	}
	return self;
}

// --------------------------------------------------------------------------------------------------------------------
object member::dereference(const object& /*self*/) const
{
	return object();
}

// --------------------------------------------------------------------------------------------------------------------
object member::Append(object& /*self*/) const
{
	return object();
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
