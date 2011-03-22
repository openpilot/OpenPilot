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
//! \file glc_bsreptoworld.h interface for the GLC_BSRepToWorld class.

#ifndef GLC_BSREPTOWORLD_H_
#define GLC_BSREPTOWORLD_H_

#include <QFile>
#include "../glc_config.h"

class GLC_World;

//////////////////////////////////////////////////////////////////////
//! \class GLC_BSRepToWorld
/*! \brief GLC_BSRepToWorld : Create an GLC_World from BSRep file */

/*! An GLC_BSRepToWorld extract the only mesh from an .BSRep file*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_BSRepToWorld
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	GLC_BSRepToWorld();
	virtual ~GLC_BSRepToWorld();
//@}

//////////////////////////////////////////////////////////////////////
/*! @name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Create and return an GLC_World* from an input BSRep File
	GLC_World* CreateWorldFromBSRep(QFile &file);
//@}

};

#endif /* GLC_BSREPTOWORLD_H_ */
