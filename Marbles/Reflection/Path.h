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

#include <reflection/type.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
namespace reflection
{
// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*) = &type_info::hash, typename _Ax = allocator<_Hash> >
class PathT : public vector<_Hash, _Ax>
{
public:
	PathT(const char* path = "");
	PathT(const string& path);
	PathT(const PathT& path);

	_Hash hashName() const;

	void push_back(const _Hash& value);
	void push_back(const char* value);
	void push_back(const string& value);
	void push_back(const PathT& path);

	PathT& operator=(const _Hash& value);
	PathT& operator=(const char* path);
	PathT& operator=(const string& path);
	PathT& operator=(const PathT& path);

	static const char Seperator;
private:
	typedef vector<_Hash, _Ax> base_type;
	static base_type Parse(const char* path);
};

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
const char PathT<_Hash, _HashFn, _Ax>::Seperator = '.';

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
PathT<_Hash, _HashFn, _Ax>::PathT(const char* path)
: base_type(Parse(path))
{
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
PathT<_Hash, _HashFn, _Ax>::PathT(const string& path)
: base_type(Parse(path.c_str()))
{
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
PathT<_Hash, _HashFn, _Ax>::PathT(const PathT& path)
: base_type(path)
{
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
_Hash PathT<_Hash, _HashFn, _Ax>::hashName() const
{
	_Hash hashName = 0;
	for (auto i = base_type::begin(); i != base_type::end(); ++i)
	{
		hashName += *i;
	}
	return hashName;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
void PathT<_Hash, _HashFn, _Ax>::push_back(const _Hash& value)
{
	base_type::push_back(value);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
void PathT<_Hash, _HashFn, _Ax>::push_back(const char* value)
{
	base_type::push_back((*_HashFn)(value));
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
void PathT<_Hash, _HashFn, _Ax>::push_back(const string& value)
{
	base_type::push_back((*_HashFn)(value.c_str()));
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
void PathT<_Hash, _HashFn, _Ax>::push_back(const PathT<_Hash, _HashFn, _Ax>& path)
{
	reserve(base_type::size() + path.size());
	for (auto i = path.begin(); i != path.end(); ++i)
	{
		base_type::push_back(*i);
	}
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
PathT<_Hash, _HashFn, _Ax>& PathT<_Hash, _HashFn, _Ax>::operator=(const _Hash& value)
{
	base_type::clear();
	base_type::push_back(value);
	return *this;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
PathT<_Hash, _HashFn, _Ax>& PathT<_Hash, _HashFn, _Ax>::operator=(const char* path)
{
	base_type::operator=(Parse(path));
	return *this;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
PathT<_Hash, _HashFn, _Ax>& PathT<_Hash, _HashFn, _Ax>::operator=(const string& path)
{
	base_type::operator=(Parse(path.c_str()));
	return *this;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
PathT<_Hash, _HashFn, _Ax>& PathT<_Hash, _HashFn, _Ax>::operator=(const PathT<_Hash, _HashFn, _Ax>& path)
{
	base_type::operator=(path);
	return *this;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename _Hash, _Hash (*_HashFn)(const char*), typename _Ax>
typename PathT<_Hash, _HashFn, _Ax>::base_type PathT<_Hash, _HashFn, _Ax>::Parse(const char* path)
{
	base_type out;
	char* end;
	char* begin = const_cast<char*>(path);

	do 
	{
		for(end = begin; '\0' != *end && Seperator != *end; ++end);
		if (Seperator == *end)
		{
			*end = '\0';						// modify end to hash the value
			out.push_back((*_HashFn)(begin));	// hash and store the value
			*end = Seperator;					// reset end back to original value
			begin = end + 1;					// A new beginning
			continue;
		}
		else
		{	// leaf name
			out.push_back(type_info::hash(begin));
			break;
		}
	} while ('\0' != *end);
	return out;
}

// --------------------------------------------------------------------------------------------------------------------
//inline path HashPath(const object& root, const object& child)
//{
//	typedef type_info::member_list::const_iterator iterator;
//	typedef vector<pair<iterator, iterator> > Stack;
//	
//	Stack stack;
//	stack.push_back(make_pair(root.members().begin(), root.members().end()));
//	while (!stack.empty() && stack.back().first->identical(child))
//	{
//		type_info::member_list& backMembers = stack.back().first->members();
//		if (backMembers.empty())
//		{	// What about the indexable elements?
//			stack.pop_back();
//		}
//		else
//		{
//			stack.push_back(make_pair(backMembers.begin(), backMembers.end()));
//		}
//		++stack.back().first;
//		if (stack.back().first == stack.back().second)
//		{	// What about the indexable elements?
//			stack.pop_back();
//		}
//	}
//
//	path path;
//	path.reserve(stack.size());
//	for (Stack::iterator i = stack.begin(); i != stack.end(); ++i)
//	{
//		path.push_back(i->first->hashName());
//	}
//	return path;
//
//	// from SerializationWrite.inl(142) -- ExtendPathTo
//	//serializer::PathT::size_type depth = path.size();
//	//for (	type_info::member_list::const_iterator i = obj.members().begin();
//	//		i != members.end() && !path.back().identical(obj);
//	//		++i)
//	//{
//	//	path.push_back(path.back().at(*i));
//	//	if (!ExtendPathTo(path, obj))
//	//	{
//	//		path.pop_back();
//	//	}
//	//}
//	//return depth != path.size();
//}

typedef PathT<hash_t> path;

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace marbles
// End of file --------------------------------------------------------------------------------------------------------
