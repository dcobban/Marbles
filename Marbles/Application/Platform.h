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

#if defined _WIN32
	#define PLATFORM_WIN32
//	#include <application/Source/platform_win32.h>
//#elif defined _IOS
//	#define PLATFORM_IOS
//#elif defined _ANDROID
//	#define PLATFORM_ANDROID
#else
	#pragma Message("Platform error", "Unknown or unsupported platform defaulting to _WIN32.")
	#define PLATFORM_WIN32
#endif

#if defined _DEBUG
#define CONFIG_DEBUG
#elif defined _PROFILE
#define CONFIG_PROFILE
#elif defined _RETAIL
#define CONFIG_RETAIL
#elif defined NDEBUG
#define CONFIG_PROFILE
#else
	#pragma Message("Config error", "Unknown or unsupported configuration defaulting to _DEBUG.")
	#define PLATFORM_WIN32
#endif

namespace marbles
{
	class Platform
	{
	public:
		static Platform* get() { ASSERT(NULL != sInstance); return sInstance; }
	private:
		static Platform* sInstance;
	};
}
