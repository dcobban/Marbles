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

#pragma once

#include <sstream>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
class argument
{
public:
	static const int32_t max_argument_length = 32;

	bool add(const char_t* option, const char_t* longName)
	{
		const bool canAdd = max_argument_length > strnlen(longName, max_argument_length);
		if (canAdd)
		{
			_arguments.push_back({ option, longName, NULL });
		}
		return canAdd;
	}

	// returns number of arguments parsed.
	template<typename T> int32_t parse(T& out, int32_t argc, char_t** argv)
	{
		reflection::shared_type type = reflection::type_of<T>(out);
		for (auto& option : _arguments)
		{
			hash_t hashName = reflection::type_info::hash(option._name);
			size_t memberIndex = type->memberIndex(hashName);
			if (0 <= memberIndex && memberIndex < type->members().size())
			{
				option._member = type->members()[memberIndex];
			}
			else
			{
				option._member = NULL;
			}
		}

		int32_t count = 0;
		for (int pos = 1; pos < argc;)
		{
			translateArgument(out, pos, argc, argv);
			++count;
		}
		return count;
	}

private:
	struct option
	{
		const char_t* _option;
		const char_t* _name;
		reflection::shared_member _member;
	};

	const option* findOption(int position, int , char** argv) const
	{
		auto arg = std::find_if(std::begin(_arguments), std::end(_arguments), [=](const option& arg)
		{
			const bool isMapped = NULL != arg._member;
			const bool isShort = 0 == _strnicmp(&argv[position][0], arg._option, max_argument_length);
			const bool isLong = 0 == _strnicmp(&argv[position][0], arg._name, max_argument_length);
			const bool canTranslate = isMapped && (isShort || isLong);

			return canTranslate;
		});
		return arg != _arguments.end() ? &(*arg) : NULL;
	}
	template<typename T> struct value
	{
		static bool translate(reflection::object& obj, argument& self, int& position, int argc, char** argv)
		{
			const bool isType = reflection::type_of<T>() == obj.typeInfo();
			const option* opt = self.findOption(position, argc, argv);
			const bool readValue = NULL == opt && isType;
			if (readValue)
			{
				std::stringstream ss(argv[position]);
				T& out = obj.as<T>();
				ss >> out;
				++position;
			}
			return isType;
		}
	};
	template<> struct value<bool>
	{
		static bool translate(reflection::object& obj, argument& self, int& position, int argc, char** argv)
		{
			const bool isBoolean = reflection::type_of<bool>() == obj.typeInfo();
			const option* opt = self.findOption(position, argc, argv);
			const bool readValue = NULL == opt;
			if (isBoolean)
			{
				if (readValue)
				{
					const bool isFalse = argv[position][0] == 'f' || argv[position][0] == 'F';
					obj.as<bool>() = !isFalse;
					++position;
				}
				else
				{
					obj.as<bool>() = true;
				}
			}
			return isBoolean;
		}
	};
	template<> struct value<std::string>
	{
		static bool translate(reflection::object& obj, argument& self, int& position, int argc, char** argv)
		{
			const bool isType = reflection::type_of<std::string>() == obj.typeInfo();
			const option* opt = self.findOption(position, argc, argv);
			const bool readValue = NULL == opt && isType;
			if (readValue)
			{
				std::stringstream ss(argv[position]);
				bool quote = false;
				do {
					if (quote)
					{
						ss << ' ' << argv[position];
					}
					for (int i = 0; argv[position][i]; ++i)
					{
						if ('\"' == argv[position][i])
							quote = !quote;
					}
					++position;
				} while (quote);
				std::string& out = obj.as<std::string>();
				out = ss.str();
			}
			return isType;
		}
	};
	template<size_t N> struct value<char[N]>
	{
		static bool translate(reflection::object& obj, argument& self, int& position, int argc, char** argv)
		{
			const bool isType = reflection::type_of<char>() == obj.typeInfo();
			const option* opt = self.findOption(position, argc, argv);
			const bool readValue = NULL == opt && isType;
			if (readValue)
			{
				std::stringstream ss(argv[position]);
				bool quote = false;
				do {
					if (quote)
					{
						ss << ' ' << argv[position];
					}
					for (int i = 0; argv[position][i]; ++i)
					{
						if ('\"' == argv[position][i])
							quote = !quote;
					}
					++position;
				} while (quote);
				std::string& out = obj.as<std::string>();
				out = ss.str();
			}
			return isType;
		}
	};
	template<typename T> bool translateArgument(T& out, int& position, int argc, char** argv)
	{
		bool consumed = false;
		const option* opt = findOption(position, argc, argv);
		++position;
		if (opt)
		{
			reflection::object obj = reflection::object(out).at(opt->_member);
			if (obj.isValid())
			{
				consumed = value<bool>::translate(obj, *this, position, argc, argv)
					|| value<int8_t>::translate(obj, *this, position, argc, argv)
					|| value<int16_t>::translate(obj, *this, position, argc, argv)
					|| value<int32_t>::translate(obj, *this, position, argc, argv)
					|| value<int64_t>::translate(obj, *this, position, argc, argv)
					|| value<uint8_t>::translate(obj, *this, position, argc, argv)
					|| value<uint16_t>::translate(obj, *this, position, argc, argv)
					|| value<uint32_t>::translate(obj, *this, position, argc, argv)
					|| value<uint64_t>::translate(obj, *this, position, argc, argv)
					|| value<float32_t>::translate(obj, *this, position, argc, argv)
					|| value<float64_t>::translate(obj, *this, position, argc, argv)
					|| value<std::string>::translate(obj, *this, position, argc, argv);
			}
		}
		return consumed;
	}

	std::stringstream _sstream;
	std::vector<option> _arguments;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of File --------------------------------------------------------------------------------------------------------
