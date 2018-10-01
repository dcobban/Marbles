// This source file is part of marbles library.
//
// Copyright (c) 2017 Dan Cobban
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

#include <application/event.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

class device
{
public:
    class manager;
    class iterator;
};

// --------------------------------------------------------------------------------------------------------------------
class device::manager
{
public:
    manager(const char* applicationName, uint32_t version, bool enableValidationLayers = false);

    device::iterator begin();
    device::iterator end();

    typedef event<void(size_t location, int32_t code, const char* layerPrefix, const char* msg)> DebugEvent;
    DebugEvent DebugInfo;
    DebugEvent DebugWarning;
    DebugEvent DebugError;
    DebugEvent DebugPerformance;

private:
    struct instance;

    shared_ptr<instance> _self; // Using shared pointer as unique pointer requires struct definition
};

// --------------------------------------------------------------------------------------------------------------------
class device::iterator
{
public:
    device::iterator operator++(int);
    device::iterator operator++();

    device& operator*();
    device* operator->();

    device* get();
    const device* get() const;
private:
    weak_ptr<device> _device;
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
