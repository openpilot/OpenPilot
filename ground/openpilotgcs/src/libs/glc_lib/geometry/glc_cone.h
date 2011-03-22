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
//! \file glc_cone.h interface for the GLC_Cone class.

#ifndef GLC_CONE_H_
#define GLC_CONE_H_

#include "glc_mesh.h"
#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Cone
/*! \brief GLC_Cone : OpenGL 3D Cone*/

/*! An GLC_Cone is a polygonnal geometry */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Cone : public GLC_Mesh
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct an GLC_Cone
	/*! By default, discretion is set to #GLC_POLYDISCRET \n
	 *  By default, Axis of Cylinder is Z Axis
	 *  dRadius must be > 0
	 *  dLength must be > 0*/
	GLC_Cone(double dRadius, double dLength);

	//! Copy contructor
	GLC_Cone(const GLC_Cone& sourceCone);

	//! Destructor
	virtual ~GLC_Cone();
//@}


//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the class Chunk ID
	static quint32 chunckID();

	//! Get Lenght of the Cone
	inline double length(void) const
	{return m_Length;}

	//! Get Radius of cone
	inline double radius(void) const
	{return m_Radius;}

	//! Get Cone discretion
	inline int discretion(void) const
	{return m_Discret;}

	//! Return a copy of the Cone
	virtual GLC_Geometry* clone() const;

	//! Return the cone bounding box
	virtual const GLC_BoundingBox& boundingBox(void);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set Cone length
	/*! Length must be > 0*/
	void setLength(double Length);

	//! Set Cone radius
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
	//! Create the cylinder mesh and wire
	void createMeshAndWire();

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:

	//! Cone's radius
	double m_Radius;

	//! Cone length (Z Axis direction)
	double m_Length;

	//! Cone polygon discretisation
	int m_Discret;

	//! Class chunk id
	static quint32 m_ChunkId;

};

#endif /* GLC_CONE_H_ */
