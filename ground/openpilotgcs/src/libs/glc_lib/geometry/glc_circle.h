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

//! \file glc_circle.h interface for the GLC_Circle class.

#ifndef GLC_CIRCLE_H_
#define GLC_CIRCLE_H_

#include "glc_geometry.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Circle
/*! \brief GLC_Circle : OpenGL 3D Circle*/

/*! An GLC_Circle is a wire geometry composed of 3d lines \n
 * It can be an entire circle or an arc.
 * */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Circle : public GLC_Geometry
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Construct an GLC_Circle
	/*! By default, discretion is set to #GLC_DISCRET*/
	GLC_Circle(const double &dRadius, double Angle= 2.0 * glc::PI);

	//! Copy constructor
	GLC_Circle(const GLC_Circle& sourceCircle);

	//! Destructor
	virtual ~GLC_Circle();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Return Circle Discretion
	inline int discretion() const
	{ return m_Discret;}

	//! Return Circle radius
	inline double radius() const
	{return m_Radius;}

	//! return Circle diameter
	inline double diameter() const
	{return m_Radius * 2.0;}

	//! return the circle bounding box
	virtual const GLC_BoundingBox& boundingBox();

	//! Return a copy of the geometry
	virtual GLC_Geometry* clone() const;
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set Circle diameter
	/*! Diameter must be > 2 * EPSILON*/
	void setDiameter(double D);

	//! Set Circle Radius
	/*! Radius must be > EPSILON*/
	void setRadius(double R);

	//! Set Circle discret
	/*! TargetDiscret must be > 0
	 *  if TargetDiscret < 6 discretion is set to 6*/
	void setDiscretion(int TargetDiscret);

	//! Set Circle Angle
	/*! AngleRadians must be > EPSILON and < 2 PI*/
	void setAngle(double AngleRadians);	// Angle in Radians

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:

	//! Virtual interface for OpenGL Geometry set up.
	/*! This Virtual function is implemented here.\n
	 *  Throw GLC_OpenGlException*/
	virtual void glDraw(const GLC_RenderProperties&);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Private services Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Create the wire
	void createWire();

//@}

//////////////////////////////////////////////////////////////////////
// private members
//////////////////////////////////////////////////////////////////////
private:
	//! Circle Radius
	double m_Radius;

	//! Circle Discretion
	int m_Discret;

	//! Angle of circle in radians
	double m_Angle;

	//! Circle Step
	GLuint m_Step;

};
#endif //GLC_CIRCLE_H_
