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
#include <platform/device.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
    VkSurfaceKHR _surface;

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
: _internal(new internal())
{
}

// --------------------------------------------------------------------------------------------------------------------
bool window::fullscreen() const
{
    return _internal && glfwGetWindowMonitor(_internal->_window);
}

// --------------------------------------------------------------------------------------------------------------------
void window::size(int width, int height)
{
    if (is_open())
    {
        glfwSetWindowSize(_internal->_window, width, height);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::size(int* width, int* height) const
{
    if (is_open())
    {
        glfwGetWindowSize(_internal->_window, width, height);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::position(int x, int y)
{
    if (is_open())
    {
        glfwSetWindowPos(_internal->_window, x, y);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::position(int* x, int* y) const
{
    if (is_open())
    {
        glfwGetWindowPos(_internal->_window, x, y);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::cursor(double x, double y)
{
    if (is_open())
    {
        glfwSetCursorPos(_internal->_window, x, y);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::cursor(double* x, double* y) const
{
    if (is_open())
    {
        glfwGetCursorPos(_internal->_window, x, y);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::hide()
{
    if (is_open())
    {
        glfwHideWindow(_internal->_window);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void window::show()
{
    if (is_open())
    {
        glfwShowWindow(_internal->_window);
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
    return _internal && nullptr != _internal->_window && !glfwWindowShouldClose(_internal->_window);
}

// --------------------------------------------------------------------------------------------------------------------
int window::close()
{
    assert(_internal);
    int result = -1;
    if (is_open())
    {
        onClose(this);
        
        if (_internal->_surface)
        {
            // vkDestroySurfaceKHR(instance, _surface, allocator);
        }
        glfwDestroyWindow(_internal->_window);
        _internal->_window = nullptr;
        result = 0;
    }
    return result;
}

// --------------------------------------------------------------------------------------------------------------------
window::builder::builder(/*allocator*/)
: _win(nullptr)
, _name("Marbles")
, _width(1280)
, _height(720)
, _fullscreen(false)
, _validation(false)
{
    _pre.reserve(8);
    _pre.push_back(async(launch::deferred, [this]() -> int
    {
        glfwDefaultWindowHints();
        if (glfwVulkanSupported())
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // use vulkan
        }
        return 0;
    }));

    _post.reserve(8);
    _post.push_back(async(launch::deferred, [this]() -> int
    {
        GLFWwindow* glfwWin = this->_win->_internal->_window;
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
void window::builder::validation(bool enable)
{
    _validation = enable;
}

// --------------------------------------------------------------------------------------------------------------------
bool window::builder::bind(marbles::device* rasterizer)
{
    (void)rasterizer;
    return false;
}

// --------------------------------------------------------------------------------------------------------------------
int window::builder::create(window* win)
{
    int result = -1;
    _win = win;
    if (_win)
    {
        _win->_internal.reset(new window::internal());
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
            _win->_internal->_window = glfwCreateWindow(_width, _height, _name, monitor, nullptr);

            if (_win->_internal->_window)
            {
                // VDeleter<VkSurfaceKHR> surface { instance, vkDestroySurfaceKHR };

                //if (glfwCreateWindowSurface(instance, _win->_internal->_window, nullptr, _surface) != VK_SUCCESS)
                //{
                //}
                //_win->_internal->_surface;
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
