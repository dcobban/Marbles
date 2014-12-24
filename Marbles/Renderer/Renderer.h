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

#pragma once

typedef struct tagPIXELFORMATDESCRIPTOR PIXELFORMATDESCRIPTOR;

// --------------------------------------------------------------------------------------------------------------------
namespace marbles
{
//class scene;	// the scene graph
//class entity;	// a location in space associtated with geometry
//class terrain;	// the terrain
//class panel;	// an interface panel
//
//class texture;  // an image container
//class shader;	// a GPU program

class renderer
{
public:
	typedef enum {
		COLOUR_BUFFER = 0x00004000, // GL_COLOR_BUFFER_BIT,
		DEPTH_BUFFER = 0x00000100, // GL_DEPTH_BUFFER_BIT,
		STENCIL_BUFFER = 0x00000400, // GL_STENCIL_BUFFER_BIT,
	} BufferType;

	typedef enum {
		DEPTH_TEST = 0x0B71, // GL_DEPTH_TEST,
		CULL_FACE = 0x0B44, // GL_CULL_FACE,
	} Options;

	typedef enum {
		points,
		lines,
		line_loop,
		line_strip,
		triangles,
		triangle_strip,
		triangle_fan,
		quads,
		quad_strip,
		polygon
	} DrawMode;

				renderer();
				~renderer();

	bool		connect(handle khWnd, const PIXELFORMATDESCRIPTOR& pf = _defaultPixelFormat);
	bool		disconnect();
	void		pixelFormat(PIXELFORMATDESCRIPTOR* pPixelFormat) const;

	static void	clearColour(float red, float green, float blue, float alpha = 1.0f);
	// Perspective
	static void setViewport(int32_t left, int32_t bottom, int32_t width, int32_t height);
	// static void setFrustum(TBox& kBox);
	static void enable(uint16_t dwFlags, bool kbEnable = true);

	// Draw
	//static void setVertexBuffer(CVertex3* kpVertices);
	static void drawElements(DrawMode drawMode, uint32_t kuSize, uint32_t* pIndices);

private:
	static void CheckError();
	static void tracePixelFormat(const PIXELFORMATDESCRIPTOR& kPF);

	bool		setPixelFormat(const PIXELFORMATDESCRIPTOR& kPixelFormat);

	static const PIXELFORMATDESCRIPTOR _defaultPixelFormat;
	static uint32_t _lastError;

	handle			_windowHandle;		// Handle to the rendering Window
	handle			_deviceContext;		// Handle to the device context
	handle			_renderingContext;	// Handle to the OpenGL rendering context
	uint32_t		_pixelFormatId;		// Pixel format identification
};

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles

// End of File --------------------------------------------------------------------------------------------------------
