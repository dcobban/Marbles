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
namespace serialization
{
using namespace reflection;

// --------------------------------------------------------------------------------------------------------------------
class serializer
{
public:
	static bool text(std::ostream& os, const object& root);
	static bool text(std::ostream& os, const object& root, const object& sub);
	//static bool Binary(std::ostream& os, const object& root, const bool big_endian = PLATFORM_BIG_ENDIAN);
	//static bool Binary(std::ostream& os, const object& root, const object& sub, const bool big_endian = PLATFORM_BIG_ENDIAN);
	static bool from(std::istream& is, object& root);

	template<typename T>				static bool text(std::ostream& os, const T& root);
	template<typename T, typename S>	static bool text(std::ostream& os, const T& root, const S& sub);
	//template<typename T>				static bool Binary(std::ostream& os, const T& root, const bool big_endian = PLATFORM_BIG_ENDIAN);
	//template<typename T, typename S>	static bool Binary(std::ostream& os, const T& root, const S& sub, const bool big_endian = PLATFORM_BIG_ENDIAN);
	template<typename T>				static bool from(std::istream& is, T& root);
};

// --------------------------------------------------------------------------------------------------------------------
inline bool serializer::text(std::ostream& os, const object& root)
{
	return text(os, root, root);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T> inline bool serializer::text(std::ostream& os, const T& root)
{
	object rootobj(root);
	return text(os, rootobj, rootobj);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T, typename S> inline bool serializer::text(std::ostream& os, const T& root, const S& sub)
{
	object rootobj(root);
	object subobj(sub);
	return text(os, rootobj, subobj);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T> inline bool serializer::from(std::istream& is, T& root)
{
	object rootobj(root);
	return from(is, rootobj);
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace serialization
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
