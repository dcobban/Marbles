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

#include <serialization/serializer.h>
#include <serialization/Reader.h>
#include <serialization/Writer.h>
#include <serialization/format.h>

#include <locale>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
namespace serialization
{
namespace 
{
// --------------------------------------------------------------------------------------------------------------------
const uint32_t EndianSwap(const uint32_t value)
{
	return ((value << 24) & 0xFF000000)
		| ((value << 16) & 0x00FF0000)
		| ((value >> 16) & 0x0000FF00) 
		| ((value >> 24) & 0x000000FF);
}

// --------------------------------------------------------------------------------------------------------------------
static const uint32_t text_big		= 'type';
static const uint32_t text_little	= 'epyt';
static const uint32_t binary_big	= 'rsb0';
static const uint32_t binary_little	= '0bsr';
static const uint32_t memory_big	= 'rsm0';
static const uint32_t memory_little	= '0msr';

// --------------------------------------------------------------------------------------------------------------------
struct TextFormat : public format
{
	ostream& Write(ostream& os, const bool& value) const
	{ return os << ios::boolalpha << value; } 
	ostream& Write(ostream& os, const marbles::uint8_t& value) const
	{ return os << value; } 
	ostream& Write(ostream& os, const marbles::uint16_t& value) const
	{ return os << value; } 
	ostream& Write(ostream& os, const marbles::uint32_t& value) const
	{ return os << value; } 
	ostream& Write(ostream& os, const marbles::uint64_t& value) const
	{ return os << value; } 
	ostream& Write(ostream& os, const marbles::int8_t& value) const
	{ return os << value; } 
	ostream& Write(ostream& os, const marbles::int16_t& value) const
	{ return os << value; } 
	ostream& Write(ostream& os, const marbles::int32_t& value) const
	{ return os << value; } 
	ostream& Write(ostream& os, const marbles::int64_t& value) const
	{ return os << value; } 
	ostream& Write(ostream& os, const float& value) const
	{ return os << value; } 
	ostream& Write(ostream& os, const double& value) const
	{ return os << value; } 
	ostream& Write(ostream& os, const string& value) const
	{ return os << "\"" << value << "\"" ; } 

	ostream& typeInfo(ostream& os, const object& obj) const
	{
		ASSERT(obj.isValid());
		return os << "type_info<" << obj.typeInfo()->name() << "> ";
	}
	ostream& Label(ostream& os, const object& obj) const
	{
		ASSERT(obj.isValid());
		return os << obj.memberInfo()->name() << " = ";
	}
	ostream& PathInfo(ostream& os, const string& path) const
	{
		return os << path;
	}
	ostream& OpenEnumeration(ostream& os) const
	{ return os << '['; }
	ostream& CloseEnumeration(ostream& os) const
	{ return os << ']'; }
	ostream& OpenMap(ostream& os) const
	{ return os << '{'; }
	ostream& CloseMap(ostream& os) const
	{ return os << '}'; }
	ostream& Seperator(ostream& os) const
	{ return os << ','; }
	ostream& Indent(ostream& os) const
	{ return os << "  "; }
	ostream& NewLine(ostream& os) const
	{ return os << endl; }
	ostream& Zero(ostream& os) const
	{ return os << '0'; }

