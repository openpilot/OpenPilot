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

//! \file glc_worldreaderplugin.h interface for reading world from 3D model

#ifndef GLC_WORLDREADERHANDLER_H_
#define GLC_WORLDREADERHANDLER_H_

class QFile;
#include <QStringList>

#include "../sceneGraph/glc_world.h"

class GLC_WorldReaderHandler
{
public:
	virtual ~GLC_WorldReaderHandler(){}

	//! Read a world from the given file
	virtual GLC_World read(QFile* pFile)= 0;

	//! Get the list of attached files
	virtual QStringList listOfAttachedFileName() const{return QStringList();}

};

#endif /* GLC_WORLDREADERHANDLER_H_ */
