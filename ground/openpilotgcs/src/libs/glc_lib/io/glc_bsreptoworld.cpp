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
//! \file glc_bsreptoworld.cpp implementation of the GLC_BSRepToWorld class.

#include "glc_bsreptoworld.h"
#include "../sceneGraph/glc_world.h"
#include "../glc_fileformatexception.h"

GLC_BSRepToWorld::GLC_BSRepToWorld()
{

}

GLC_BSRepToWorld::~GLC_BSRepToWorld()
{

}

GLC_World* GLC_BSRepToWorld::CreateWorldFromBSRep(QFile &file)
{
	//////////////////////////////////////////////////////////////////
	// Test if the file exist and can be opened
	//////////////////////////////////////////////////////////////////
	if (!file.open(QIODevice::ReadOnly))
	{
		QString fileName(file.fileName());
		QString message(QString("GLC_BSRepToWorld::CreateWorldFromBSRep File ") + fileName + QString(" doesn't exist"));
		GLC_FileFormatException fileFormatException(message, fileName, GLC_FileFormatException::FileNotFound);
		throw(fileFormatException);
	}
	file.close();
	GLC_BSRep bsRep(file.fileName());
	GLC_3DRep rep(bsRep.loadRep());
	GLC_World* pWorld= new GLC_World();
	pWorld->rootOccurence()->addChild(new GLC_StructOccurence(new GLC_3DRep(rep)));
	return pWorld;
}
