/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 1.2.0, packaged on September 2009.

 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/
//! \file glc_ext.cpp implementation of the GLC Opengl extension functions.

#include "glc_ext.h"
#include <QString>
#include <QGLContext>
#include <QDebug>

#if !defined(Q_OS_MAC)
// ARB_vertex_buffer_object
PFNGLBINDBUFFERARBPROC				glBindBuffer			= NULL;
PFNGLDELETEBUFFERSARBPROC			glDeleteBuffers			= NULL;
PFNGLGENBUFFERSARBPROC				glGenBuffers			= NULL;
PFNGLISBUFFERARBPROC				glIsBuffer				= NULL;
PFNGLBUFFERDATAARBPROC				glBufferData			= NULL;
PFNGLBUFFERSUBDATAARBPROC			glBufferSubData			= NULL;
PFNGLGETBUFFERSUBDATAARBPROC		glGetBufferSubData		= NULL;
PFNGLMAPBUFFERARBPROC				glMapBuffer				= NULL;
PFNGLUNMAPBUFFERARBPROC				glUnmapBuffer			= NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC	glGetBufferParameteriv	= NULL;
PFNGLGETBUFFERPOINTERVARBPROC		glGetBufferPointerv		= NULL;
// glDrawRangElement
//PFNGLDRAWRANGEELEMENTSPROC 			glDrawRangeElements		= NULL;

// glMultiDrawElement
PFNGLMULTIDRAWELEMENTSPROC			glMultiDrawElements		= NULL;
// GL_ARB_shader_objects
PFNGLCREATEPROGRAMOBJECTARBPROC		glCreateProgram			= NULL;
PFNGLDELETEPROGRAMPROC				glDeleteProgram			= NULL;
PFNGLUSEPROGRAMOBJECTARBPROC		glUseProgram			= NULL;
PFNGLCREATESHADEROBJECTARBPROC		glCreateShader			= NULL;
PFNGLDELETESHADERPROC				glDeleteShader			= NULL;
PFNGLSHADERSOURCEARBPROC			glShaderSource			= NULL;
PFNGLCOMPILESHADERARBPROC			glCompileShader			= NULL;
PFNGLATTACHOBJECTARBPROC			glAttachShader			= NULL;
PFNGLDETACHOBJECTARBPROC			glDetachShader			= NULL;
PFNGLLINKPROGRAMARBPROC				glLinkProgram			= NULL;
PFNGLGETUNIFORMLOCATIONARBPROC		glGetUniformLocation	= NULL;
PFNGLUNIFORM4FARBPROC				glUniform4f				= NULL;
PFNGLUNIFORM1IARBPROC				glUniform1i				= NULL;
PFNGLGETSHADERIVPROC				glGetShaderiv			= NULL;
PFNGLGETPROGRAMIVARBPROC			glGetProgramiv			= NULL;
PFNGLISPROGRAMARBPROC				glIsProgram				= NULL;

// GL_point_parameters Point Sprite
PFNGLPOINTPARAMETERFARBPROC			glPointParameterf		= NULL;
PFNGLPOINTPARAMETERFVARBPROC		glPointParameterfv		= NULL;

#endif

//const QString glExtension(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));

