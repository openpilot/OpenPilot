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
//! \file glc_pullmanipulator.h Interface for the GLC_PullManipulator class.

#ifndef GLC_PULLMANIPULATOR_H_
#define GLC_PULLMANIPULATOR_H_
#include "../glc_config.h"
#include "glc_abstractmanipulator.h"

class GLC_Viewport;

//////////////////////////////////////////////////////////////////////
//! \class GLC_PullManipulator
/*! \brief GLC_PullManipulator :  */

/*! GLC_PullManipulator */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_PullManipulator : public GLC_AbstractManipulator
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct pull manipulator with the given viewport and pulling direction
	GLC_PullManipulator(GLC_Viewport* pViewport, const GLC_Vector3d& pullDirection);

	//! Copy constructor
	GLC_PullManipulator(const GLC_PullManipulator& pullManipulator);

	//! Destructor
	virtual ~GLC_PullManipulator();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the pulling direction of this pull manipulator
	inline GLC_Vector3d pullingDirection() const
	{return m_PullDirection;}

	//! Clone the concrete manipulator
	virtual GLC_AbstractManipulator* clone() const;
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set the pulling direction of this manipulator
	void setPullingDirection(const GLC_Vector3d& pullingDirection);

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
	//! The pulling direction
	GLC_Vector3d m_PullDirection;
};

#endif /* GLC_PULLMANIPULATOR_H_ */
