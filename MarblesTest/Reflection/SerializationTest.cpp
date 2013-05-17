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

#include "FooBar.h"
#include <Serialization/Serializer.h>

BOOST_AUTO_TEST_SUITE( serialization )

BOOST_AUTO_TEST_CASE( text_serialization )
{
	BOOST_MESSAGE( "Serialization.format.text" );
	Foo foo;
	foo.x = 3;
	foo.y = 2;
	foo.z = 1;

	// Serialize same data should yield same result
	std::stringstream ss1;
	std::stringstream ss2;
	BOOST_CHECK(Marbles::Serialization::Serializer::Text(ss1, foo));
	BOOST_CHECK(Marbles::Serialization::Serializer::Text(ss2, foo));
	BOOST_CHECK_EQUAL(ss1.str(), ss2.str());
	
	// Serialize different should yield different result
	foo.x = 1;
	std::stringstream ss3;
	BOOST_CHECK(Marbles::Serialization::Serializer::Text(ss3, foo));
	BOOST_CHECK_NE(ss1.str(), ss3.str());

	// Reading outputed data should yield same result
	Foo* foo_ref = NULL;
	Foo foo2;
	foo2.bar.reference2_foo = &foo_ref;
	foo2.bar.reference_foo = &foo;
	foo2.bar.reference_zero = NULL;
	foo2.bar.shared_foo = std::make_shared<Foo>();
	foo2.bar.weak_foo = foo2.bar.shared_foo;
	BOOST_CHECK(Marbles::Serialization::Serializer::From(ss1, foo2));
	BOOST_CHECK(Marbles::Serialization::Serializer::From(ss2, foo_ref));
	BOOST_CHECK_NE(static_cast<Foo*>(NULL), foo_ref);
	if (NULL != foo_ref)
	{
		BOOST_CHECK_EQUAL(foo_ref->x, foo2.x);
		BOOST_CHECK_EQUAL(foo_ref->y, foo2.y);
		BOOST_CHECK_EQUAL(foo_ref->z, foo2.z);
		delete foo_ref;
	}
}

BOOST_AUTO_TEST_SUITE_END()
