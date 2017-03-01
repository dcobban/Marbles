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

#include "FooBar.h"
#include <serialization/serializer.h>

TEST(serialization, text_serialization)
{
	Foo foo;
	foo.x = 3;
	foo.y = 2;
	foo.z = 1;

	foo.bar.reference_foo = &foo;
	foo.bar.shared_foo = marbles::make_shared<Foo>();
	foo.bar.shared_foo->x = 1;
	foo.bar.shared_foo->y = 2;
	foo.bar.shared_foo->z = 3;
	foo.bar.weak_foo = foo.bar.shared_foo;

	// Serialize same data should yield same result
    marbles::stringstream ss1;
    marbles::stringstream ss2;
	EXPECT_EQ(true, marbles::serializer::text(ss1, foo));
	EXPECT_EQ(true, marbles::serializer::text(ss2, foo));
	EXPECT_EQ(ss1.str(), ss2.str());
	
	// Serialize different should yield different result
	foo.x = 1;
    marbles::stringstream ss3;
	EXPECT_EQ(true, marbles::serializer::text(ss3, foo));
	EXPECT_NE(ss1.str(), ss3.str());

	// Reading outputed data should yield same results
	{
		Foo* foo_ref = NULL;
		Foo foo2;

		// some fake data for override
		foo2.bar.reference_zero = NULL;
		foo2.bar.reference_foo = &foo;
		foo2.bar.reference2_foo = &foo_ref;
		foo2.bar.shared_foo = marbles::make_shared<Foo>();
		foo2.bar.weak_foo = foo2.bar.shared_foo;

		EXPECT_EQ(true, marbles::serializer::from(ss1, foo2));
		EXPECT_NE(foo2.bar.reference_foo, &foo);
		EXPECT_EQ(foo2.bar.reference_foo, &foo2);

		EXPECT_EQ(true, marbles::serializer::from(ss2, foo_ref));
		EXPECT_NE(static_cast<Foo*>(NULL), foo_ref);
		if (NULL != foo_ref)
		{
			EXPECT_EQ(foo_ref->x, foo2.x);
			EXPECT_EQ(foo_ref->y, foo2.y);
			EXPECT_EQ(foo_ref->z, foo2.z);
			EXPECT_EQ(foo_ref->bar.reference_foo, foo_ref);
			EXPECT_EQ(foo2.bar.reference_foo, &foo2);
			EXPECT_EQ(foo_ref->bar.shared_foo->x, foo2.bar.shared_foo->x);
			EXPECT_EQ(foo_ref->bar.shared_foo->y, foo2.bar.shared_foo->y);
			EXPECT_EQ(foo_ref->bar.shared_foo->z, foo2.bar.shared_foo->z);
			delete foo_ref;
			foo_ref = NULL;
		}
	}

	// Sub element serialization
	// 
    marbles::stringstream sub;
    marbles::shared_ptr<Foo> org = marbles::make_shared<Foo>();

	// Intialize data 
	foo.bar.reference_foo = &foo;
	foo.bar.shared_foo = org;
	foo.bar.shared_foo->x = 1;
	foo.bar.shared_foo->y = 2;
	foo.bar.shared_foo->z = 3;
	foo.bar.weak_foo = foo.bar.shared_foo;

	EXPECT_EQ(true, marbles::serializer::text(sub, foo, foo.bar.shared_foo));
	foo.bar.shared_foo.reset();
	EXPECT_EQ(true, marbles::serializer::from(sub, foo));

	EXPECT_NE(foo.bar.shared_foo, org);
	EXPECT_EQ(foo.bar.shared_foo->x, org->x);
	EXPECT_EQ(foo.bar.shared_foo->y, org->y);
	EXPECT_EQ(foo.bar.shared_foo->z, org->z);

	marbles::reflection::type_info::clear_registrar();
}
