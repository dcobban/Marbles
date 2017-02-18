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

#include <Application/event.h>

struct GLFWmonitor;

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
class window
{
public:
    window();

    void size(int width, int height);
    void size(int* width, int* height) const;
    void position(int x, int y);
    void position(int* x, int* y) const;
    void cursor(double x, double y);
    void cursor(double* x, double* y) const;

    bool visible() const;
    void visible(bool enable);
    bool fullscreen() const;
    void fullscreen(bool enable);

    bool is_open() const;
    void hide();
    void show();
    void poll() const;
    int  close();

    event<window*> onClose;
    event<window*, int, int> onResize;
    event<window*, int, int> onReposition;

    class builder;
private:
    struct internal;
    struct delete_internal
    {
        void operator()(internal* _win) const;
    };
    typedef unique_ptr<internal, delete_internal> unique_internal;

    unique_internal _win;
}; // class window

// --------------------------------------------------------------------------------------------------------------------
class window::builder
{
public:
    builder();

    void name(const char* name);
    void size(int width, int height);
    void position(int x, int y);
    void visible(bool enable);
    void fullscreen(bool enable);

    int create(window*);
private:
    vector<future<int>> _pre;
    vector<future<int>> _post;

    window* _win;
    const char* _name;
    int _width;
    int _height;
    bool _fullscreen;
}; // class window::builder

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
