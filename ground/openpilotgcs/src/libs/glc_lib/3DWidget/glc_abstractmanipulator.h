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

#ifndef GLC_ABSTRACTMANIPULATOR_H_
#define GLC_ABSTRACTMANIPULATOR_H_

#include "../maths/glc_vector3d.h"
#include "../maths/glc_matrix4x4.h"
#include "../maths/glc_plane.h"

#include "../glc_config.h"

class GLC_Viewport;

//////////////////////////////////////////////////////////////////////
//! \class GLC_AbstractManipulator
/*! \brief GLC_AbstractManipulator :  Base class for all manipulator*/

/*! GLC_AbstractManipulator */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_AbstractManipulator
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct an abstract manipulator with the given viewport
	GLC_AbstractManipulator(GLC_Viewport* pViewport);

	//! Copy constructor
	GLC_AbstractManipulator(const GLC_AbstractManipulator& abstractManipulator);

	//! Destructor
	virtual ~GLC_AbstractManipulator();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return true if this manipulator is in manipulate state
	inline bool isInManipulateState() const
	{return m_IsInManipulateState;}

	//! Return a const reference on the previous position
	const GLC_Point3d& previousPosition() const
	{return m_PreviousPosition;}

	//! Return the viewport of this manipulator
	inline GLC_Viewport* viewportHandle() const
	{return m_pViewport;}

	//! Clone the concrete manipulator
	virtual GLC_AbstractManipulator* clone() const= 0;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Put this manipulator in manipulate state
	void enterManipulateState(const GLC_Point3d& startPoint);

	//! Manipulate this manipulator and return the moving matrix
	GLC_Matrix4x4 manipulate(const GLC_Point3d& newPoint);

	//! Exit this manipulator of manipulate state
	inline void exitManipulateState()
	{m_IsInManipulateState= false;}

	//! Set the viewport of this manipulator
	inline void setViewport(GLC_Viewport* pViewport)
	{m_pViewport= pViewport;}

//@}

//////////////////////////////////////////////////////////////////////
// Protected services function
//////////////////////////////////////////////////////////////////////
protected:
	//! Manipulate the concret manipulator and return the moving matrix
	virtual GLC_Matrix4x4 doManipulate(const GLC_Point3d& newPoint, const GLC_Vector3d& projectionDirection)= 0;

//////////////////////////////////////////////////////////////////////
// protected Member
//////////////////////////////////////////////////////////////////////
protected:
	//! The viewport associated with this manipulator
	GLC_Viewport* m_pViewport;

	//! The currentSlidding plane
	GLC_Plane m_SliddingPlane;

	//! The previous position
	GLC_Point3d m_PreviousPosition;

	//! Flag to know if this manipulator is in manipulate state
	bool m_IsInManipulateState;

};

#endif /* GLC_ABSTRACTMANIPULATOR_H_ */
