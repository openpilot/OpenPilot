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

//! \file glc_viewport.h interface for the GLC_Viewport class.

#ifndef GLC_VIEWPORT_H_
#define GLC_VIEWPORT_H_
#include <QGLWidget>
#include <QPair>
#include <QHash>
#include "glc_camera.h"
#include "glc_imageplane.h"
#include "../glc_boundingbox.h"
#include "glc_frustum.h"
#include "../maths/glc_plane.h"
#include "../sceneGraph/glc_3dviewcollection.h"

#include "../glc_config.h"

class GLC_3DViewInstance;

//////////////////////////////////////////////////////////////////////
//! \class GLC_Viewport
/*! \brief GLC_Viewport : OpenGL Viewport */

/*! An GLC_Viewport define Viewport with these specification
 * 		- Default GLC_Camera
 * 		- Max distance of view
 * 		- Min distance of view
 * 		- Angle of view
 * 		- Maximum zoom factor
 */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Viewport
{

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	/*! Construct Viewport with these specifications :
	 * 		- Default GLC_Camera
	 * 		- Max distance of view	: <b>500</b>
	 * 		- Min distance of view	: <b>0.01</b>
	 * 		- Angle of view			: <b>35</b>
	 * 		- Maximum zoom factor	: <b>3.0</b>
	 * */
	GLC_Viewport(QGLWidget *GLWidget);

	//! Delete Camera, Image Plane and orbit circle
	virtual ~GLC_Viewport();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the camera associate to this viewport
	inline GLC_Camera* cameraHandle() const
	{return m_pViewCam;}

	//! Get this viewport Horizontal size
	inline int viewHSize() const
	{ return m_WindowHSize;}

	//! Get this viewport Vertical size
	inline int viewVSize() const
	{ return m_WindowVSize;}

	//! Get this viewport ratio
	inline double aspectRatio() const
	{ return static_cast<double>(m_WindowHSize) / static_cast<double>(m_WindowVSize);}

	//! Return the normalyse mouse position from screen coordinate
	GLC_Point2d normalyseMousePosition(int x, int y);

	//! Map screen position to OpenGL screen position
	GLC_Point2d mapToOpenGLScreen(int x, int y);

	//! Map normalyze screen position to OpenGL screen position
	GLC_Point2d mapNormalyzeToOpenGLScreen(double x, double y);

	//! Map Screen position to OpenGL position (On image Plane) according to this viewport
	GLC_Vector3d mapPosMouse( GLdouble Posx, GLdouble Posy) const;

	//! Map normalyse Screen position to OpenGL position (On image Plane) according to this viewport
	GLC_Vector3d mapNormalyzePosMouse(double Posx, double Posy) const;

	//! Get this viewport's camera's angle of view
	inline double viewAngle() const
	{ return m_ViewAngle;}

	//! Get this viewport's camera's tangent value of view
	inline double viewTangent() const
	{ return m_ViewTangent;}


	//! Get this viewport near clipping plane distance
	inline double nearClippingPlaneDist(void) const
	{ return m_dDistanceMini;}

	//! Get this viewport far clipping plane distance
	inline double farClippingPlaneDist(void) const
	{ return m_DistanceMax;}

	//! Get this viewportbackground Color
	inline QColor backgroundColor(void) const
	{ return m_BackgroundColor;}

	//! Return the selection square size of this viewport
	inline GLsizei selectionSquareSize() const
	{return m_SelectionSquareSize;}

	//! Return this viewport's the projection matrix
	inline GLC_Matrix4x4 projectionMatrix() const
	{return m_ProjectionMatrix;}

	//! Return the composition matrix between projection matrix and view matrix
	inline GLC_Matrix4x4 compositionMatrix() const;

	//! Return an handle to the widget 3D collection
	inline GLC_3DViewCollection* widget3dCollectionHandle()
	{return &m_3DWidgetCollection;}

	//! Return true if this viewport use orthographic projection
	inline bool useOrtho()const
	{return m_UseParallelProjection;}

	//! Return the minimum pixel culling size
	inline int minimumPixelCullingSize() const
	{return m_MinimumStaticPixelSize;}

	//! Return the minimum pixel culling ratio
	inline double minimumStaticPixelCullingRatio() const
	{return m_MinimumStaticRatioSize;}

	//! Return the minimum dynamic pixel culling ratio
	inline double minimumDynamicPixelCullingRatio() const
	{return m_MinimumDynamicRatioSize;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Initialize OpenGL with default values
	/*! Glew initialisation is made here */
	void initGl();

	//! Load camera's transformation Matrix and display image if necessary
	void glExecuteCam(void);

	//! Update this viewport OpenGL projection matrix
	void updateProjectionMat(void);

	//! Force the aspect ratio of this viewport
	void forceAspectRatio(double);

	//! Update the aspect ratio of this viewport
	void updateAspectRatio();

	//! Return the frustum associated to this viewport
	const GLC_Frustum& frustum() const
	{return m_Frustum;}

	//! Return the frustum associated to a selection coordinate
	GLC_Frustum selectionFrustum(int, int) const;

	//! Return the world 3d point from the given screen coordinate
	GLC_Point3d unProject(int, int) const;

	//! Return the list af world 3d point form the givne list af screen coordinates
	/*! The size of the given list must be a multiple of 2*/
	QList<GLC_Point3d> unproject(const QList<int>&)const;

	//! Return an handle of the QGLWidget of this viewport
	inline QGLWidget* qGLWidgetHandle()
	{return m_pQGLWidget;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:

	//! Render this viewport's image plane
	void renderImagePlane();

public:

	//! Render viewport 3D widget
	void render3DWidget();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Inform the viewport that the OpenGL window size has been modified
	void setWinGLSize(int HSize, int VSize);

	//! Call the attached QGLWidgetSelect  updateGL function and return the picking id
	/*! Return UID of the nearest picked object */
	GLC_uint renderAndSelect(int x, int y);

	//! Return the picking id from the already render window
	GLC_uint selectOnPreviousRender(int x, int y);

	//! Select a body inside a 3DViewInstance and return its UID
	/*! Return UID of the nearest picked body */
	GLC_uint selectBody(GLC_3DViewInstance*, int x, int y);

	//! Select a primitive inside a 3DViewInstance and return its UID and its body index
	/*! Return UID of the nearest picked primitive */
	QPair<int, GLC_uint> selectPrimitive(GLC_3DViewInstance*, int x, int y);

	//! Select objects inside specified square and return its UID in a set
	QSet<GLC_uint> selectInsideSquare(int x1, int y1, int x2, int y2);

	//! load background image from file in this viewport
	void loadBackGroundImage(const QString& imageFile);

	//! load background image in this viewport
	void loadBackGroundImage(const QImage& image);

	//! delete background image of this viewport
	void deleteBackGroundImage();

	//! Set Camera's angle of view of this viewport
	inline void setViewAngle(double TargetFov)
	{
		m_ViewAngle= TargetFov;
		m_ViewTangent= tan(glc::toRadian(m_ViewAngle));
		updateProjectionMat();
	}

	//! Set near clipping distance of this viewport
	bool setDistMin(double DistMin);

	//! Set far clipping distance of this viewport
	bool setDistMax(double DistMax);

	//! Set Near and Far clipping distance of this viewport
	/*! box shouldn't be empty*/
	void setDistMinAndMax(const GLC_BoundingBox& bBox);

	//! Set the Background color of this viewport
	void setBackgroundColor(QColor setColor);

	//! Set the selection square size of this viewport
	inline void setSelectionSquareSize(GLsizei size)
	{m_SelectionSquareSize= size;}

	//! Update this viewport frustum (frustum cullin purpose)
	/*! Return true if the frustum has changed*/
	inline bool updateFrustum(GLC_Matrix4x4* pMat= NULL);

	//! Add a clipping plane to this viewport
	void addClipPlane(GLenum planeGlEnum, GLC_Plane* pPlane);

	//! Remove the clip plane coresponding to the given id
	void removeClipPlane(GLenum planeGlEnum);

	//! Remove all clip plane
	void removeAllClipPlane();

	//! Set the clipping plane usage
	void useClipPlane(bool flag);

	//! Add 3DWidget to this viewport
	inline void add3DWidget(GLC_3DViewInstance& widget)
	{m_3DWidgetCollection.add(widget);}

	//! Clear the background color with the specified color
	inline void clearBackground(const QColor& c) const
	{m_pQGLWidget->qglClearColor(c);}

	//! Set othographic usage to the given flag
	void setToOrtho(bool useOrtho);

	//! Set minimum pixel culling size
	inline void setMinimumPixelCullingSize(int size)
	{
		m_MinimumStaticPixelSize= size;
		updateMinimumRatioSize();
	}
//@}


/////////////////////////////////////////////////////////////////////
//! @name zoom Functions
//{@
	//! Set the viewport's camera in order to reframe on the current scene
	/*! box shouldn't be empty*/
	void reframe(const GLC_BoundingBox& box);

//@} End Zooming functions
/////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// private services functions
//////////////////////////////////////////////////////////////////////
private:
	//! Return the meaningful color ID inside a square in screen coordinates
	GLC_uint meaningfulIdInsideSquare(GLint x, GLint y, GLsizei width, GLsizei height);

	//! Return the Set of ID inside a square in screen coordinate
	QSet<GLC_uint> listOfIdInsideSquare(GLint x, GLint y, GLsizei width, GLsizei height);

	//! Update minimum ratio size for pixel culling
	void updateMinimumRatioSize();


//////////////////////////////////////////////////////////////////////
// Private Members
//////////////////////////////////////////////////////////////////////
private:

	//! Viewport's camera
	GLC_Camera *m_pViewCam;

	double m_DistanceMax;		//!< Camera Maximum distance (far clipping plane)
	double m_dDistanceMini;		//!< Camera Minimum distance (near clipping plane)
	double m_ViewAngle;		//!< Camera angle of view
	double m_ViewTangent;		//!< Camera angle tangent


	//! Image plane (Background image)
	GLC_ImagePlane* m_pImagePlane;

	// OpenGL View Definition
	int m_WindowHSize;			//!< Horizontal OpenGL viewport size
	int m_WindowVSize;			//!< Vertical OpenGL viewport size

	//! View AspectRatio
	double m_AspectRatio;

	//! The QGLWidget attached to the viewport (rendering context)
	QGLWidget* m_pQGLWidget;

	//! Viewport Background color
	QColor m_BackgroundColor;

	//! The selection square size
	GLsizei m_SelectionSquareSize;

	//! The projection matrix
	GLC_Matrix4x4 m_ProjectionMatrix;

	//! The frustum associated to the viewport
	GLC_Frustum m_Frustum;

	//! The list of additionnal clipping plane
	QHash<GLenum, GLC_Plane*> m_ClipPlanesHash;

	//! Flag to know if clipping plane must be used
	bool m_UseClipPlane;

	//! The collection wich contains 3D widget
	GLC_3DViewCollection m_3DWidgetCollection;

	//! Flag to know if the viewport use orthographic projection
	bool m_UseParallelProjection;

	//! The minimum static pixel culling size
	int m_MinimumStaticPixelSize;

	//! The minimum static size ratio
	double m_MinimumStaticRatioSize;

	//! The minimum dynamic size ratio
	double m_MinimumDynamicRatioSize;
};

GLC_Matrix4x4 GLC_Viewport::compositionMatrix() const
{
	// Get the viewport projection matrix
	GLC_Matrix4x4 projectionMatrix= m_ProjectionMatrix;
	// Get the camera modelView matrix
	GLC_Matrix4x4 modelViewMatrix= m_pViewCam->modelViewMatrix();
	// Composition matrix
	GLC_Matrix4x4 compMatrix= projectionMatrix * modelViewMatrix;

	return compMatrix;
}

bool GLC_Viewport::updateFrustum(GLC_Matrix4x4* pMat)
{
	if (NULL == pMat)
	{
		return m_Frustum.update(compositionMatrix());
	}
	else
	{
		return m_Frustum.update(*pMat);
	}
}
#endif //GLC_VIEWPORT_H_
