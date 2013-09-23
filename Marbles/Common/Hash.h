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
#pragma warning(disable:4706)
// --------------------------------------------------------------------------------------------------------------------
namespace Marbles
{
typedef size_t hash_t;

// --------------------------------------------------------------------------------------------------------------------
class hash
{
public:
	// http://www.cse.yorku.ca/~oz/hash.html
	static inline hash_t sdbm(const char* string)
	{
        hash_t hash = 0;
        int c;

        while (c = *string++)
		{
            hash = c + (hash << 6) + (hash << 16) - hash;
		}

        return hash;
	}

	static inline hash_t sdbm(const void* data, size_t size)
	{
		const unsigned char* pos = static_cast<const unsigned char*>(data);
		const unsigned char* end = pos + size;
        hash_t hash = 0;
        int c;

        while (pos != end && (c = *pos++))
		{
            hash = c + (hash << 6) + (hash << 16) - hash;
		}

        return hash;
	}

	// http://www.cse.yorku.ca/~oz/hash.html
    static inline hash_t djb2(const char *string)
    {
        hash_t hash = 5381;
        int c;

        while (c = *string++)
		{
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		}

        return hash;
    }

	static inline hash_t djb2(const void *data, size_t size)
    {
		const unsigned char* pos = static_cast<const unsigned char*>(data);
		const unsigned char* end = pos + size;
        hash_t hash = 5381;

        while (pos != end)
		{
			int c = *pos++;
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		}

        return hash;
    }
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace Marbles

#pragma warning(default:4706)

// End of file --------------------------------------------------------------------------------------------------------
