/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2011 JŽr™me Forrissier
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

//! \file glc_fileloader.h common interface to 3D file parsers.

#ifndef GLC_FILELOADER_H_
#define GLC_FILELOADER_H_

#include <QString>
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QColor>
#include <QList>

#include "../glc_config.h"

class GLC_World;
class QGLContext;

//////////////////////////////////////////////////////////////////////
//! \class GLC_FileLoader
/*! \brief GLC_FileLoader : Create a GLC_World from file */

/*! GLC_FileLoader loads a 3D model from a file.
 * 	A suitable parser is selected based on the file name extension.
 */
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_FileLoader : public QObject
{
	Q_OBJECT
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

public:
	GLC_FileLoader();
	virtual ~GLC_FileLoader();
//@}
//////////////////////////////////////////////////////////////////////
/*! @name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Create a GLC_World from a file
	GLC_World createWorldFromFile(QFile &file, QStringList* pAttachedFileName= NULL);
//@}


//////////////////////////////////////////////////////////////////////
// Qt Signals
//////////////////////////////////////////////////////////////////////
	signals:
	void currentQuantum(int);

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
};

#endif /*GLC_FILELOADER_H_*/
