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
class object;
template<typename T> class memberT;

// --------------------------------------------------------------------------------------------------------------------
class member : public std::enable_shared_from_this<const member>
{
public:
	member(const std::string& name, const declaration& declaration, const char* usage);
	member(const std::string& name, const shared_type& type_info, const char* usage);

	const std::string&	name() const		{ return mName; }
	hash_t				hashname() const	{ return mHashName; }
	declaration			declare_info() const{ return declaration(shared_from_this(), mDeclaration); }
	const char*			usage() const		{ return mUsage; }

	virtual shared_type typeInfo() const	{ return mType.lock(); }
	virtual object		assign(object self, const object& rhs) const;
	virtual object		dereference(const object& self) const;
	virtual object		append(object& self) const;

private:
	std::string			mName;
	hash_t				mHashName;
	declaration			mDeclaration;
	weak_type			mType;
	const char*			mUsage;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
