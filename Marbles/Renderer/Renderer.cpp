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

// Bad name fix this.
#define GLERROR(exp)												\
{																	\
	exp;															\
	sm_uLastError = glGetError();									\
	if (GL_NO_ERROR != sm_uLastError)								\
	{																\
	TRACE_ERROR("OpenGL: %s\n", gluErrorString(sm_uLastError)); \
	}																\
}

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wingdi.h>
#include <renderer/renderer.h>
#include <gl\gl.h>
#include <gl\glu.h>


#define TRACE(x, ...) printf(x, __VA_ARGS__)
#if defined _DEBUG
#define VERIFY(x) ASSERT(x)
#else
#define VERIFY(x) x
#endif

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{

// --------------------------------------------------------------------------------------------------------------------
uint32_t renderer::_lastError = GL_NO_ERROR;
const PIXELFORMATDESCRIPTOR renderer::_defaultPixelFormat =
{ 
	sizeof(PIXELFORMATDESCRIPTOR),
	1, // version 1
	PFD_DRAW_TO_WINDOW |
	PFD_GENERIC_ACCELERATED |
	PFD_SUPPORT_OPENGL |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,
	32,                // Bits per pixel
	0, 0, 0, 0, 0, 0,  // Ignore colour channels
	8,                 // No Alpha
	0,                 // Alpha shift bits ignored
	0,                 // no accumulation buffer 
	0, 0, 0, 0,        // accum bits ignored
	16,                // Size of depth/z buffer
	0,
	0,
	PFD_MAIN_PLANE,
	0, 0, 0, 0 
};
// CGLTextureBuffer& renderer::sm_TextureBuffer = CGLTextureBuffer();

// --------------------------------------------------------------------------------------------------------------------
renderer::renderer()
: _windowHandle(NULL)
, _deviceContext(NULL)
, _renderingContext(NULL)
, _pixelFormatId(static_cast<uint32_t>(-1))
{
}

// --------------------------------------------------------------------------------------------------------------------
void renderer::tracePixelFormat(const PIXELFORMATDESCRIPTOR& pixelFormat)
{
	bool append = false;
	uint16_t flags = 1;
	TRACE("\tFlags: ");
	while (PFD_STEREO_DONTCARE > flags)
	{
		if (append && (pixelFormat.dwFlags & flags)) TRACE(" | ");
		switch (pixelFormat.dwFlags & flags)
		{
		case PFD_DOUBLEBUFFER:
			TRACE("PFD_DOUBLEBUFFER");
			append = true;
			break;
		case PFD_STEREO:
			TRACE("PFD_STEREO");
			append = true;
			break;
		case PFD_DRAW_TO_WINDOW:
			TRACE("PFD_DRAW_TO_WINDOW");
			append = true;
			break;
		case PFD_DRAW_TO_BITMAP:
			TRACE("PFD_DRAW_TO_BITMAP");
			append = true;
			break;
		case PFD_SUPPORT_GDI:
			TRACE("PFD_SUPPORT_GDI");
			append = true;
			break;
		case PFD_SUPPORT_OPENGL:
			TRACE("PFD_SUPPORT_OPENGL");
			append = true;
			break;
		case PFD_GENERIC_FORMAT:
			TRACE("PFD_GENERIC_FORMAT");
			append = true;
			break;
		case PFD_NEED_PALETTE:
			TRACE("PFD_NEED_PALETTE");
			append = true;
			break;
		case PFD_NEED_SYSTEM_PALETTE:
			TRACE("PFD_NEED_SYSTEM_PALETTE");
			append = true;
			break;
		case PFD_SWAP_EXCHANGE:
			TRACE("PFD_SWAP_EXCHANGE");
			append = true;
			break;
		case PFD_SWAP_COPY:
			TRACE("PFD_SWAP_COPY");
			append = true;
			break;
		case PFD_SWAP_LAYER_BUFFERS:
			TRACE("PFD_SWAP_LAYER_BUFFERS");
			append = true;
			break;
		case PFD_GENERIC_ACCELERATED:
			TRACE("PFD_GENERIC_ACCELERATED");
			append = true;
			break;
		case PFD_SUPPORT_DIRECTDRAW:
			TRACE("PFD_SUPPORT_DIRECTDRAW");
			append = true;
			break;
		}
		flags <<= 1;
	}

	TRACE("\n\tPixel Type: ");
	switch (pixelFormat.iPixelType)
	{
	case PFD_TYPE_RGBA:
		TRACE("PFD_TYPE_RGBA");
		break;
	case PFD_TYPE_COLORINDEX:
		TRACE("PFD_TYPE_COLORINDEX");
		break;
	}

	TRACE("\n\tBits per Pixel: %d (Red: %d Green: %d Blue: %d Alpha: %d)\n",
		pixelFormat.cColorBits, pixelFormat.cRedBits, pixelFormat.cGreenBits, pixelFormat.cBlueBits,
		pixelFormat.cAlphaBits);

	TRACE("\tDepth Bits: %d\n\tStencil Bits: %d\n", pixelFormat.cDepthBits,
		pixelFormat.cStencilBits);
}

// --------------------------------------------------------------------------------------------------------------------
bool renderer::setPixelFormat(const PIXELFORMATDESCRIPTOR& pixelFormat)
{
	ASSERT(NULL != _deviceContext);

	TRACE("Pixel Format requested:\n");
	tracePixelFormat(pixelFormat);

	_pixelFormatId = ::ChoosePixelFormat(reinterpret_cast<HDC>(_deviceContext), &pixelFormat);
	const bool result = ::SetPixelFormat(reinterpret_cast<HDC>(_deviceContext), _pixelFormatId, &pixelFormat);

	PIXELFORMATDESCRIPTOR pixelFormatDesc;
	memset(&pixelFormatDesc, 0, sizeof(PIXELFORMATDESCRIPTOR));

	// Enum Pixel formats
	//  TUInt uMaxPixelFormat = 0xFFFFFFFF;
	//  TRACE("Pixel Format Enumeration:");
	//  for (TUInt uPixelFormat = 1; uPixelFormat < uMaxPixelFormat; ++uPixelFormat)
	//  {
	//    uMaxPixelFormat = ::DescribePixelFormat(reinterpret_cast<HDC>(_deviceContext), uPixelFormat, 
	//                                            sizeof(PIXELFORMATDESCRIPTOR), &pixelFormat);
	//    TRACE("Pixel Format %02d:\n", uPixelFormat);
	//    TracePixelFormat(pixelFormat);
	//  }

	renderer::pixelFormat(&pixelFormatDesc);

	TRACE("Using Pixel Format:\n");
	tracePixelFormat(pixelFormatDesc);

	return result;
}

// --------------------------------------------------------------------------------------------------------------------
bool renderer::connect(const handle hWindow, const PIXELFORMATDESCRIPTOR& pixelFormat)
{
	if (hWindow != NULL)
	{
		disconnect();
		TRACE("Connecting Window (0x%08x) to marbles::renderer(0x%08x).\n", hWindow, this);
		_windowHandle = hWindow;
		reinterpret_cast<HDC&>(_deviceContext) = ::GetDC(reinterpret_cast<HWND>(_windowHandle));
		if (NULL == _deviceContext && setPixelFormat(pixelFormat))
		{
			reinterpret_cast<HGLRC&>(_renderingContext) = wglCreateContext(reinterpret_cast<HDC>(_deviceContext));
			if (NULL != _renderingContext && wglMakeCurrent(reinterpret_cast<HDC>(_deviceContext), reinterpret_cast<HGLRC>(_renderingContext)))
			{
				TRACE("\n%s OpenGL v%s\n\n", glGetString(GL_VENDOR), glGetString(GL_VERSION));
				enable(DEPTH_TEST);
				clearColour(0.0f, 0.0f, 0.0f, 1.0f);
				return true;
			}
		}
	}

	disconnect();
	TRACE("Unable to connect window(0x%08x) to marbles::renderer(0x%08x).\n", hWindow, this);
	return false;
}

bool renderer::disconnect()
{
	const bool renderingContextEnabled = NULL != _deviceContext && NULL != _renderingContext;
	if (renderingContextEnabled)
	{
		TRACE("Disconnecting Window (0x%08x) from marbles::renderer(0x%08x).\n", _windowHandle, this);
		wglMakeCurrent(reinterpret_cast<HDC>(_deviceContext), NULL);
		wglDeleteContext(reinterpret_cast<HGLRC>(_renderingContext));
		VERIFY(::ReleaseDC(reinterpret_cast<HWND>(_windowHandle), reinterpret_cast<HDC>(_deviceContext)));
		_renderingContext = NULL;
		_deviceContext = NULL;
		_windowHandle = NULL;
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of file --------------------------------------------------------------------------------------------------------
