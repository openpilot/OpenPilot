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
#include <QGLBuffer>

#if !defined(Q_OS_MAC)

// GL_point_parameters Point Sprite
PFNGLPOINTPARAMETERFARBPROC			glPointParameterf		= NULL;
PFNGLPOINTPARAMETERFVARBPROC		glPointParameterfv		= NULL;

#endif


// Return true if the extension is supported
bool glc::extensionIsSupported(const QString& extension)
{
	QString glExtension(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
	return glExtension.contains(extension);
}

// Return true if VBO extension is succesfully loaded
bool glc::loadVboExtension()
{
	QGLBuffer buffer;
	bool result= buffer.create();
	buffer.destroy();
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
#if !defined(Q_OS_MAC) && !defined(Q_OS_LINUX)
	const QGLContext* pContext= QGLContext::currentContext();
	glPointParameterf				= (PFNGLPOINTPARAMETERFARBPROC)pContext->getProcAddress(QLatin1String("glPointParameterf"));
	if (!glPointParameterf) qDebug() << "not glPointParameterf";
	glPointParameterfv				= (PFNGLPOINTPARAMETERFVARBPROC)pContext->getProcAddress(QLatin1String("glPointParameterfv"));
	if (!glPointParameterfv) qDebug() << "not glPointParameterfv";

	result= glPointParameterf && glPointParameterfv;

#endif
    return result;
}

