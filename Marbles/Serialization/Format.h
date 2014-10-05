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

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
namespace serialization
{

// --------------------------------------------------------------------------------------------------------------------
class format
{
	bool mEndianSwap;
	reflection::object mRoot;
public:
	format(object root, bool endianSwap) : mEndianSwap(endianSwap), mRoot(root) {}

	bool					EndianSwap() const { return mEndianSwap; }

	// Put put interface
	virtual ostream&	Write(ostream& os, const bool& value) const = 0;
	virtual ostream&	Write(ostream& os, const marbles::uint8_t& value) const = 0;
	virtual ostream&	Write(ostream& os, const marbles::uint16_t& value) const = 0;
	virtual ostream&	Write(ostream& os, const marbles::uint32_t& value) const = 0;
	virtual ostream&	Write(ostream& os, const marbles::uint64_t& value) const = 0;
	virtual ostream&	Write(ostream& os, const marbles::int8_t& value) const = 0;
	virtual ostream&	Write(ostream& os, const marbles::int16_t& value) const = 0;
	virtual ostream&	Write(ostream& os, const marbles::int32_t& value) const = 0;
	virtual ostream&	Write(ostream& os, const marbles::int64_t& value) const = 0;
	virtual ostream&	Write(ostream& os, const string& value) const = 0;
	virtual ostream&	Write(ostream& os, const double& value) const = 0;
	virtual ostream&	Write(ostream& os, const float& value) const = 0;

	virtual ostream&	typeInfo(ostream& os, const object& root) const = 0;
	virtual ostream&	Label(ostream& os, const object& root) const = 0;
	virtual ostream&	OpenEnumeration(ostream& os) const = 0;
	virtual ostream&	CloseEnumeration(ostream& os) const = 0;
	virtual ostream&	OpenMap(ostream& os) const = 0;
	virtual ostream&	CloseMap(ostream& os) const = 0;

	virtual ostream&	Seperator(ostream& os) const = 0;
	virtual ostream&	Indent(ostream& os) const = 0;
	virtual ostream&	NewLine(ostream& os) const = 0;

	// Read Interface
	virtual bool			Read(istream& is, object& value) const = 0;

	virtual bool			typeInfo(istream& is, object& value) const = 0;
	virtual string		Label(istream& is) const = 0;
	virtual bool			OpenEnumeration(istream& is) const = 0;
	virtual bool			CloseEnumeration(istream& is) const = 0;
	virtual bool			OpenMap(istream& is) const = 0;
	virtual bool			CloseMap(istream& is) const = 0;

	virtual bool			Consume(istream& is) const = 0;
	virtual bool			Seperator(istream& is) const = 0;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace serialization
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
