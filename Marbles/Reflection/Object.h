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

#pragma once

// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{
namespace reflection
{
class object;
typedef std::vector<object> ObjectList;
typedef std::map<hash_t, object> ObjectMap;

#pragma warning(push)
#pragma warning(disable: 4521) // C4521 : multiple copy constructors specified

// --------------------------------------------------------------------------------------------------------------------
class object
{
public:
	object();
	~object();
	object(const object& obj);
	object(object& obj); // Second copy constructor such that the template<> constructor does not apply

							explicit object(const declaration& declaration, 
											void* pointee);
							explicit object(const declaration& declaration, 
											const std::shared_ptr<void>& pointee);
	template<typename T>	explicit object(T& obj);

	object&					operator=(const object& obj);
	//object&					operator=(const object& obj);
	template<typename T>	object&	operator=(const T& obj);

	bool					isValid() const;
	bool					IsEnumerable() const{ return false; }
	bool					isCallable() const	{ return mInfo.isCallable(); }
	bool					isConstant() const	{ return mInfo.isConstant(); }
	bool					isValue() const		{ return mInfo.isValue(); }
	bool					isReference() const	{ return mInfo.isReference(); }
	bool					isShared() const	{ return mInfo.isShared(); }
	bool					isWeak() const		{ return mInfo.isWeak(); }

	object&					swap(object& obj);
	void					Reset()				{ mPointee.reset(); }
	void*					Address() const		{ return mPointee.get(); }
	hash_t					HashName() const;

	shared_type				_type_info() const	{ return mInfo.typeInfo(); }
	shared_type				typeInfo() const	{ return _type_info(); }
	shared_member			memberInfo() const	{ return mInfo.memberInfo(); }
	shared_member			memberInfo(const char* name) const;
	shared_member			memberInfo(const std::string& name) const;
	shared_member			memberInfo(const path& route) const;
	shared_member			memberInfo(hash_t hashName) const;
	const type_info::member_list& members() const		{ return typeInfo()->members(); }

	template<typename T>	T& as();
	template<typename T>	T& as() const;

	object					at(const char* name) const;
	object					at(const std::string& name) const;
	object					at(const path& route) const;
	object					at(const hash_t hashName) const;
	object					at(const shared_member& member) const;
	object					Append();
	object					Append(const object& obj);

	bool					Equal(const object& obj) const;
	bool					Identical(const object& obj) const;

	bool					operator==(const object& obj) const { return Equal(obj); }
	bool					operator!=(const object& obj) const { return !operator==(obj); }

	object					operator*() const;
	//object operator()() const;
	//object operator()(object );

	template<typename T>	static void create(object& obj);
	template<typename T>	static void CreateShared(object& obj);
private:
	template<typename T>	struct To;
	template<typename T>	struct Put;

	bool	_IsZero() const;
	template<typename T>
	object& _AssignReference(const object& rhs);
	object& _AssignZero(const object& zero);
	void	_SetAddress(void* p);

