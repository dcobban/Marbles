// This source file is part of marbles library.
//
// Copyright (c) 2014 Dan Cobban
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

#include <Common/Argument.h>

struct TestResults
{
	bool flagTest = false;
	int intTest = 0;
	float floatTest = 0.0;
    marbles::string stringTest = "";
	char stringArrayTest[32] = "";
	// unique_ptr<char> stringDynamicTest;
};

REFLECT_TYPE(TestResults,
	REFLECT_MEMBER("flagTest", &TestResults::flagTest)
	REFLECT_MEMBER("intTest", &TestResults::intTest)
	REFLECT_MEMBER("floatTest", &TestResults::floatTest)
	REFLECT_MEMBER("stringTest", &TestResults::stringTest)
	//REFLECT_MEMBER("stringArrayTest", &TestResults::stringArrayTest)
	// REFLECT_MEMBER("stringDynamicTest", &TestResults::stringDynamicTest)
	)

TEST(argument_test, parse_basic_arguments)
{
	const marbles::reflection::shared_type info = marbles::reflection::type_of<TestResults>();
	TestResults results;
	marbles::argument args;

	args.add("-d", "flagTest");
	args.add("-i", "intTest");
	args.add("-f", "floatTest");
	args.add("-s", "stringTest");
	args.add("-a", "stringArrayTest");

	const char *argv[] = { 
		"MyApp.exe",
		"-d",
		"-i", "3.5",
		"-f", "3.5",
		"-s", "\"Happy hunting.\"",
		//"-a", "\"Continue hunting.\"",
	};
	const size_t argc = sizeof(argv) / sizeof(argv[0]);
	const size_t count = args.parse(results, argc, const_cast<char**>(argv));

	EXPECT_EQ(count, info->members().size());
	EXPECT_EQ(true, results.flagTest);
	EXPECT_EQ(3, results.intTest);
	EXPECT_EQ(3.5f, results.floatTest);
	EXPECT_EQ("\"Happy hunting.\"", results.stringTest);
	//EXPECT_EQ(0, strcmp("\"Continue hunting.\"", &results.stringArrayTest[0]));
}

// End of file --------------------------------------------------------------------------------------------------------
