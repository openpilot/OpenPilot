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
//! \file glc_plane.h Interface for the GLC_Plane class.

#ifndef GLC_PLANE_H_
#define GLC_PLANE_H_

#include "glc_vector3d.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Plane
/*! \brief GLC_Plane : Math plane representation */

/*! GLC_Plane is definined by its equation : Ax + By + CZ + D= 0 */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Plane
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_Plane();

	//! Contruct a plan with specified parameter
	/*! Plane equation : Ax + By + CZ + D= 0*/
	GLC_Plane(double A, double B, double C, double D);

	//! Construct a plane with normal vector and the minimum distance from this plane to the origin
	GLC_Plane(const GLC_Vector3d& normal, double minimumDistance);

	//! Construct a plane with normal vector and a 3d point
	GLC_Plane(const GLC_Vector3d& normal, const GLC_Point3d& point);

	//! Contruct a plane with 3 given 3d points
	/*! first : origine, second x, third y*/
	GLC_Plane(const GLC_Point3d&, const GLC_Point3d&, const GLC_Point3d&);

	//! Copy constructor
	GLC_Plane(const GLC_Plane&);

	//! Assignement operator
	GLC_Plane &operator=(const GLC_Plane&);

	//! Destructor
	~GLC_Plane();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Return A coef
	inline double coefA() const
	{return m_Eq[0];}

	//! Return B coef
	inline double coefB() const
	{return m_Eq[1];}

	//! Return C coef
	inline double coefC() const
	{return m_Eq[2];}

	//! Return D coef
	inline double coefD() const
	{return m_Eq[3];}

	//! Return the signed distance to a point
	inline double distanceToPoint(const GLC_Point3d& p) const
	{return m_Eq[0] * p.x() + m_Eq[1] * p.y() + m_Eq[2] * p.z() + m_Eq[3];}

	//! Equality operator
	bool operator==(GLC_Plane) const;

	//! diff operator
	inline bool operator!=(const GLC_Plane& p) const
	{return !operator==(p);}

	//! Return this plane normal
	inline GLC_Vector3d normal() const
	{return GLC_Vector3d(m_Eq[0], m_Eq[1], m_Eq[2]);}

	//! Return true if the given point is on this plane
	inline bool lieOnThisPlane(const GLC_Point3d& p)
	{return (m_Eq[0] * p.x() + m_Eq[1] * p.y() + m_Eq[2] * p.z() + m_Eq[3]) == 0.0f;}

	//! Return a pointer to this plane equation data
	const double* data() const
	{return m_Eq;}

	//! Return the plane data to string
	QString toString() const;
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set A coef
	inline void setA(double a)
	{m_Eq[0]= a;}

	//! Set B coef
	inline void setB(double b)
	{m_Eq[1]= b;}

	//! Set C coef
	inline void setC(double c)
	{m_Eq[2]= c;}

	//! Set D coef
	inline void setD(double d)
	{m_Eq[3]= d;}

	//! Normalize the plane
	void normalize();

	//! Set the plane from the given normal and point and return a reference to this plane
	GLC_Plane& setPlane(const GLC_Vector3d& normal, const GLC_Point3d& point);


//@}

//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////
private:
	//! Plane is define by equation : Ax + By + Cz + D= 0
	double m_Eq[4];
};

#endif /* GLC_PLANE_H_ */
