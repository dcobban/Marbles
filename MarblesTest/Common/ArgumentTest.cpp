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
#include <string>

struct TestResults
{
	bool flagTest;
	int intTest;
	float floatTest;
	std::string stringTest;
	char stringArrayTest[32];
	// std::unique_ptr<char> stringDynamicTest;
};

REFLECT_TYPE(TestResults,
	REFLECT_MEMBER("flagTest", &TestResults::flagTest)
	REFLECT_MEMBER("intTest", &TestResults::intTest)
	REFLECT_MEMBER("floatTest", &TestResults::floatTest)
	REFLECT_MEMBER("stringTest", &TestResults::stringTest)
	//REFLECT_MEMBER("stringArrayTest", &TestResults::stringArrayTest)
	// REFLECT_MEMBER("stringDynamicTest", &TestResults::stringDynamicTest)
	)

BOOST_AUTO_TEST_SUITE(argument_test)

BOOST_AUTO_TEST_CASE(parse_basic_arguments)
{
	BOOST_MESSAGE("argument.parse_basic_arguments");

	const marbles::reflection::shared_type info = marbles::reflection::type_of<TestResults>();
	TestResults results;
	marbles::argument args;

	args.add("-d", "flagTest");
	args.add("-i", "intTest");
	args.add("-f", "floatTest");
	args.add("-s", "stringTest");
	args.add("-a", "stringArrayTest");

	char *argv[] = { 
		"MyApp.exe",
		"-d",
		"-i", "3.5",
		"-f", "3.5",
		"-s", "\"Happy hunting.\"",
		//"-a", "\"Continue hunting.\"",
	};
	const int argc = sizeof(argv) / sizeof(argv[0]);
	const int count = args.parse(results, argc, argv);

	BOOST_CHECK_EQUAL(count, info->members().size());
	BOOST_CHECK(results.flagTest);
	BOOST_CHECK_EQUAL(3, results.intTest);
	BOOST_CHECK_EQUAL(3.5f, results.floatTest);
	BOOST_CHECK_EQUAL("\"Happy hunting.\"", results.stringTest);
	//BOOST_CHECK(0 == strcmp("\"Continue hunting.\"", &results.stringArrayTest[0]));
}

BOOST_AUTO_TEST_SUITE_END()
