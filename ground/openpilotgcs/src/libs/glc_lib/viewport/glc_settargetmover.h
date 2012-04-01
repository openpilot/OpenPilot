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

#ifndef GLC_SETTARGETMOVER_H_
#define GLC_SETTARGETMOVER_H_

#include "glc_mover.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_SetTargetMover
/*! \brief GLC_SetTargetMover : Define the view target */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_SetTargetMover : public GLC_Mover
{
public:
	//! Default constructor
	GLC_SetTargetMover(GLC_Viewport*, const QList<GLC_RepMover*>& repsList= QList<GLC_RepMover*>());

	//! Copy constructor
	GLC_SetTargetMover(const GLC_SetTargetMover&);

	//! Destructor
	virtual ~GLC_SetTargetMover();

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return a clone of the mover
	virtual GLC_Mover* clone() const;
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Initialized the mover
	virtual void init(const GLC_UserInput& userInput);

	//! Move the camera
	virtual bool move(const GLC_UserInput&){return true;}
//@}

};

#endif /* GLC_SETTARGETMOVER_H_ */
