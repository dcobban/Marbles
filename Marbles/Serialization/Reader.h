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
namespace serialization
{
// --------------------------------------------------------------------------------------------------------------------
template<typename F> class Reader
{
public:
					Reader(bool endianSwap = false);

	const path&		Context() const;
	bool			Read(std::istream& is, object& obj);

private:
	template <typename T> bool Translate(std::istream& is, object& obj);
	bool			ReadReference(std::istream& is, object& obj);
	bool			ReadEnumerable(std::istream& is, object& obj);
	bool			ReadMembers(std::istream& is, object& obj);
	static hash_t	hash(const path& route);

	F				mFormat;
	ObjectList		mPath;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
Reader<F>::Reader(bool endianSwap = false)
: mFormat(object(), endianSwap)
{
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F>
bool Reader<F>::Read(std::istream& is, object& value)
{
	mFormat.typeInfo(is, value);
	
	bool canRead = value.isValid();
	if (canRead)
	{
		mPath.push_back(value);
		if (ReadReference(is, value))
		{
			ReadMembers(is, value);
		}
		else if (mFormat.OpenEnumeration(is))
		{
			do {
				object element = value.append();
				Read(is, element);
			} while(!mFormat.CloseEnumeration(is));
		}
		else if (ReadMembers(is, value))
		{	
		}
		else 
		{
			mFormat.Read(is, value);
		}
		mPath.pop_back();
	}
	return canRead;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> template <typename T> 
bool Reader<F>::Translate(std::istream& is, object& obj)
{
	bool canWrite = type_of<T>() == obj.typeInfo();
	if (canWrite)
	{
		mFormat.Read(os, obj.as<T>());
	}
	return canWrite;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F>
bool Reader<F>::ReadReference(std::istream& is, object& value)
{
	return mFormat.ReadReference(is, mPath, value);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F>
bool Reader<F>::ReadMembers(std::istream& is, object& value)
{
	std::istream::pos_type start = is.tellg();
	if (mFormat.OpenMap(is))
	{
		do {
			const std::string& label = mFormat.Label(is);
			object member = value.at(label.c_str());
			if (member.isValid()) 
			{	// Read this element
				Read(is, member);
			}
			else
			{	// Ignore the current object
				mFormat.Consume(is);
			}
		} while (!mFormat.CloseMap(is));
	}
	return start != is.tellg();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F>
hash_t Reader<F>::hash(const path& route)
{
	hash_t hash = 0;
	for(path::const_iterator i = path.begin(); i != path.end(); ++i)
	{
		hash += i->memberInfo()->hashName();
	}
	return hash;
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace serialization
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