// Return true if the extension is supported
bool glc::extensionIsSupported(const QString& extension)
{
	QString glExtension(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
	return glExtension.contains(extension);
}

// Return true if VBO extension is succesfully loaded
bool glc::loadVboExtension()
{
	bool result= true;
#if !defined(Q_OS_MAC)
	const QGLContext* pContext= QGLContext::currentContext();
    glBindBuffer				= (PFNGLBINDBUFFERARBPROC)pContext->getProcAddress(QLatin1String("glBindBuffer"));
    glDeleteBuffers				= (PFNGLDELETEBUFFERSARBPROC)pContext->getProcAddress(QLatin1String("glDeleteBuffers"));
    glGenBuffers				= (PFNGLGENBUFFERSARBPROC)pContext->getProcAddress(QLatin1String("glGenBuffers"));
    glIsBuffer					= (PFNGLISBUFFERARBPROC)pContext->getProcAddress(QLatin1String("glIsBuffer"));
    glBufferData				= (PFNGLBUFFERDATAARBPROC)pContext->getProcAddress(QLatin1String("glBufferData"));
    glBufferSubData				= (PFNGLBUFFERSUBDATAARBPROC)pContext->getProcAddress(QLatin1String("glBufferSubData"));
    glGetBufferSubData			= (PFNGLGETBUFFERSUBDATAARBPROC)pContext->getProcAddress(QLatin1String("glGetBufferSubData"));
    glMapBuffer					= (PFNGLMAPBUFFERARBPROC)pContext->getProcAddress(QLatin1String("glMapBuffer"));
    glUnmapBuffer				= (PFNGLUNMAPBUFFERARBPROC)pContext->getProcAddress(QLatin1String("glUnmapBuffer"));
    glGetBufferParameteriv		= (PFNGLGETBUFFERPARAMETERIVARBPROC)pContext->getProcAddress(QLatin1String("glGetBufferParameteriv"));
    glGetBufferPointerv			= (PFNGLGETBUFFERPOINTERVARBPROC)pContext->getProcAddress(QLatin1String("glGetBufferPointerv"));
    //glDrawRangeElements			= (PFNGLDRAWRANGEELEMENTSPROC)pContext->getProcAddress(QLatin1String("glDrawRangeElements"));
    glMultiDrawElements			= (PFNGLMULTIDRAWELEMENTSPROC)pContext->getProcAddress(QLatin1String("glMultiDrawElements"));

    result= glBindBuffer and glDeleteBuffers and glGenBuffers and glIsBuffer and glBufferData and glBufferSubData and
    glGetBufferSubData and glMapBuffer and glUnmapBuffer and glGetBufferParameteriv and glGetBufferPointerv and glMultiDrawElements;// and glDrawRangeElements;
#endif
    return result;

}

// Load GLSL extensions
bool glc::loadGlSlExtension()
{
	bool result= true;
#if !defined(Q_OS_MAC)
	const QGLContext* pContext= QGLContext::currentContext();
	glCreateProgram				= (PFNGLCREATEPROGRAMOBJECTARBPROC)pContext->getProcAddress(QLatin1String("glCreateProgram"));
	if (not glCreateProgram) qDebug() << "not glCreateProgram";
	glDeleteProgram 			= (PFNGLDELETEPROGRAMPROC)pContext->getProcAddress(QLatin1String("glDeleteProgram"));
	if (not glDeleteProgram) qDebug() << "not glDeleteProgram";
	glUseProgram				= (PFNGLUSEPROGRAMOBJECTARBPROC)pContext->getProcAddress(QLatin1String("glUseProgram"));
	if (not glUseProgram) qDebug() << "not glUseProgram";
	glCreateShader				= (PFNGLCREATESHADEROBJECTARBPROC)pContext->getProcAddress(QLatin1String("glCreateShader"));
	if (not glCreateShader) qDebug() << "not glCreateShader";
	glDeleteShader				= (PFNGLDELETESHADERPROC)pContext->getProcAddress(QLatin1String("glDeleteShader"));
	if (not glDeleteShader) qDebug() << "not glDeleteShader";
	glShaderSource				= (PFNGLSHADERSOURCEARBPROC)pContext->getProcAddress(QLatin1String("glShaderSource"));
	if (not glShaderSource) qDebug() << "not glShaderSource";
	glCompileShader				= (PFNGLCOMPILESHADERARBPROC)pContext->getProcAddress(QLatin1String("glCompileShader"));
	if (not glCompileShader) qDebug() << "not glCompileShader";
	glAttachShader				= (PFNGLATTACHOBJECTARBPROC)pContext->getProcAddress(QLatin1String("glAttachShader"));
	if (not glAttachShader) qDebug() << "not glAttachShader";
	glDetachShader				= (PFNGLDETACHOBJECTARBPROC)pContext->getProcAddress(QLatin1String("glDetachShader"));
	if (not glDetachShader) qDebug() << "not glDetachShader";
	glLinkProgram				= (PFNGLLINKPROGRAMARBPROC)pContext->getProcAddress(QLatin1String("glLinkProgram"));
	if (not glLinkProgram) qDebug() << "not glLinkProgram";
	glGetUniformLocation		= (PFNGLGETUNIFORMLOCATIONARBPROC)pContext->getProcAddress(QLatin1String("glGetUniformLocation"));
	if (not glGetUniformLocation) qDebug() << "not glGetUniformLocation";
	glUniform4f					= (PFNGLUNIFORM4FARBPROC)pContext->getProcAddress(QLatin1String("glUniform4f"));
	if (not glUniform4f) qDebug() << "not glUniform4f";
	glUniform1i					= (PFNGLUNIFORM1IARBPROC)pContext->getProcAddress(QLatin1String("glUniform1i"));
	if (not glUniform1i) qDebug() << "not glUniform1i";
	glGetShaderiv				= (PFNGLGETSHADERIVPROC)pContext->getProcAddress(QLatin1String("glGetShaderiv"));
	if (not glGetShaderiv) qDebug() << "not glGetShaderiv";
	glGetProgramiv				= (PFNGLGETPROGRAMIVARBPROC)pContext->getProcAddress(QLatin1String("glGetProgramiv"));
	if (not glGetProgramiv) qDebug() << "not glGetProgramiv";
	glIsProgram					= (PFNGLISPROGRAMARBPROC)pContext->getProcAddress(QLatin1String("glIsProgram"));

	result= glCreateProgram and glDeleteProgram and glUseProgram and glCreateShader and glDeleteShader and
    glShaderSource and glCompileShader and glAttachShader and glDetachShader and glLinkProgram and
    glGetUniformLocation and glUniform4f and glUniform1i and glGetShaderiv and glGetProgramiv and glIsProgram;

#endif
    return result;
}

// Load Point Sprite extension
bool glc::loadPointSpriteExtension()
{
	bool result= true;
#if !defined(Q_OS_MAC)
	const QGLContext* pContext= QGLContext::currentContext();
	glPointParameterf				= (PFNGLPOINTPARAMETERFARBPROC)pContext->getProcAddress(QLatin1String("glPointParameterf"));
	if (not glPointParameterf) qDebug() << "not glPointParameterf";
	glPointParameterfv				= (PFNGLPOINTPARAMETERFVARBPROC)pContext->getProcAddress(QLatin1String("glPointParameterfv"));
	if (not glPointParameterfv) qDebug() << "not glPointParameterfv";

	result= glPointParameterf and glPointParameterfv;

#endif
    return result;
}

