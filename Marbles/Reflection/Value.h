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
namespace Marbles
{
namespace Reflection
{

// --------------------------------------------------------------------------------------------------------------------
template<typename T> 
class MemberT : public Member
{
public:
	MemberT(const std::string& name, const shared_type& type, const char* usage)
	: Member(name, type, usage) {}
	MemberT(const std::string& name, const Declaration& declaration, const char* usage)
	: Member(name, declaration, usage) {}

	virtual shared_type	DeclaredType() const { return TypeOf<T>(); }
	virtual Object		Assign(Object self, const Object& rhs) const;
	virtual Object		Dereference(const Object& /*self*/) const;
	virtual Object		Append(Object& self) const;
private:
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> 
Object MemberT<T>::Assign(Object self, const Object& rhs) const
{
	ASSERT(self.IsValid());
	ASSERT(self.TypeInfo()->Implements(rhs.TypeInfo()));
	if (self.IsValue())
	{
		self.As<T>() = rhs.As<T>();
	}
	//else if (self.IsShared())
	//{
	//	self.As< std::shared_ptr<T> >() = rhs.As<T>();
	//}
	//else if (self.IsWeak())
	//{
	//	self.As< std::weak_ptr<T> >() = rhs.As<T>();
	//}
	else if (self.IsReference())
	{
		self.As<T*>() = rhs.As<T*>();
	}
	return self;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T> 
Object MemberT<T>::Dereference(const Object& /*self*/) const
{ 
	ASSERT(!"Not implemented");
	return Object(); 
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T> 
Object MemberT<T>::Append(Object& /*self*/) const
{
	ASSERT(!"Not implemented");
	return Object();
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace Reflection
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
