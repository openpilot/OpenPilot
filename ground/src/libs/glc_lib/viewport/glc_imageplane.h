/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 1.2.0, packaged on September 2009.

 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

//! \file glc_imagePlane.h interface for the GLC_ImagePlane class.

#ifndef GLC_IMAGEPLANE_H_
#define GLC_IMAGEPLANE_H_

#include "../geometry/glc_vbogeom.h"


class GLC_Viewport;

//////////////////////////////////////////////////////////////////////
//! \class GLC_ImagePlane
/*! \brief GLC_ImagePlane : Viewport background image*/

/*! An GLC_ImagePlane is just a plane with a image texture.*/
//////////////////////////////////////////////////////////////////////

class GLC_ImagePlane : public GLC_VboGeom
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct an Image plane linked to a viewport
	GLC_ImagePlane(GLC_Viewport* pViewport);

	//! Remove OpenGL Texture from memmory
	virtual ~GLC_ImagePlane(void);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Return a copy of the geometry
	virtual GLC_VboGeom* clone() const;

	//! Return the geometry bounding box
	virtual GLC_BoundingBox& boundingBox(void);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Load image
	void loadImageFile(const QGLContext *pContext, const QString ImageName);

	//! Update image plane size
	void updatePlaneSize(void);

	//! Update Plane Z position
	void updateZPosition(void);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Virtual interface for OpenGL Geometry set up.
	/*! This Virtual function is implemented here.\n*/
	virtual void glDraw(bool transparent= false);
//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////

private:

	//! ViewPort
	GLC_Viewport* m_pViewport;

	//! Image with
	double m_dLgImage;

	//! Plane Position in Z direction
	double m_dZpos;

	//! Polygons display style
	GLenum m_PolyFace;
	GLenum m_PolyMode;

	//! image plane boundingBox (not yet used)
	GLC_BoundingBox m_BoundingBox;

};

#endif //GLC_IMAGEPLANE_H_
