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

#include <application/Platform.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Marbles
{
namespace 
{
// --------------------------------------------------------------------------------------------------------------------
class Platform_Win32 : public Platform
{
private:
	static TCHAR sgMarblesWindow[];

	// --------------------------------------------------------------------------------------------------------------------
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		int wmId, wmEvent;
	//	PAINTSTRUCT ps;
	//	HDC hdc;

		switch (message)
		{
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			//switch (wmId)
			//{
			//case IDM_ABOUT:
			//	DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			//	break;
			//case IDM_EXIT:
			//	DestroyWindow(hWnd);
			//	break;
			//default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			//}
			break;
		case WM_PAINT:
			//hdc = BeginPaint(hWnd, &ps);
			//// TODO: Add any drawing code here...
			//EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

	// --------------------------------------------------------------------------------------------------------------------
	ATOM RegisterMarblesWindow(HINSTANCE hInstance)
	{
		WNDCLASSEX wcex;

		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL; // LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEST));
		wcex.hCursor		= NULL; // LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= NULL; // MAKEINTRESOURCE(IDC_TEST);
		wcex.lpszClassName	= sgMarblesWindow;
		wcex.hIconSm		= NULL; // LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		return RegisterClassEx(&wcex);
	}
};

// --------------------------------------------------------------------------------------------------------------------
TCHAR Platform_Win32::sgMarblesWindow[] = TEXT("MarblesWindow");

// --------------------------------------------------------------------------------------------------------------------
static Platform_Win32 sPlatform;

// --------------------------------------------------------------------------------------------------------------------
} // namespace <>

// --------------------------------------------------------------------------------------------------------------------
Platform* Platform::sInstance = &sPlatform;

} // namespace Marbles

// End of file --------------------------------------------------------------------------------------------------------
