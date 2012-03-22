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
//! \file glc_ext.h Header of the GLC Opengl extension functions.

#ifndef GLC_EXT_H_
#define GLC_EXT_H_

#include <QtOpenGL>
#include "3rdparty/glext/glext.h"

// Buffer offset used by VBO
#define BUFFER_OFFSET(i) ((char*)NULL + (i))

#if !defined(Q_OS_MAC)

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
