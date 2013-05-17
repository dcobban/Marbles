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

#include <iosfwd>
#include <reflection.h>

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{
namespace Serialization
{
using namespace Reflection;

// --------------------------------------------------------------------------------------------------------------------
class Serializer
{
public:
	static bool Text(std::ostream& os, const Object& root);
	static bool Text(std::ostream& os, const Object& root, const Object& sub);
	//static bool Binary(std::ostream& os, const Object& root, const bool big_endian = PLATFORM_BIG_ENDIAN);
	//static bool Binary(std::ostream& os, const Object& root, const Object& sub, const bool big_endian = PLATFORM_BIG_ENDIAN);
	static bool From(std::istream& is, Object& root);

	template<typename T>				static bool Text(std::ostream& os, const T& root);
	template<typename T, typename S>	static bool Text(std::ostream& os, const T& root, const S& sub);
	//template<typename T>				static bool Binary(std::ostream& os, const T& root, const bool big_endian = PLATFORM_BIG_ENDIAN);
	//template<typename T, typename S>	static bool Binary(std::ostream& os, const T& root, const S& sub, const bool big_endian = PLATFORM_BIG_ENDIAN);
	template<typename T>				static bool From(std::istream& is, T& root);
};

// --------------------------------------------------------------------------------------------------------------------
inline bool Serializer::Text(std::ostream& os, const Object& root)
{
	return Text(os, root, root);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T> inline bool Serializer::Text(std::ostream& os, const T& root)
{
	Object rootobj(root);
	return Text(os, rootobj, rootobj);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T, typename S> inline bool Serializer::Text(std::ostream& os, const T& root, const S& sub)
{
	Object rootobj(root);
	Object subobj(sub);
	return Text(os, rootobj, subobj);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T> inline bool Serializer::From(std::istream& is, T& root)
{
	Object rootobj(root);
	return From(is, rootobj);
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace Serialization
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
