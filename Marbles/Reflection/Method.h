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
template<typename R, typename T> 
class memberT<R (T::*)()> : public memberT<R>
{
public:
	typedef R return_type;
	typedef R (T::*member_type)();
	typedef R (T::*signature_type)();

	memberT(const std::string& name, signature_type method, const char* usage)
	: memberT<return_type>(name, declarationT<return_type>(), usage)
	, mMethod(method)
	{
	}

	virtual bool		callable() const { return true; }
	virtual bool		readOnly() const { return true; }

	virtual object		dereference(const object& self) const
	{
		declaration info(this->shared_from_this(), declarationT<member_type>());
		object value(info, self.address());
		return value;
	}
	virtual object		assign(object& self, const object& rhs) const
	{
		return member::assign(self, rhs);
	}
	virtual object		call(object& self, object* pObjs, unsigned count) const
	{
		object result;
		// if (count == function_traits<
		// validateParameters(pObjs, count);
		// validate each parameter
		// execute callback
	}

private:
	member_type mMethod;
};

template<typename R, typename T> struct type_of_t<R (T::*)()>
{ 
	static shared_type typeInfo() { return type_of< std::function<R ()> >(); } 
};

// --------------------------------------------------------------------------------------------------------------------
template<typename R, typename T> 
class memberT<R (T::*)() const> : public memberT<R>
{
public:
	typedef R return_type;
	typedef R (T::*member_type)() const;
	typedef R (T::*signature_type)() const;

	memberT(const std::string& name, signature_type method, const char* usage)
	: memberT<return_type>(name, declarationT<return_type>(), usage)
	, mMethod(method)
	{
	}

	virtual bool		callable() const { return true; }
	virtual bool		readOnly() const { return true; }

	virtual object		dereference(const object& self) const
	{
		declaration info(this->shared_from_this(), declarationT<member_type>());
		object value(info, self.address());
		return value;
	}
	virtual object		assign(object& self, const object& rhs) const
	{
		return member::assign(self, rhs);
	}
	virtual object		call(object& self, object* pObjs, unsigned count) const
	{
		(void)count; (void)pObjs; (void)self;
		object result;
		// if (count == function_traits<
		// validateParameters(pObjs, count);
		// validate each parameter
		// execute callback
		return result;
	}

private:
	member_type mMethod;
};

template<typename R, typename T> struct type_of_t<R (T::*)() const>
{ 
	static shared_type typeInfo() { return type_of< std::function<R ()> >(); } 
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace reflection
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
