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
//! \file glc_arrow.h interface for the GLC_Arrow class.

#ifndef GLC_ARROW_H_
#define GLC_ARROW_H_

#include <QColor>
#include "../maths/glc_vector3d.h"
#include "glc_geometry.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Arrow
/*! \brief GLC_Arrow : OpenGL 3D Arrow*/

/*! An GLC_Arrow is a wire Simple Arrow*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Arrow : public GLC_Geometry
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct an arrow with the given points and view direction
	GLC_Arrow(const GLC_Point3d& startPoint, const GLC_Point3d& endPoint, const GLC_Vector3d& viewDir);

	//! Copy constructor
	GLC_Arrow(const GLC_Arrow& arrow);

	//! Destructor
	virtual ~GLC_Arrow();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the arrow bounding box
	const GLC_BoundingBox& boundingBox();

	//! Return the start point of this arrow
	inline GLC_Point3d startPoint() const
	{return m_StartPoint;}

	//! Return the end point of this arrow
	inline GLC_Point3d endPoint() const
	{return m_EndPoint;}

	//! Return the lenght of the head of this arrow
	inline double headLenght() const
	{return m_HeadLenght;}

	//! Return the angle in radians of the head of this arrow
	inline double headAngle() const
	{return m_HeadAngle;}

	//! Return the viewing direction of this arrow
	inline GLC_Vector3d viewDir() const
	{return m_ViewDir;}

	//! Return a copy of the geometry
	virtual GLC_Geometry* clone() const;

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set this arrow from the given arrow
	GLC_Arrow& operator=(const GLC_Arrow& arrow);

	//! Set the start point of this arrow
	void setStartPoint(const GLC_Point3d& startPoint);

	//! Set the end point of this arrow
	void setEndPoint(const GLC_Point3d& endPoint);

	//! Set the length of the head of this arrow to the given lenght
	void setHeadLength(double headLenght);

	//! Set the angle of the head of this arrow to the given angle in radians
	void setHeadAngle(double headAngle);

	//! Set the view dir of this arrow to the given vector 3d
	void setViewDir(const GLC_Vector3d& viewDir);

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
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Start point
	GLC_Point3d m_StartPoint;

	//! End point
	GLC_Point3d m_EndPoint;

	//! Head lenght
	double m_HeadLenght;

	//! Head angle
	double m_HeadAngle;

	//! The arrow viewing dir
	GLC_Vector3d m_ViewDir;
};

#endif /* GLC_ARROW_H_ */