	// Reader interface
	bool Read(istream& is, object& value) const
	{
		shared_type type_info = value.typeInfo();
		string readValue;
		istream::pos_type pos = is.tellg();
		locale prev = is.getloc();

		is.imbue(mReadStop);
		char peek = static_cast<char>(is.peek());
		if ('"' == peek || '\'' == peek)
		{
			// Read open quote
			ReadIf(is, '"') || ReadIf(is, '\'');

			is.imbue(mStringStop);
			is >> value.as<string>();
			is.imbue(mReadStop);

			// Read closing quote
			ReadIf(is, '"') || ReadIf(is, '\'');
		}
		else if ('-' == peek || isdigit(peek, mReadStop))
		{	// Read a number
			if (*type_info == *type_of<marbles::uint8_t>())		is >> value.as<marbles::uint8_t>();
			else if (*type_info == *type_of<marbles::uint16_t>())	is >> value.as<marbles::uint16_t>();
			else if (*type_info == *type_of<marbles::uint32_t>())	is >> value.as<marbles::uint32_t>();
			else if (*type_info == *type_of<marbles::uint64_t>())	is >> value.as<marbles::uint64_t>();
			else if (*type_info == *type_of<marbles::int8_t>())	is >> value.as<marbles::int8_t>();
			else if (*type_info == *type_of<marbles::int16_t>())	is >> value.as<marbles::int16_t>();
			else if (*type_info == *type_of<marbles::int32_t>())	is >> value.as<marbles::int32_t>();
			else if (*type_info == *type_of<marbles::int64_t>())	is >> value.as<marbles::int64_t>();
			else if (*type_info == *type_of<float>())				is >> value.as<float>();
			else if (*type_info == *type_of<double>())			is >> value.as<double>();
			else { ASSERT(!"Unknown numeric type_info!"); }
		}
		else if ('t' == tolower(peek, prev) || 'f' == tolower(peek, prev))
		{
			string boolean;
			is >> boolean;
			value.as<bool>() = "true" == boolean;
		}
		else 
		{	
			ASSERT(!"Unable to read");
		}
		if (pos != is.tellg())
		{
			//hash_t hashName = 0;
			//for (ObjectList::iterator i = mPath.begin(); i != mPath.end(); ++i)
			//{
			//	hashName += i->HashName();
			//}
			//mElements[hashName] = value;
		}
		is.imbue(prev);
		return pos != is.tellg();
	}

	bool typeInfo(istream& is, object& value) const
	{
		char name[1024];
		shared_type type_info;
		istream::pos_type start = is.tellg();
		locale prev = is.getloc();

		// Read formatted type_info information
		is.imbue(mReadStop);
		//is >> ios::uppercase;
		is >> name;
		// TODO(danc): this should be case insensative
		if (0 == char_traits<char>::compare("type_info", name, 4)) 
		{
			is >> name;
			type_info = type_info::find(name);
		}
		else
		{
			is.seekg(start, ios::beg);
			type_info = value.typeInfo();
		}
		is.imbue(prev);

		const bool createRequired = !value.isValid();
		const bool canCreate = type_info && (!value.isValid() || type_info->implements(value.typeInfo()));

		if (createRequired && canCreate)
		{
			value = type_info->create();
		}
		return !createRequired;
	}

	bool ReadReference(istream& is, ObjectList& path, object& value)
	{
		istream::pos_type start = is.tellg();
		if (value.isReference())
		{
			ConsumeWhitespace(is);

			locale prev = is.getloc();
			is.imbue(mReadStop);

			char txtPath[1024];
			if (isdigit(is.peek(), mReadStop))
			{	// NULL value
				is >> txtPath; // consumes 0
				value.as<void*>() = 0;
			}
			else if ('.' == is.peek())
			{	// Look up reference
				is >> txtPath;

				// TODO: A new stream? Use 'is' instead.
				char name[256] = { '\0' };
				object root(path.front());

				stringstream ss(txtPath); 
				ss.imbue(mPathStop);
				do
				{
					ss >> name;
					if ('\0' != *name)
					{
						object deref(root.at(name));
						root.swap(deref);
					}
				} while(!ss.eof() && root.isValid());
				if (root.isValid())
				{
					value = root;
				}
			}
			else if (value.isReference())
			{
				const object deref(*value);
				const bool createRequired = 0 == deref.as<void*>();
				if (createRequired)
				{	
					value = deref.typeInfo()->create();
				}
			}
			is.imbue(prev);
		}
		return start != is.tellg();
	}

	string Label(istream& is) const
	{
		istream::pos_type start = is.tellg();
		locale prev = is.getloc();
		string label;
		is.imbue(mReadStop);
		is >> label;
		is.imbue(prev);

		return label;
	}

	bool OpenEnumeration(istream& is) const
	{
		return ReadIf(is, '[');
	}

	bool CloseEnumeration(istream& is) const
	{
		return ReadIf(is, ']');
	}

	bool OpenMap(istream& is) const
	{
		return ReadIf(is, '{');
	}

	bool CloseMap(istream& is) const
	{
		return ReadIf(is, '}');
	}
	
