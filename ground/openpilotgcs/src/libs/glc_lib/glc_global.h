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

//! \file glc_global.h provide usefull utilities

#ifndef GLC_GLOBAL_H_
#define GLC_GLOBAL_H_

#include <QMutex>
#include <QtOpenGL>
#include <QList>
#include <QVector>
#include <QHash>

#include "glc_config.h"

// GLC_lib typedef
//! Type used for all GLC_lib ID
typedef unsigned int GLC_uint;

//! Types used for Bulk Opengl Data : QVector of GLfloat
typedef QVector<GLfloat> GLfloatVector;

//! Types used for index Opengl Data : QVector of GLuint
typedef QVector<GLuint> GLuintVector;

typedef QList<GLuint> IndexList;
typedef QVector<GLsizei> IndexSizes;
typedef QVector<GLvoid*> OffsetVector;
typedef QVector<GLuint> OffsetVectori;


namespace glc
{
	//! Simple ID generation
	GLC_LIB_EXPORT GLC_uint GLC_GenID();

	//! Simple Geom ID generation
	GLC_LIB_EXPORT GLC_uint GLC_GenGeomID();

	//! Simple User ID generation
	GLC_LIB_EXPORT GLC_uint GLC_GenUserID();

	//! Simple 3D widget ID generation
	GLC_LIB_EXPORT GLC_uint GLC_Gen3DWidgetID();

	//! Simple shading group generation
	GLC_LIB_EXPORT GLC_uint GLC_GenShaderGroupID();

	//! Return the GLC_uint decoded ID from RGB encoded ID
	inline GLC_uint decodeRgbId(const GLubyte*);

	//! Return the encoded color of the id
	inline void encodeRgbId(GLC_uint, GLubyte*);

	const int GLC_DISCRET= 70;
	const int GLC_POLYDISCRET= 60;

	extern QMutex iDMutex;
	extern QMutex geomIdMutex;
	extern QMutex userIdMutex;
	extern QMutex widget3dIdMutex;
	extern QMutex shadingGroupIdMutex;

	//! 3D widget event flag
	enum WidgetEventFlag
	{
		AcceptEvent,
		IgnoreEvent,
		BlockedEvent
	};

	//! Return GLC_lib Archive prefix string
	GLC_LIB_EXPORT const QString archivePrefix();

	//! Return GLC_lib Archive infix string
	GLC_LIB_EXPORT const QString archiveInfix();

	//! Return GLC_lib File prefix string
	GLC_LIB_EXPORT const QString filePrefix();

	//! Return GLC_lib File infix string
	GLC_LIB_EXPORT const QString fileInfix();

	//! Return true if the given file name is in a archive string
	GLC_LIB_EXPORT bool isArchiveString(const QString& fileName);

	//! Return true if the given file name is in a File string
	GLC_LIB_EXPORT bool isFileString(const QString& fileName);

	//! Return archive string form the given archive fileName and fileName entry
	GLC_LIB_EXPORT QString builtArchiveString(const QString& Archive, const QString& entry);

	//! Return archive string form the given archive fileName and fileName entry
	GLC_LIB_EXPORT QString builtFileString(const QString& File, const QString& entry);

	//! Return Archive filename from the given archive string
	GLC_LIB_EXPORT QString archiveFileName(const QString& archiveString);

	//! Return Archive entry filname from the given archive string
	GLC_LIB_EXPORT QString archiveEntryFileName(const QString& archiveString);

	// GLC_Lib version
	const QString version("2.2.0");
	const QString description("GLC_lib is a Open Source C++ class library that enables the quick creation of an OpenGL application based on QT4.");

};

// Return the GLC_uint decoded ID from RGBA encoded ID
GLC_uint glc::decodeRgbId(const GLubyte* pcolorId)
{
	GLC_uint returnId= 0;
	returnId|= (GLC_uint)pcolorId[0] << (0 * 8);
	returnId|= (GLC_uint)pcolorId[1] << (1 * 8);
	returnId|= (GLC_uint)pcolorId[2] << (2 * 8);
	// Only get first 24 bits
	//returnId|= (GLC_uint)pcolorId[3] << (3 * 8);

	return returnId;
}

// Encode Id to RGBA color
void glc::encodeRgbId(GLC_uint id, GLubyte* colorId)
{
	colorId[0]= static_cast<GLubyte>((id >> (0 * 8)) & 0xFF);
	colorId[1]= static_cast<GLubyte>((id >> (1 * 8)) & 0xFF);
	colorId[2]= static_cast<GLubyte>((id >> (2 * 8)) & 0xFF);
	colorId[3]= static_cast<GLubyte>((id >> (3 * 8)) & 0xFF);
}


#endif //GLC_GLOBAL_H_


