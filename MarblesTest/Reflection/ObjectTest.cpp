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

using namespace marbles::reflection;

TEST(reflection_object, reflection_object_field )
{
	Foo foo;
	float half = 0.5f;
	double tiny = std::numeric_limits<double>::min();
	int negative = -1; 
	bool question = false; 
	unsigned long long huge = std::numeric_limits<unsigned long long>::max(); 

	object foo_obj(foo);
	object half_obj(half);
	object tiny_obj(tiny);
	object negative_obj(negative);
	object question_obj(question);
	object huge_obj(huge);

	EXPECT_EQ(half_obj.as<float>(), half);
	EXPECT_EQ(tiny_obj.as<double>(), tiny);
	EXPECT_EQ(negative_obj.as<int>(), negative);
	EXPECT_EQ(question_obj.as<bool>(), question);
	EXPECT_EQ(huge_obj.as<unsigned long long>(), huge);

	half_obj = 0.25f;
	EXPECT_EQ(half, 0.25f);
	tiny_obj = 0.0000000001;
	EXPECT_EQ(tiny, 0.0000000001);
	negative_obj = -25;
	EXPECT_EQ(negative, -25);
	question_obj = 0 > negative;
	EXPECT_EQ(question, true);
	huge_obj = 0ull;
	EXPECT_EQ(huge, 0);

	foo.x = 1;
	foo.y = 2.0f;
	foo.z = 3ull;

	EXPECT_EQ(foo_obj.at("X").as<int>(), foo.x);
	EXPECT_EQ(foo_obj.at("Y").as<float>(), foo.y);
	EXPECT_EQ(foo_obj.at("Z").as<marbles::uint64_t>(), foo.z);

	object bar_obj = foo_obj.at("Bar");
	bar_obj.at("reference_foo") = &foo;
	EXPECT_EQ(foo.bar.reference_foo, &foo);
	object new_foo_obj = type_of<Foo>()->create();
	bar_obj.at("shared_foo") = new_foo_obj.as<Foo*>();
	Foo* test1 = new_foo_obj.as<Foo*>();
	Foo* test2 = bar_obj.at("shared_foo").as<Foo*>();
	EXPECT_EQ(test1, test2);
	bar_obj.at("weak_foo") = bar_obj.at("shared_foo");
	Foo* test3 = bar_obj.at("weak_foo").as<Foo*>();
	EXPECT_EQ(test1, test3);
	
	marbles::reflection::type_info::clear_registrar();
}
