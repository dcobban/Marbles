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

// The writer should collect and then write.  for example
// Writer<TextFormat> writer ( root ); // if no sub call is made all will be serialized
// writer.Include( root.a ); // only sub elements serialized
// writer.Exclude( root.b.c.g );
// ostd::stringstream oss;
// writer.write( oss );
// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
class Writer
{
public:
					Writer(const object& obj, bool endianSwap = false);

	void			Include(const object& obj);
	void			Exclude(const object& obj);
	bool			Write(std::ostream& os);

private:
	const ObjectList& Context() const;
	void			PathWritten();

	bool			Write(std::ostream& os, const object& obj);
	template <typename T> bool Translate(std::ostream& os, const object& obj);
	bool			WriteReference(std::ostream& os, const object& obj);
	bool			WriteEnumerable(std::ostream& os, const object& obj);
	bool			WriteMembers(std::ostream& os, const object& obj);
	void			WriteNewLine(std::ostream& os) const;
	bool			ExtendPathTo(ObjectList& route, const object& obj);

	typedef std::map<hash_t, std::string> PathMap;
	F				mFormat;
	ObjectList		mPath;
	PathMap			mWritten;
	ObjectList		mIncludes;
	ObjectMap		mExcludes; // Can this be done with mWritten?
};

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
Writer<F>::Writer(const object& obj, bool endianSwap = false)
: mFormat(obj, endianSwap)
{
	mPath.push_back(obj);
	mWritten[NULL] = "0";
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
void Writer<F>::Include(const object& obj)
{
	mIncludes.push_back(obj);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
void Writer<F>::Exclude(const object& obj)
{	// Review(danc): Can this be implemented using mWritten? ie do we need the mExcludes variable?
	mExcludes.push_back(obj); 
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
const ObjectList& Writer<F>::Context() const
{
	return mPath;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
void Writer<F>::PathWritten()
{
	hash_t hashName = mPath.back().hashName();
	PathMap::const_iterator written = mWritten.find(hashName);
	if (written == mWritten.end())
	{
		std::stringstream ss;
		for (ObjectList::const_iterator i = mPath.begin() + 1; i != mPath.end(); ++i)
		{
			ss << "." << i->memberInfo()->name();
		}
		if (0 == ss.tellg())
		{
			ss << ".";
		}
		mWritten[hashName] = ss.str();
		if (mPath.back().isReference())
		{
			object deref = *mPath.back();
			hashName = deref.hashName();
			written = mWritten.find(hashName);
			if (written == mWritten.end())
			{
				mWritten[hashName] = ss.str();
			}
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
bool Writer<F>::WriteReference(std::ostream& os, const object& obj)
{
	std::ios::pos_type pos = os.tellp();
	bool write = obj.isValid() && obj.isReference();
	if (write)
	{
		object value = *obj;
		PathMap::const_iterator i = mWritten.find(value.hashName());
		const bool hasPath = i != mWritten.end();

		PathWritten();
		if (0 == value.address())
		{
			mFormat.Zero(os);
		}
		else if (hasPath)
		{
			mFormat.PathInfo(os, i->second);
		}
		else if (value.isValid())
		{
			Write(os, value);
		}
	}
	return pos != os.tellp();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
bool Writer<F>::WriteEnumerable(std::ostream& os, const object& obj)
{
	bool write = obj.IsEnumerable();
	if (write)
	{
		PathWritten();

		mFormat.OpenEnumeration(os);
		std::ios::pos_type pos = os.tellp();
		mPath.push_back(obj);

		mPath.pop_back();
		if (pos != os.tellp())
		{
			WriteNewLine(os);
		}
		mFormat.CloseEnumeration(os);
	}
	return write;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
bool Writer<F>::WriteMembers(std::ostream& os, const object& obj)
{
	bool write = obj.isValid() && 0 != obj.members().size();
	if (write)
	{
		PathWritten();

		mFormat.OpenMap(os);
		std::ios::pos_type pos = os.tellp();
		
		bool first = true;
		type_info::member_list::const_iterator end = obj.members().end();
		type_info::member_list::const_iterator begin = obj.members().begin();
		for (type_info::member_list::const_iterator i = begin; i != end; ++i)
		{
			object member(obj.at(*i));
			mPath.push_back(member);
			if (!first)
			{
				mFormat.Seperator(os);
			}
			first = false;
			WriteNewLine(os);
			mFormat.Label(os, member);
			Write(os, member);
			mPath.pop_back();
		}

		if (pos != os.tellp())
		{
			WriteNewLine(os);
		}
		mFormat.CloseMap(os);
	}
	return write;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
void Writer<F>::WriteNewLine(std::ostream& os) const
{
	mFormat.NewLine(os);
	for (type_info::member_list::size_type i = 1; i < mPath.size(); ++i)
	{
		mFormat.Indent(os);
	}
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> 
bool Writer<F>::ExtendPathTo(ObjectList& path, const object& obj)
{
	const object parent = path.back();
	const type_info::member_list& members = parent.members();
	const ObjectList::size_type depth = path.size();
	for (	type_info::member_list::const_iterator i = members.begin();
			!parent.Identical(obj) && i != members.end();
			++i)
	{
		object member = parent.at(*i);
		if (member.isValid())
		{
			ObjectList::const_reverse_iterator j = path.rbegin(); 
			while(j != path.rend() && !j->Identical(member))
			{
				++j;
			}
			if (j == path.rend())
			{
				path.push_back(member);
			//		PathWritten();
				if (!member.Identical(obj) && !ExtendPathTo(path, obj))
				{
					path.pop_back();
				}
				else
				{
					break;
				}
			}
		}
	}
	return depth != path.size();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F> template <typename T> 
bool Writer<F>::Translate(std::ostream& os, const object& obj)
{
	bool canWrite = type_of<T>() == obj.typeInfo();
	if (canWrite)
	{
		PathWritten();
		mFormat.Write(os, obj.as<T>());
	}
	return canWrite;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F>
bool Writer<F>::Write(std::ostream& os)
{
	std::ios::pos_type pos = os.tellp();
	for(ObjectList::iterator i = mIncludes.begin(); i != mIncludes.end(); ++i)
	{
		PathMap::iterator written = mWritten.find(i->hashName());
		if (mWritten.end() == written)
		{
			Write(os, *i);
		}
	}
	return pos != os.tellp();
}

// --------------------------------------------------------------------------------------------------------------------
template<typename F>
bool Writer<F>::Write(std::ostream& os, const object& value)
{
	ASSERT(0 < mPath.size() && mPath.back().isValid());
	std::ios::pos_type pos = os.tellp();
	if (1 == mPath.size() && !mPath.back().isReference())
	{	// The first element should always output type_info info.
		mFormat.typeInfo(os, mPath.back());
	}

	ObjectList::size_type top = mPath.size() - 1;
	if (ExtendPathTo(mPath, value))
	{	// The path has been modified, write out path navigation to value
		if (mPath[top].IsEnumerable())
		{
			mFormat.OpenEnumeration(os);
		}
		else if (0 != mPath[top].typeInfo()->members().size())
		{
			mFormat.OpenMap(os);
		}
		for (ObjectList::size_type i = top + 1; i < mPath.size(); ++i)
		{
			// mFormat.WriteNewLine(os);
			mFormat.NewLine(os);
			for (ObjectList::size_type depth = 1; depth <= i; ++depth)
			{
				mFormat.Indent(os);
			}

			mFormat.Label(os, mPath[i]);
			if (mPath[i].IsEnumerable())
			{
				mFormat.OpenEnumeration(os);
			}
			else if (0 != mPath[i].typeInfo()->members().size())
			{
				mFormat.OpenMap(os);
			}
		}
		Write(os, value);
		while(top < mPath.size() - 1)
		{
			mPath.pop_back();
			WriteNewLine(os);

			if (mPath.back().IsEnumerable())
			{
				mFormat.CloseEnumeration(os);
			}
			else if (0 != mPath.back().typeInfo()->members().size())
			{
				mFormat.CloseMap(os);
			}
		}
	}
	else if (Translate<bool>(os, value))				{}
	else if (Translate<marbles::uint8_t>(os, value))	{}
	else if (Translate<marbles::uint16_t>(os, value))	{}
	else if (Translate<marbles::uint32_t>(os, value))	{}
	else if (Translate<marbles::uint64_t>(os, value))	{}
	else if (Translate<marbles::int8_t>(os, value))		{}
	else if (Translate<marbles::int16_t>(os, value))	{}
	else if (Translate<marbles::int32_t>(os, value))	{}
	else if (Translate<marbles::int64_t>(os, value))	{}
	else if (Translate<float>(os, value))				{}
	else if (Translate<double>(os, value))				{}
	else if (Translate<std::string>(os, value))			{}
	else if (WriteReference(os, value))					{}
	else if (WriteEnumerable(os, value))				{}
	else if (WriteMembers(os, value))					{}
	
	return pos != os.tellp();
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace serialization
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
