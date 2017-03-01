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

#include <platform/window.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles {

// --------------------------------------------------------------------------------------------------------------------
struct window::internal
{
    struct glfw
    {
        glfw()
        {
            glfwInit();
        }
        ~glfw()
        {
            glfwTerminate();
        }
        void poll()
        {
            glfwPollEvents();
        }
    };

    internal()
    {
        _window = nullptr;
        _cursor = nullptr;
        _glfw = s_glfw.lock();

        if (!_glfw)
        {
            _glfw = make_shared<glfw>();
            s_glfw = _glfw;
        }
    }

    shared_ptr<glfw> _glfw;
    GLFWwindow* _window;
    GLFWcursor* _cursor;

    static weak_ptr<glfw> s_glfw;
};

// --------------------------------------------------------------------------------------------------------------------
weak_ptr<window::internal::glfw> window::internal::s_glfw;

// --------------------------------------------------------------------------------------------------------------------
void window::delete_internal::operator()(internal* win) const
{
    delete win;
}

// --------------------------------------------------------------------------------------------------------------------
window::window() 
: _win(new internal())
{
}

// --------------------------------------------------------------------------------------------------------------------
bool window::fullscreen() const
{
    return _win && glfwGetWindowMonitor(_win->_window);
}

// --------------------------------------------------------------------------------------------------------------------
void window::size(int width, int height)
{
    if (is_open())
    {
        glfwSetWindowSize(_win->_window, width, height);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::size(int* width, int* height) const
{
    if (is_open())
    {
        glfwGetWindowSize(_win->_window, width, height);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::position(int x, int y)
{
    if (is_open())
    {
        glfwSetWindowPos(_win->_window, x, y);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::position(int* x, int* y) const
{
    if (is_open())
    {
        glfwGetWindowPos(_win->_window, x, y);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::cursor(double x, double y)
{
    if (is_open())
    {
        glfwSetCursorPos(_win->_window, x, y);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::cursor(double* x, double* y) const
{
    if (is_open())
    {
        glfwGetCursorPos(_win->_window, x, y);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::hide()
{
    if (is_open())
    {
        glfwHideWindow(_win->_window);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::show()
{
    if (is_open())
    {
        glfwShowWindow(_win->_window);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::poll() const
{
    glfwPollEvents();
}

// --------------------------------------------------------------------------------------------------------------------
bool window::is_open() const
{
    return _win && nullptr != _win->_window && !glfwWindowShouldClose(_win->_window);
}

// --------------------------------------------------------------------------------------------------------------------
int window::close()
{
    assert(_win);
    int result = -1;
    if (is_open())
    {
        onClose(this);
        glfwDestroyWindow(_win->_window);
        _win->_window = nullptr;
        result = 0;
    }
    return result;
}

// --------------------------------------------------------------------------------------------------------------------
window::builder::builder(/*allocator*/)
: _win(nullptr)
, _name("Marble Window")
, _width(1280)
, _height(720)
, _fullscreen(false)
{
    _pre.reserve(8);
    _pre.push_back(async(launch::deferred, [this]() -> int
    {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // using vulkan
        return 0;
    }));

    _post.reserve(8);
    _post.push_back(async(launch::deferred, [this]() -> int
    {
        GLFWwindow* glfwWin = this->_win->_win->_window;
        glfwSetWindowUserPointer(glfwWin, this->_win);
        glfwSetWindowPosCallback(glfwWin, [](GLFWwindow* glfwWin, int x, int y)
        {
            window* win = reinterpret_cast<window*>(glfwGetWindowUserPointer(glfwWin));
            win->onReposition(win, x, y);
        });
        glfwSetWindowCloseCallback(glfwWin, [](GLFWwindow* glfwWin)
        {
            window* win = reinterpret_cast<window*>(glfwGetWindowUserPointer(glfwWin));
            win->onClose(win);
        });

        return 0;
    }));
}

// --------------------------------------------------------------------------------------------------------------------
void window::builder::name(const char* name)
{
    _name = name; // Should probably copy this string.
}

// --------------------------------------------------------------------------------------------------------------------
void window::builder::size(int width, int height)
{
    _width = width;
    _height = height;
}

// --------------------------------------------------------------------------------------------------------------------
void window::builder::position(int x, int y)
{
    _post.push_back(async(launch::deferred, [this, x, y]() -> int
    {   
        this->_win->position(x, y);
        return 0;
    }));
}

// --------------------------------------------------------------------------------------------------------------------
void window::builder::visible(bool enable)
{
    _post.push_back(async(launch::deferred, [this, enable]() -> int
    {
        if (enable)
        {
            this->_win->show();
        }
        else
        {
            this->_win->hide();
        }
        return 0;
    }));
}

// --------------------------------------------------------------------------------------------------------------------
void window::builder::fullscreen(bool enable)
{
    _fullscreen = enable;
}

// --------------------------------------------------------------------------------------------------------------------
int window::builder::create(window* win)
{
    int result = -1;
    _win = win;
    if (_win)
    {
        _win->_win.reset(new window::internal());
        for (auto& task : _pre)
        {
            result = task.get();
            if (0 > result)
            {
                break;
            }
        }

        if (0 <= result)
        {
            GLFWmonitor* monitor = nullptr;
            if (_fullscreen)
            {
                monitor = glfwGetPrimaryMonitor();
            }
            _win->_win->_window = glfwCreateWindow(_width, _height, _name, monitor, nullptr);

            if (_win->_win->_window)
            {
                for (auto& task : _post)
                {
                    result = task.get();
                    if (0 > result)
                    {
                        break;
                    }
                }
            }
        }
    }
    _win = nullptr;
    return result;
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles 

// End of file --------------------------------------------------------------------------------------------------------
