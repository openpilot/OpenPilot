/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Copyright (C) 2009 Laurent Bauer
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
//! \file glc_pointsprite.h interface for the GLC_PointSprite class.

#include "glc_pointcloud.h"
#include <QVector>

#include "../glc_config.h"

#ifndef GLC_POINTSPRITE_H_
#define GLC_POINTSPRITE_H_

//////////////////////////////////////////////////////////////////////
//! \class GLC_PointSprite
/*! \brief GLC_PointSprite : OpenGL 3D Point Sprite Using extension : GL_ARB_point_parameters*/

/*! An GLC_PointSprite is just a simple 3D Sprite Point*/
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_PointSprite : public GLC_PointCloud
{
public:
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
	//! Default constructor
	/*! The material must exist and had texture*/
	GLC_PointSprite(float, GLC_Material*);

	//! Copy constructor
	GLC_PointSprite(const GLC_PointSprite& point);

	//! Default destructor
	virtual ~GLC_PointSprite();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Return the point size
	inline float size() const
	{return m_Size;}

	//! Return a copy of the geometry
	virtual GLC_Geometry* clone() const;

	//! Return the fade thresold size
	inline float fadeThresoldSize()
	{return m_FadeThresoldSize;}

	//! Return the maximum point size
	/*! Return -1 if the size is unknown*/
	inline static float maximumPointSize()
	{return m_MaxSize;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Return the point size
	void setSize(float size);

	//! Set the point distance attenuation values
	/*! Vector size must be equal to 3*/
	void setPointDistanceAttenuation(QVector<float>);

	//! Set the fade thresold size
	inline void setFadeThresoldSize(float value)
	{m_FadeThresoldSize= value;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Specific glExecute method
	virtual void render(const GLC_RenderProperties&);

	//! Virtual interface for OpenGL Geometry set up.
	/*! This Virtual function is implemented here.\n*/
	virtual void glDraw(const GLC_RenderProperties&);
//@}
//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////

private:
	//! The point size
	float m_Size;

	//! The Distance attenuation values
	QVector<float> m_DistanceAttenuation;

	//! The Fade Thresold Size
	float m_FadeThresoldSize;

	//! The maximum point size
	static float m_MaxSize;
};

#endif /* GLC_POINTSPRITE_H_ */
