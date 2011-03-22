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
//! \file glc_repflymover.h Interface for the GLC_RepFlyMover class.

#ifndef GLC_REPFLYMOVER_H_
#define GLC_REPFLYMOVER_H_

#include "glc_repmover.h"
#include "../geometry/glc_circle.h"
#include "../sceneGraph/glc_3dviewinstance.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_RepFlyMover
/*! \brief GLC_RepFlyMover : Fly representation*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_RepFlyMover : public GLC_RepMover
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_RepFlyMover(GLC_Viewport* pViewport);

	//! Copy constructor
	GLC_RepFlyMover(const GLC_RepFlyMover& repFlyMover);

	//! Destructor
	virtual ~GLC_RepFlyMover();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
	//! Return a clone of the flymover
	virtual GLC_RepMover* clone() const;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
	//! Update the representation
	virtual void update();

	//! Set representation main color
	virtual void setMainColor(const QColor& color);

	//! Set representation wire thickness
	virtual void setThickness(double thickness);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Virtual interface for OpenGL Geometry set up.
	virtual void glDraw();

//@}


/////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
private:
	//! Create the plane representation
	void createRepresentation();

//////////////////////////////////////////////////////////////////////
// Private Members
//////////////////////////////////////////////////////////////////////
private:
	//! Center Circle radius
	double m_Radius;

	//! Center Circle
	GLC_3DViewInstance m_CenterCircle;

	//! Plane
	GLC_3DViewInstance m_Plane;

	//! HUD
	GLC_3DViewInstance m_Hud;

	//! HUD offset
	GLC_Vector2d m_HudOffset;
};

#endif /* GLC_REPFLYMOVER_H_ */
