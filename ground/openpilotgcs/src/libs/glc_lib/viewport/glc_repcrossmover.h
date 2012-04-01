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

#ifndef GLC_REPCROSSMOVER_H_
#define GLC_REPCROSSMOVER_H_

#include "glc_repmover.h"

#include "../sceneGraph/glc_3dviewinstance.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_RepCrossMover
/*! \brief GLC_RepCrossMover : Cross representation*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_RepCrossMover : public GLC_RepMover
{
public:
	//! Default constructor
	GLC_RepCrossMover(GLC_Viewport*);

	//! Copy constructor
	GLC_RepCrossMover(const GLC_RepCrossMover&);

	//! Destructor
	virtual ~GLC_RepCrossMover();

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
	//! Return a clone of the repmover
	virtual GLC_RepMover* clone() const;
//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Virtual interface for OpenGL Geometry set up.
	virtual void glDraw();

//@}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
private:
	//! Create and return the cross instance
	GLC_3DViewInstance createCrossInstance();

};

#endif /* GLC_REPCROSSMOVER_H_ */
