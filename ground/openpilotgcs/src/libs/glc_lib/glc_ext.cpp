/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
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
#include <QGLShaderProgram>

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
    return QGLShaderProgram::hasOpenGLShaderPrograms();
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

