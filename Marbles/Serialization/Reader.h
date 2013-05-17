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
#include <Common/Hash.h>

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{
namespace Serialization
{
// --------------------------------------------------------------------------------------------------------------------
template<typename F> class Reader
{
public:
					Reader(bool endianSwap = false);

	const Path&		Context() const;
	bool			Read(std::istream& is, Object& obj);

private:
	template <typename T> bool Translate(std::istream& is, Object& obj);
	bool			ReadReference(std::istream& is, Object& obj);
	bool			ReadEnumerable(std::istream& is, Object& obj);
	bool			ReadMembers(std::istream& is, Object& obj);
	static hash_t	Hash(const Path& route);

	F				mFormat;
	Path			mPath;
	std::map<hash_t, std::string> mReferenceMap;
};

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
Reader<F>::Reader(bool endianSwap = false)
: mFormat(Object(), endianSwap)
{
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F>
bool Reader<F>::Read(std::istream& is, Object& value)
{
	mFormat.TypeInfo(is, value);
	
	bool canRead = value.IsValid();
	if (canRead)
	{
		if (ReadReference(is, value))
		{
		}
		else if (mFormat.OpenEnumeration(is))
		{
			do {
				Object element = value.Append();
				Read(is, element);
			} while(!mFormat.CloseEnumeration(is));
		}
		else if (mFormat.OpenMap(is))
		{	// Read map
			do {
				const std::string& label = mFormat.Label(is);
				Object member = value.At(label.c_str());
				if (member.IsValid()) 
				{	// Read this element
					Read(is, member);
				}
				else
				{	// Ignore the current object
					mFormat.Consume(is);
				}
			} while (!mFormat.CloseMap(is));
		}
		else 
		{
			mFormat.Read(is, value);
		}
	}
	return canRead;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> template <typename T> 
bool Reader<F>::Translate(std::istream& is, Object& obj)
{
	bool canWrite = TypeOf<T>() == obj.TypeInfo();
	if (canWrite)
	{
		mFormat.Read(os, obj.As<T>());
	}
	return canWrite;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F>
bool Reader<F>::ReadReference(std::istream& is, Object& value)
{
	return mFormat.ReadReference(is, value);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F>
hash_t Reader<F>::Hash(const Path& route)
{
	hash_t hash = 0;
	for(Path::const_iterator i = path.begin(); i != path.end(); ++i)
	{
		hash += i->MemberInfo()->HashName();
	}
	return hash;
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace Serialization
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
