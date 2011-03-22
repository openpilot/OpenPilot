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
//! \file glc_rotationmanipulator.h Interface for the GLC_RotationManipulator class.

#ifndef GLC_ROTATIONMANIPULATOR_H_
#define GLC_ROTATIONMANIPULATOR_H_

#include "../glc_config.h"
#include "glc_abstractmanipulator.h"
#include "../maths/glc_line3d.h"

class GLC_Viewport;

//////////////////////////////////////////////////////////////////////
//! \class GLC_RotationManipulator
/*! \brief GLC_RotationManipulator :  */

/*! GLC_RotationManipulator */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_RotationManipulator : public GLC_AbstractManipulator
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct a rotation manipulator with the given viewport and rotation line
	GLC_RotationManipulator(GLC_Viewport* pViewport, const GLC_Line3d& rotationLine);

	//! Copy constructor
	GLC_RotationManipulator(const GLC_RotationManipulator& rotationmanipulator);

	//! Destructor
	virtual ~GLC_RotationManipulator();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Clone the concrete manipulator
	virtual GLC_AbstractManipulator* clone() const;

	//! Return the rotation line of this rotation manipulator
	inline GLC_Line3d rotationLine() const
	{return m_RotationLine;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set the rotation line of this rotation manipulator
	void setRotationLine(const GLC_Line3d line)
	{m_RotationLine= line;}

//@}

//////////////////////////////////////////////////////////////////////
// Protected services function
//////////////////////////////////////////////////////////////////////
protected:
	//! Manipulate this manipulator and return the moving matrix
	virtual GLC_Matrix4x4 doManipulate(const GLC_Point3d& newPoint, const GLC_Vector3d& projectionDirection);

//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////
private:
	//! The rotation line of this manipulator
	GLC_Line3d m_RotationLine;

};

#endif /* GLC_ROTATIONMANIPULATOR_H_ */
