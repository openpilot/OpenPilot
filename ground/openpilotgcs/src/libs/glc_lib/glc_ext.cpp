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

    result= glBindBuffer && glDeleteBuffers && glGenBuffers && glIsBuffer && glBufferData && glBufferSubData &&
    glGetBufferSubData && glMapBuffer && glUnmapBuffer && glGetBufferParameteriv && glGetBufferPointerv && glMultiDrawElements;// and glDrawRangeElements;
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
	if (!glCreateProgram) qDebug() << "not glCreateProgram";
	glDeleteProgram 			= (PFNGLDELETEPROGRAMPROC)pContext->getProcAddress(QLatin1String("glDeleteProgram"));
	if (!glDeleteProgram) qDebug() << "not glDeleteProgram";
	glUseProgram				= (PFNGLUSEPROGRAMOBJECTARBPROC)pContext->getProcAddress(QLatin1String("glUseProgram"));
	if (!glUseProgram) qDebug() << "not glUseProgram";
	glCreateShader				= (PFNGLCREATESHADEROBJECTARBPROC)pContext->getProcAddress(QLatin1String("glCreateShader"));
	if (!glCreateShader) qDebug() << "not glCreateShader";
	glDeleteShader				= (PFNGLDELETESHADERPROC)pContext->getProcAddress(QLatin1String("glDeleteShader"));
	if (!glDeleteShader) qDebug() << "not glDeleteShader";
	glShaderSource				= (PFNGLSHADERSOURCEARBPROC)pContext->getProcAddress(QLatin1String("glShaderSource"));
	if (!glShaderSource) qDebug() << "not glShaderSource";
	glCompileShader				= (PFNGLCOMPILESHADERARBPROC)pContext->getProcAddress(QLatin1String("glCompileShader"));
	if (!glCompileShader) qDebug() << "not glCompileShader";
	glAttachShader				= (PFNGLATTACHOBJECTARBPROC)pContext->getProcAddress(QLatin1String("glAttachShader"));
	if (!glAttachShader) qDebug() << "not glAttachShader";
	glDetachShader				= (PFNGLDETACHOBJECTARBPROC)pContext->getProcAddress(QLatin1String("glDetachShader"));
	if (!glDetachShader) qDebug() << "not glDetachShader";
	glLinkProgram				= (PFNGLLINKPROGRAMARBPROC)pContext->getProcAddress(QLatin1String("glLinkProgram"));
	if (!glLinkProgram) qDebug() << "not glLinkProgram";
	glGetUniformLocation		= (PFNGLGETUNIFORMLOCATIONARBPROC)pContext->getProcAddress(QLatin1String("glGetUniformLocation"));
	if (!glGetUniformLocation) qDebug() << "not glGetUniformLocation";
	glUniform4f					= (PFNGLUNIFORM4FARBPROC)pContext->getProcAddress(QLatin1String("glUniform4f"));
	if (!glUniform4f) qDebug() << "not glUniform4f";
	glUniform1i					= (PFNGLUNIFORM1IARBPROC)pContext->getProcAddress(QLatin1String("glUniform1i"));
	if (!glUniform1i) qDebug() << "not glUniform1i";
	glGetShaderiv				= (PFNGLGETSHADERIVPROC)pContext->getProcAddress(QLatin1String("glGetShaderiv"));
	if (!glGetShaderiv) qDebug() << "not glGetShaderiv";
	glGetProgramiv				= (PFNGLGETPROGRAMIVARBPROC)pContext->getProcAddress(QLatin1String("glGetProgramiv"));
	if (!glGetProgramiv) qDebug() << "not glGetProgramiv";
	glIsProgram					= (PFNGLISPROGRAMARBPROC)pContext->getProcAddress(QLatin1String("glIsProgram"));

	result= glCreateProgram && glDeleteProgram && glUseProgram && glCreateShader && glDeleteShader &&
    glShaderSource && glCompileShader && glAttachShader && glDetachShader && glLinkProgram &&
    glGetUniformLocation && glUniform4f && glUniform1i && glGetShaderiv && glGetProgramiv && glIsProgram;

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
	if (!glPointParameterf) qDebug() << "not glPointParameterf";
	glPointParameterfv				= (PFNGLPOINTPARAMETERFVARBPROC)pContext->getProcAddress(QLatin1String("glPointParameterfv"));
	if (!glPointParameterfv) qDebug() << "not glPointParameterfv";

	result= glPointParameterf && glPointParameterfv;

#endif
    return result;
}

