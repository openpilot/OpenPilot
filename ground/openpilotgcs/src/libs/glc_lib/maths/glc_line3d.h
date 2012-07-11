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
//! \file glc_line3d.h Interface for the GLC_Line3d class.

#ifndef GLC_LINE3D_H_
#define GLC_LINE3D_H_

#include "glc_vector3d.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Line3d
/*! \brief GLC_Line3d : Math 3d line representation */

/*! GLC_Line3d is definined by a 3d point and a vector*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Line3d
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_Line3d();

	//! Construct a 3d line with the given 3d point and vector
	GLC_Line3d(const GLC_Point3d& point, const GLC_Vector3d& vector);

	//! Construct a 3d line with the given 3d line
	GLC_Line3d(const GLC_Line3d& line);

	//! Destructor
	~GLC_Line3d();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the starting 3d point of this line
	inline GLC_Point3d startingPoint() const
	{return m_Point;}

	//! Return the direction vector of this line
	inline GLC_Vector3d direction() const
	{return m_Vector;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set the starting point of this  3d line
	inline void setStartingPoint(const GLC_Point3d& point)
	{m_Point= point;}

	//! Set the direction vector of this line
	inline void setDirection(const GLC_Vector3d& direction)
	{m_Vector= direction;}
//@}

//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////
private:
	//! Starting point of the 3d line
	GLC_Point3d m_Point;

	//! Vector of the line
	GLC_Vector3d m_Vector;

};

#endif /* GLC_LINE3D_H_ */
