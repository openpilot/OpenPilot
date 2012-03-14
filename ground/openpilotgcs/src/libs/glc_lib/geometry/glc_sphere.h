/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2010 Laurent Bauer
 Copyright (C) 2010 Laurent Ribon (laumaya@users.sourceforge.net)
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
//! \file glc_sphere.h interface for the GLC_Sphere class.

#ifndef GLC_SPHERE_H_
#define GLC_SPHERE_H_

#include "glc_mesh.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Sphere
/*! \brief GLC_Sphere : OpenGL 3D Sphere*/

/*! An GLC_Sphere is a polygonnal geometry */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Sphere : public GLC_Mesh
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct a sphere with the given radius
	GLC_Sphere(double radius);

	//! Copy constructor
	GLC_Sphere(const GLC_Sphere & sphere);

	//! Destructor
	virtual ~GLC_Sphere();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the class Chunk ID
	static quint32 chunckID();

	//! Return the Radius of this sphere
	inline double radius(void) const
	{return m_Radius;}

	//! Get Sphere discretion
	inline int discretion(void) const
	{return m_Discret;}

	//! Return a copy of the Sphere
	virtual GLC_Geometry* clone() const;

	//! Return the sphere bounding box
	virtual const GLC_BoundingBox& boundingBox(void);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set Sphere radius
	/*! Radius must be > 0*/
	void setRadius(double Radius);

	//! Set Discretion
	/*! Discretion must be > 0*/
	void setDiscretion(int TargetDiscret);

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
	//! Create the sphere mesh
	void createMesh();

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Sphere radius
	double m_Radius;

	//! Sphere polygon discretisation
	int m_Discret;

	double m_ThetaMin;
	double m_ThetaMax;
	double m_PhiMin;
	double m_PhiMax;

	//! Class chunk id
	static quint32 m_ChunkId;

};

#endif /* GLC_SPHERE_H_ */
