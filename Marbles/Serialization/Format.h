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
namespace Serialization
{

// --------------------------------------------------------------------------------------------------------------------
class Format
{
	bool mEndianSwap;
	Reflection::Object mRoot;
public:
	Format(Object root, bool endianSwap) : mEndianSwap(endianSwap), mRoot(root) {}

	bool					EndianSwap() const { return mEndianSwap; }

	// Put put interface
	virtual std::ostream&	Write(std::ostream& os, const bool& value) const = 0;
	virtual std::ostream&	Write(std::ostream& os, const Marbles::uint8_t& value) const = 0;
	virtual std::ostream&	Write(std::ostream& os, const Marbles::uint16_t& value) const = 0;
	virtual std::ostream&	Write(std::ostream& os, const Marbles::uint32_t& value) const = 0;
	virtual std::ostream&	Write(std::ostream& os, const Marbles::uint64_t& value) const = 0;
	virtual std::ostream&	Write(std::ostream& os, const Marbles::int8_t& value) const = 0;
	virtual std::ostream&	Write(std::ostream& os, const Marbles::int16_t& value) const = 0;
	virtual std::ostream&	Write(std::ostream& os, const Marbles::int32_t& value) const = 0;
	virtual std::ostream&	Write(std::ostream& os, const Marbles::int64_t& value) const = 0;
	virtual std::ostream&	Write(std::ostream& os, const std::string& value) const = 0;
	virtual std::ostream&	Write(std::ostream& os, const double& value) const = 0;
	virtual std::ostream&	Write(std::ostream& os, const float& value) const = 0;

	virtual std::ostream&	TypeInfo(std::ostream& os, const Object& root) const = 0;
	virtual std::ostream&	Label(std::ostream& os, const Object& root) const = 0;
	virtual std::ostream&	OpenEnumeration(std::ostream& os) const = 0;
	virtual std::ostream&	CloseEnumeration(std::ostream& os) const = 0;
	virtual std::ostream&	OpenMap(std::ostream& os) const = 0;
	virtual std::ostream&	CloseMap(std::ostream& os) const = 0;

	virtual std::ostream&	Seperator(std::ostream& os) const = 0;
	virtual std::ostream&	Indent(std::ostream& os) const = 0;
	virtual std::ostream&	NewLine(std::ostream& os) const = 0;

	// Read Interface
	virtual bool			Read(std::istream& is, Object& value) const = 0;

	virtual bool			TypeInfo(std::istream& is, Object& value) const = 0;
	virtual std::string		Label(std::istream& is) const = 0;
	virtual bool			OpenEnumeration(std::istream& is) const = 0;
	virtual bool			CloseEnumeration(std::istream& is) const = 0;
	virtual bool			OpenMap(std::istream& is) const = 0;
	virtual bool			CloseMap(std::istream& is) const = 0;

	virtual bool			Consume(std::istream& is) const = 0;
	virtual bool			Seperator(std::istream& is) const = 0;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace Serialization
} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
