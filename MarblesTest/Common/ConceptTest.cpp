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

#include <common/concept.h>

// --------------------------------------------------------------------------------------------------------------------
struct nonKeyable { int list[1024]; };
struct nonconstKeyable : public nonKeyable
{
	int operator[](const int index) { return list[index]; }
};
struct constKeyable : public nonKeyable
{
	const int operator[](const int index) const { return list[index]; }
};
struct keyable : public constKeyable
{
	int operator[](const int index) { return list[index]; }
};

TEST( serialization, keyable_test )
{
	EXPECT_EQ(marbles::keyable<int>::value, false);
	EXPECT_EQ(marbles::keyable<nonKeyable>::value, false);
	EXPECT_EQ(marbles::keyable<nonconstKeyable>::value, true);
	EXPECT_EQ(marbles::keyable<constKeyable>::value, true);
	EXPECT_EQ(marbles::keyable<keyable>::value, true);
}


// End of file --------------------------------------------------------------------------------------------------------
