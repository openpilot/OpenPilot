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
//! \file glc_disc.h interface for the GLC_Disc class.

#ifndef GLC_DISC_H_
#define GLC_DISC_H_

#include "glc_mesh.h"
#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Disc
/*! \brief GLC_Disc : OpenGL 3D Disc*/

/*! An GLC_Disc is a polygonnal disc */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Disc : public GLC_Mesh
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct a disc with the given radius
	GLC_Disc(double radius, double angle= 2.0 * glc::PI);

	//! Copy constructor
	GLC_Disc(const GLC_Disc& disc);

	//! Destructor
	virtual ~GLC_Disc();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return this disc bounding box
	virtual const GLC_BoundingBox& boundingBox(void);

	//! Return a copy of this Disc
	virtual GLC_Geometry* clone() const;

	//! Return the radius of this disc
	inline double radius() const
	{return m_Radius;}

	//! Return the discretion of this disc
	inline int discretion() const
	{return m_Discret;}

	//! Return the angle of this disc
	inline double angle() const
	{return m_Angle;}


//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set this disc from the given disc and return a reference of this disc
	GLC_Disc& operator=(const GLC_Disc& disc);

	//! Set this disc radius to the given radius
	void setRadius(double radius);

	//! Set this disc discretion to the given discretion
	void setDiscretion(int targetDiscret);

	//! Set this disc angle in radians
	void setAngle(double angle);

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
	//! Create the cylinder mesh and wire
	void createMeshAndWire();

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Disc radius
	double m_Radius;

	//! Disc polygon discretisation
	int m_Discret;

	//! Angle of disc in radians
	double m_Angle;

	//! Disc Step
	GLuint m_Step;

};

#endif /* GLC_DISC_H_ */
