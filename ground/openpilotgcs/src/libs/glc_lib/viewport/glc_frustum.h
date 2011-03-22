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
//! \file glc_frustum.h Interface for the GLC_Frustum class.

#ifndef GLC_FRUSTUM_H_
#define GLC_FRUSTUM_H_

#include "../maths/glc_plane.h"
#include "../glc_boundingbox.h"
#include "../glc_config.h"

class GLC_LIB_EXPORT GLC_Viewport;

//////////////////////////////////////////////////////////////////////
//! \class GLC_Frustum
/*! \brief GLC_Frustum : OpenGL Frustum */

/*! GLC_Frustum by 6 planes */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Frustum
{
private:
	enum planeId
	{
		LeftPlane= 0,
		RightPlane= 1,
		TopPlane= 2,
		BottomPlane= 3,
		NearPlane= 4,
		FarPlane= 5
	};
public:
	enum Localisation
	{
		InFrustum = 0,
		IntersectFrustum = 1,
		OutFrustum= 3
	};
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_Frustum();

	//! Copy constructor
	GLC_Frustum(const GLC_Frustum&);

	//! Destructor
	~GLC_Frustum();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Return the left clipping plane
	inline GLC_Plane leftClippingPlane() const
	{return m_PlaneList.at(LeftPlane);}

	//! Return the Right clipping plane
	inline GLC_Plane rightClippingPlane() const
	{return m_PlaneList.at(RightPlane);}

	//! Return the top clipping plane
	inline GLC_Plane topClippingPlane() const
	{return m_PlaneList.at(TopPlane);}

	//! Return the bottom clipping plane
	inline GLC_Plane bottomClippingPlane() const
	{return m_PlaneList.at(BottomPlane);}

	//! Return the near clipping plane
	inline GLC_Plane nearClippingPlane() const
	{return m_PlaneList.at(NearPlane);}

	//! Return the far clipping plane
	inline GLC_Plane farClippingPlane() const
	{return m_PlaneList.at(FarPlane);}

	//! Localize bounding box
	Localisation localizeBoundingBox(const GLC_BoundingBox&) const;

	//! Localize sphere
	Localisation localizeSphere(const GLC_Point3d&, double) const;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Set the left clipping plane
	inline void setLeftClippingPlane(const GLC_Plane& plane)
	{m_PlaneList[LeftPlane]= plane;}

	//! Set the right clipping plane
	inline void setRightClippingPlane(const GLC_Plane& plane)
	{m_PlaneList[RightPlane]= plane;}

	//! Set the top clipping plane
	inline void setTopClippingPlane(const GLC_Plane& plane)
	{m_PlaneList[TopPlane]= plane;}

	//! Set the bottom clipping plane
	inline void setBottomClippingPlane(const GLC_Plane& plane)
	{m_PlaneList[BottomPlane]= plane;}

	//! Set the near clipping plane
	inline void setNearClippingPlane(const GLC_Plane& plane)
	{m_PlaneList[NearPlane]= plane;}

	//! Set the far clipping plane
	inline void setFarClippingPlane(const GLC_Plane& plane)
	{m_PlaneList[FarPlane]= plane;}

	//! Update the frustum
	/*! Return true if the frustum as change*/
	bool update(const GLC_Matrix4x4&);

//@}
//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////
private:
	//! localize a sphere to a plane
	Localisation localizeSphereToPlane(const GLC_Point3d&, double, const GLC_Plane&) const;

//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////
private:

	//! The list of frustum plane
	QList<GLC_Plane> m_PlaneList;

	//! The previous frustum matrix
	GLC_Matrix4x4 m_PreviousMatrix;
};

#endif /* GLC_FRUSTUM_H_ */
