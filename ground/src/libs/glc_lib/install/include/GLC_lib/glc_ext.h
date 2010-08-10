/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 2.0.0, packaged on July 2010.

 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/
//! \file glc_ext.h Header of the GLC Opengl extension functions.

#ifndef GLC_EXT_H_
#define GLC_EXT_H_

#include <QtOpenGL>
#include "3rdparty/glext/glext.h"

// Buffer offset used by VBO
#define BUFFER_OFFSET(i) ((char*)NULL + (i))

#if !defined(Q_OS_MAC)
// ARB_vertex_buffer_object
extern PFNGLBINDBUFFERARBPROC			glBindBuffer;
extern PFNGLDELETEBUFFERSARBPROC		glDeleteBuffers;
extern PFNGLGENBUFFERSARBPROC			glGenBuffers;
extern PFNGLISBUFFERARBPROC				glIsBuffer;
extern PFNGLBUFFERDATAARBPROC			glBufferData;
extern PFNGLBUFFERSUBDATAARBPROC		glBufferSubData;
extern PFNGLGETBUFFERSUBDATAARBPROC		glGetBufferSubData;
extern PFNGLMAPBUFFERARBPROC			glMapBuffer;
extern PFNGLUNMAPBUFFERARBPROC			glUnmapBuffer;
extern PFNGLGETBUFFERPARAMETERIVARBPROC	glGetBufferParameteriv;
extern PFNGLGETBUFFERPOINTERVARBPROC	glGetBufferPointerv;
// glDrawRangElement
//extern PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements;
// glMultiDrawElement
extern PFNGLMULTIDRAWELEMENTSPROC		glMultiDrawElements;
// GL_ARB_shader_objects
extern PFNGLCREATEPROGRAMOBJECTARBPROC	glCreateProgram;
extern PFNGLDELETEPROGRAMPROC		  	glDeleteProgram;
extern PFNGLUSEPROGRAMOBJECTARBPROC		glUseProgram;
extern PFNGLCREATESHADEROBJECTARBPROC	glCreateShader;
extern PFNGLDELETESHADERPROC			glDeleteShader;
extern PFNGLSHADERSOURCEARBPROC         glShaderSource;
extern PFNGLCOMPILESHADERARBPROC        glCompileShader;
extern PFNGLATTACHOBJECTARBPROC			glAttachShader;
extern PFNGLDETACHOBJECTARBPROC			glDetachShader;
extern PFNGLLINKPROGRAMARBPROC          glLinkProgram;
extern PFNGLGETUNIFORMLOCATIONARBPROC   glGetUniformLocation;
extern PFNGLUNIFORM4FARBPROC            glUniform4f;
extern PFNGLUNIFORM1IARBPROC            glUniform1i;
extern PFNGLGETSHADERIVPROC				glGetShaderiv;
extern PFNGLGETPROGRAMIVARBPROC			glGetProgramiv;
extern PFNGLISPROGRAMARBPROC			glIsProgram;

// GL_point_parameters Point Sprite
extern PFNGLPOINTPARAMETERFARBPROC  glPointParameterf;
extern PFNGLPOINTPARAMETERFVARBPROC glPointParameterfv;

#endif

namespace glc
{
	//! Return true if the extension is supported
	bool extensionIsSupported(const QString&);

	//! Load VBO extension
	bool loadVboExtension();

	//! Load GLSL extensions
	bool loadGlSlExtension();

	//! Load Point Sprite extension
	bool loadPointSpriteExtension();
};
#endif /*GLC_EXT_H_*/