	friend class declaration;
	declaration				mInfo;		// Description of how to interpret mPointee
	std::shared_ptr<void>	mPointee;	// Generic reference to the object
};

#pragma warning(pop)

// --------------------------------------------------------------------------------------------------------------------
template<typename T> object::object(T& obj)
: mInfo(declarationT<T>())
{
	_SetAddress(declarationT<T>::store(obj)); 
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T> object& object::operator=(const T& value)
{
	object rhs(value);
	return operator=(rhs);
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T> struct Put
{
	static object& Value(object& obj, const T& value) 
	{ 
		return obj;
	}
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> struct object::To
{
	static T& from(const object& obj) 
	{ 
		return *To<T*>::from(obj); 
	}
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> struct object::To<T*>
{
	static T*& from(const object& obj)
	{	// TODO: check readonly flag here
		typedef std::remove_cv<T>::type NoConstT;
		const void* address = &obj.mPointee;
		return const_cast<T*>(*reinterpret_cast<NoConstT* const *>(address));
	}
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> inline T& object::as()
{
	if (isReference())
	{
		object deref(*(*this));
		return To<T>::from(deref); 
	}
	return To<T>::from(*this); 
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T> inline	T& object::as() const
{ 
	return const_cast<object*>(this)->as<T>();
}

// --------------------------------------------------------------------------------------------------------------------
inline bool object::isValid() const
{
	return mPointee && mInfo.isValid();
}

// --------------------------------------------------------------------------------------------------------------------
inline bool object::Identical(const object& obj) const
{
	return	obj.typeInfo() == typeInfo() &&
			obj.Address() == Address();
}

// --------------------------------------------------------------------------------------------------------------------
inline object& object::swap(object& obj)
{
	std::swap(obj.mInfo, mInfo);
	std::swap(obj.mPointee, mPointee);
	return *this;
}

// --------------------------------------------------------------------------------------------------------------------
inline shared_member object::memberInfo(const std::string& name) const
{
	return memberInfo(name.c_str());
}

// --------------------------------------------------------------------------------------------------------------------
inline shared_member object::memberInfo(const char* name) const
{
	path path(name);
	return memberInfo(path);
}

// --------------------------------------------------------------------------------------------------------------------
inline shared_member object::memberInfo(hash_t hashName) const 
{
	const type_info::member_list& members = typeInfo()->members();
	const type_info::member_list::size_type index = typeInfo()->memberIndex(hashName); 
	return members[index];
}

// --------------------------------------------------------------------------------------------------------------------
inline object object::operator*() const 
{
	object result;
	if (isShared())
	{
		std::shared_ptr<void>* pointee = reinterpret_cast<std::shared_ptr<void>*>(mPointee.get());
		object deref(	declaration(typeInfo()->parameters()[0], isConstant()),
						*pointee);
		result.swap(deref);
	}
	else if (isWeak())
	{
		std::weak_ptr<void>* pointee = reinterpret_cast<std::weak_ptr<void>*>(mPointee.get());
		object deref(	declaration(typeInfo()->parameters()[0], isConstant()),
						pointee->lock());
		result.swap(deref);
	}
	else if (isReference())
	{
		object deref(	declaration(typeInfo()->valueDeclaration(), isConstant()),
						*reinterpret_cast<void**>(mPointee.get()));
		result.swap(deref);
	}
	return result; 
}

// --------------------------------------------------------------------------------------------------------------------
template<> inline void object::create<void>(object& obj) { object invalid; obj.swap(invalid); }
template<typename T> inline void object::create(object& obj)
{
	obj.mInfo = declarationT<T>();
	obj._SetAddress(new T());
}

// --------------------------------------------------------------------------------------------------------------------
template<> inline void object::CreateShared<void>(object& obj) { object invalid; obj.swap(invalid); }
template<typename T> inline void object::CreateShared(object& obj)
{
	std::shared_ptr<T> pT = std::make_shared<T>();
	obj.Clone(*object(pT));
}

// --------------------------------------------------------------------------------------------------------------------
inline bool object::_IsZero() const
{
	return	isReference() 
		? 0 == To<unsigned*>::from(*this)
		: 0 == To<unsigned>::from(*this);
}

// --------------------------------------------------------------------------------------------------------------------
inline void object::_SetAddress(void* p) 
{	// std::shared_ptr<> does not allow us to set a raw pointer without allocating managed data.
	// Since std::shared_ptr<> uses only the allocated managed data to perform deletion operations 
	// setting the raw pointer does not put the std::shared_ptr<> into an invalid state.
	mPointee.reset();
	*reinterpret_cast<void**>(&mPointee) = p;
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace Marbles

template<typename T> inline bool operator==(const Marbles::reflection::object& obj, const T& var)
{ return obj.typeInfo()->implements(Marbles::reflection::type_of<T>()) && obj.as<T>() == var; }
template<typename T> inline bool operator==(const T& var, const Marbles::reflection::object& obj)
{ return obj.typeInfo()->implements(Marbles::reflection::type_of<T>()) && var == obj.as<T>(); }
template<typename T> inline bool operator!=(const Marbles::reflection::object& obj, const T& var)
{ return !(obj == var); }
template<typename T> inline bool operator!=(const T& var, const Marbles::reflection::object& obj)
{ return !(var == obj); }

// End of file --------------------------------------------------------------------------------------------------------