	bool Consume(istream& is) const
	{
		istream::pos_type start = is.tellg();
		locale prev = is.getloc();
		string consume;
		int count = 0; 

		is.imbue(mReadStop);
		do 
		{
			if (ReadIf(is, '[') || ReadIf(is, '{'))
			{
				++count;
			}
			else if (ReadIf(is, ']') || ReadIf(is, '}'))
			{
				--count;
			}
			else
			{
				is >> consume;
			}
		} while (0 != count || is.eof());

		is.imbue(prev);
		return true;
	}

	bool Seperator(istream& is) const
	{
		return ReadIf(is, ',');
	}

	TextFormat(object root, const bool endianSwap) 
	: format(root, endianSwap)
	, mReadStop(locale(), new read_stop())
	, mStringStop(locale(), new string_stop())
	, mPathStop(locale(), new path_stop())
	{}

private:
	bool ReadIf(istream& is, char character) const
	{
		ConsumeWhitespace(is);
		const bool isCharacter = character == static_cast<char>(is.peek());
		if (isCharacter)
		{
			char value;
			is.read(&value, sizeof(value));
		}
		return isCharacter;
	}

	void ConsumeWhitespace(istream& is) const
	{
		char peek = static_cast<char>(is.peek());
		while (isspace(peek, mReadStop))
		{
			is.read(&peek, sizeof(peek));
			peek = static_cast<char>(is.peek());
		}
	}

	struct read_stop : public ctype<char>
	{
	public:
		read_stop()
		: ctype<char>(generate(), false, sizeof(_table))
		{}

	private:
		virtual ~read_stop()
		{}

		ctype_base::mask _table[256];
		const ctype_base::mask *generate()
		{
			const ctype<char>& stdFacet = use_facet<ctype<char>>(locale());
			const int tableSize = sizeof(_table) / sizeof(_table[0]);
			char keys[tableSize];
			for (int ch = 0; ch < tableSize; ++ch)
				keys[ch] = static_cast<char>(ch);
			stdFacet.is(&keys[0], &keys[0] + tableSize, &_table[0]);

			_table['\n'] = _table['\r'] = _table['\t'] = _table[' '] = ctype<char>::space;
			_table['<'] = _table['>'] = _table[':'] = _table['='] = ctype<char>::space;
			_table[','] = ctype<char>::space;
			// _table['['] = _table[']'] = _table['{'] = _table['}'] = ctype<char>::space;
			return &_table[0];
		}
	};
	struct string_stop : public ctype<char>
	{
	public:
		string_stop()
		: ctype<char>(generate(), false, sizeof(_table))
		{}

	private:
		virtual ~string_stop()
		{}

		ctype_base::mask _table[256];
		const ctype_base::mask *generate()
		{
			const int tableSize = sizeof(_table) / sizeof(_table[0]);
			for (int ch = 0; ch < tableSize; ++ch)
				_table[ch] = ctype<char>::punct;

			_table['\''] = _table['"'] = ctype<char>::space;
			return &_table[0];
		}
	};
	struct path_stop : public ctype<char>
	{
	public:
		path_stop()
		: ctype<char>(generate(), false, sizeof(_table))
		{}

	private:
		virtual ~path_stop()
		{}

		ctype_base::mask _table[256];
		const ctype_base::mask *generate()
		{
			const int tableSize = sizeof(_table) / sizeof(_table[0]);
			for (int ch = 0; ch < tableSize; ++ch)
				_table[ch] = ctype<char>::punct;

			_table['.'] = ctype<char>::space;
			return &_table[0];
		}
	};
	locale mReadStop;
	locale mStringStop;
	locale mPathStop;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace <>

// --------------------------------------------------------------------------------------------------------------------
bool serializer::text(ostream& os, const object& root, const object& sub)
{	// start serialization
	Writer<TextFormat> writer(root);
	writer.Include(sub);
	return writer.Write(os);
}

// --------------------------------------------------------------------------------------------------------------------
bool serializer::from(istream& is, object& root)
{
	uint32_t header;
	bool littleEndian = false;
	ios::pos_type pos = is.tellg();
	is.read(reinterpret_cast<char*>(&header), sizeof(header));

	switch(header)
	{
	case 'epyt': 
		littleEndian = true;
	case 'type':
		{	// text format
			Reader<TextFormat> reader(littleEndian);
			is.seekg(0, ios::beg);
			reader.Read(is, root);
		}
		break;
	}

	return pos != is.tellg();
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace serialization
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
