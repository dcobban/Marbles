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

namespace marbles
{
	extern int allocs; // = 0;
} // namespace marbles

inline void* operator new(size_t size/*, const char* filename = __FILE__, size_t line = __LINE__*/)
{
	void* p = malloc(size);
	printf("%s(%d): Alloc(%d) 0x%X\n", "", 0, ++marbles::allocs, (unsigned)(p));
	return p;
}

inline void* operator new[](size_t size, size_t N/*, const char* filename = __FILE__, size_t line = __LINE__*/)
{
	void* p = malloc(size*N);
	printf("%s(%d): Alloc(%d) 0x%X\n", "", 0, ++marbles::allocs, (unsigned)(p));
	return p;
}

inline void operator delete(void*p)
{
//	printf("%s(%d): free 0x%X\n", "", 0, (unsigned)(p));
	free(p);
}

inline void operator delete[](void*p)
{
//	printf("%s(%d): free 0x%X\n", "", 0, (unsigned)(p));
	free(p);
}

// End of file --------------------------------------------------------------------------------------------------------
