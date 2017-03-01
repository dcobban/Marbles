// This source file is part of marbles library.
//
// Copyright (c) 2014 Dan Cobban
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

TEST(window, basic_operations)
{
    int update = 1000;
    int x[3] = { 25 }, y[3] = { 25 };
    int closed = 0;
    marbles::window win;
    marbles::window::builder builder;

    builder.position(x[0], y[0]);
    win.onReposition += [&x, &y](marbles::window* win, int xPos, int yPos)
    {
        (void)win;
        x[1] = xPos;
        y[1] = yPos;
    };
    win.onClose += [&closed](marbles::window* win)
    {
        (void)win;
        closed = 1;
    };
    builder.create(&win);

    win.position(&x[2], &y[2]);
    while (win.is_open())
    {
        win.poll();
        if (!(--update))
            win.close();
    }
    EXPECT_EQ(x[0], x[1]);
    EXPECT_EQ(y[0], y[1]);
    EXPECT_EQ(x[1], x[2]);
    EXPECT_EQ(y[1], y[2]);
    EXPECT_EQ(1, closed);
    win.poll();
    win.close();
}

TEST(window, application_windows)
{
    // EXPECT_EQ(count, info->members().size());
}
