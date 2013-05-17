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

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{
namespace Reflection
{
class Object;
template<typename T> class MemberT;

// --------------------------------------------------------------------------------------------------------------------
class Member : public std::enable_shared_from_this<const Member>
{
public:
	Member(const std::string& name, const Declaration& declaration, const char* usage);
	Member(const std::string& name, const shared_type& type, const char* usage);

	const std::string&	Name() const		{ return mName; }
	hash_t				HashName() const	{ return mHashName; }
	Declaration			DeclareInfo() const	{ return Declaration(shared_from_this(), mDeclaration); }
	const char*			Usage() const		{ return mUsage; }

	virtual shared_type TypeInfo() const	{ return mType.lock(); }
	virtual Object		Assign(Object self, const Object& rhs) const;
	virtual Object		Dereference(const Object& self) const;
	virtual Object		Append(Object& self) const;

private:
	std::string			mName;
	hash_t				mHashName;
	Declaration			mDeclaration;
	weak_type			mType;
	const char*			mUsage;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace Reflection
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
