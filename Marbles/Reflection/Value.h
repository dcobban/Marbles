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
namespace reflection
{

// --------------------------------------------------------------------------------------------------------------------
template<typename T> 
class memberT : public member
{
public:
	memberT(const string name, const shared_type& type_info, const char* usage)
	: member(forward<const string>(name), type_info, usage) {}
	memberT(const string name, const declaration& declaration, const char* usage)
	: member(forward<const string>(name), declaration, usage) {}

	virtual shared_type	declaredType() const { return type_of<T>(); }
	virtual object		dereference(const object& self) const
	{
		declaration info(typeInfo()->valueDeclaration(), self.isConstant());
		object value(info, *reinterpret_cast<void**>(self.address()));
		return value;
	}
	virtual object		append(object& /*self*/) const
	{
		ASSERT(!"Not implemented");
		return object();
	}
	virtual object		assign(object self, const object& rhs) const;
private:
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T, int N>
class memberT<T[N]> : public member
{
public:
	memberT(const string name, const shared_type& type_info, const char* usage)
		: member(forward<const string>(name), type_info, usage) {}
	memberT(const string name, const declaration& declaration, const char* usage)
		: member(forward<const string>(name), declaration, usage) {}

	//virtual shared_type	declaredType() const { return type_of<T>(); }
	virtual object		dereference(const object& self) const
	{
		declaration info(typeInfo()->valueDeclaration(), self.isConstant());
		object value(info, *reinterpret_cast<void**>(self.address()));
		return value;
	}
	virtual object		append(object& /*self*/) const
	{
		ASSERT(!"Not implemented");
		return object();
	}
	virtual object		assign(object self, const object& rhs) const;
private:
};

// --------------------------------------------------------------------------------------------------------------------
template<typename T> 
object memberT<T>::assign(object self, const object& rhs) const
{
	ASSERT(self.isValid());
	ASSERT(self.typeInfo()->implements(rhs.typeInfo()));
	if (self.isValue())
	{
		self.as<T>() = rhs.as<T>();
	}
	//else if (self.isShared())
	//{
	//	self.as< shared_ptr<T> >() = rhs.as<T>();
	//}
	//else if (self.isWeak())
	//{
	//	self.as< weak_ptr<T> >() = rhs.as<T>();
	//}
	else if (self.isReference())
	{
		self.as<T*>() = rhs.as<T*>();
	}
	return self;
}

// --------------------------------------------------------------------------------------------------------------------
template<typename T, int N>
object memberT<T[N]>::assign(object self, const object& rhs) const
{
	ASSERT(self.isValid());
	ASSERT(self.typeInfo()->implements(rhs.typeInfo()));
	if (self.isValue())
	{
		T* selfArray = self.as<T*>();
		T* rhsArray = rhs.as<T*>();
		for (int element = 0; element < N; ++element)
		{
			selfArray[element] = rhsArray[element];
		}
	}
	//else if (self.isShared())
	//{
	//	self.as< shared_ptr<T> >() = rhs.as<T>();
	//}
	//else if (self.isWeak())
	//{
	//	self.as< weak_ptr<T> >() = rhs.as<T>();
	//}
	else if (self.isReference())
	{
		self.as<T**>() = rhs.as<T**>();
	}
	return self;
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
