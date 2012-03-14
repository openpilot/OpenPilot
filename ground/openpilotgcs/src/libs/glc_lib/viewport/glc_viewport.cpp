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

//! \file glc_viewport.cpp implementation of the GLC_Viewport class.

#include <QtOpenGL>

#include "../glu/glc_glu.h"
#include "glc_viewport.h"
#include "../glc_openglexception.h"
#include "../glc_ext.h"
#include "../shading/glc_selectionmaterial.h"
#include "../glc_state.h"
#include "../sceneGraph/glc_3dviewinstance.h"

#include <QtDebug>

using namespace glc;
//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////

GLC_Viewport::GLC_Viewport(QGLWidget *GLWidget)
// Camera definition
: m_pViewCam(new GLC_Camera())				// Camera
, m_DistanceMax(500.0)			// Camera Maximum distance
, m_dDistanceMini(0.01)			// Camera Minimum distance
, m_ViewAngle(35)					// Camera angle of view
, m_ViewTangent(tan(glc::toRadian(m_ViewAngle)))
, m_pImagePlane(NULL)			// Background image
// OpenGL Window size
, m_WindowHSize(0)				// Horizontal OpenGL viewport size
, m_WindowVSize(0)				// Vertical OpenGL viewport size
, m_AspectRatio(1.0)
, m_pQGLWidget(GLWidget)		// Attached QGLWidget
// the default backgroundColor
, m_BackgroundColor(Qt::black)
, m_SelectionSquareSize(4)
, m_ProjectionMatrix()
, m_Frustum()
, m_ClipPlanesHash()
, m_UseClipPlane(false)
, m_3DWidgetCollection()
, m_UseParallelProjection(false)
, m_MinimumStaticPixelSize(10)
, m_MinimumStaticRatioSize(0.0)
, m_MinimumDynamicRatioSize(0.0)
{
	updateMinimumRatioSize();
}

GLC_Viewport::~GLC_Viewport()
{
	delete m_pViewCam;

	// delete background image
	deleteBackGroundImage();

	// Delete clip planes
	QHash<GLenum, GLC_Plane*>::iterator iClip= m_ClipPlanesHash.begin();
	while (m_ClipPlanesHash.constEnd() != iClip)
	{
		delete iClip.value();
		++iClip;
	}
}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////
GLC_Point2d  GLC_Viewport::normalyseMousePosition(int x, int y)
{
	double nX= 0.0;
	double nY= 0.0;
	if (m_WindowHSize != 0)
	{
		nX= static_cast<double>(x) / static_cast<double>(m_WindowHSize);
	}

	if (m_WindowVSize != 0)
	{
		nY= static_cast<double>(y) / static_cast<double>(m_WindowVSize);
	}

	return GLC_Point2d(nX, nY);
}

GLC_Point2d GLC_Viewport::mapToOpenGLScreen(int x, int y)
{
	GLC_Point2d nPos= normalyseMousePosition(x, y);

	return mapNormalyzeToOpenGLScreen(nPos.getX(), nPos.getY());
}

GLC_Point2d GLC_Viewport::mapNormalyzeToOpenGLScreen(double x, double y)
{
	GLC_Point2d pos(x, y);
	pos= pos * 2.0;
	pos.setY(pos.getY() * -1.0);
	pos= pos + GLC_Point2d(-1.0, 1.0);
	return pos;
}

GLC_Vector3d GLC_Viewport::mapPosMouse( GLdouble Posx, GLdouble Posy) const
{
	// Change the window origin (Up Left -> centred)
	Posx= Posx - static_cast<double>(m_WindowHSize)  / 2.0;
	Posy= static_cast<double>(m_WindowVSize) / 2.0 - Posy;

	GLC_Vector3d VectMouse(Posx, Posy,0);

	// Compute the length of camera's field of view
	const double ChampsVision = m_pViewCam->distEyeTarget() *  m_ViewTangent;

	// the side of camera's square is mapped on Vertical length of window
	// Ratio OpenGL/Pixel = dimend GL / dimens Pixel
	const double Ratio= ChampsVision / static_cast<double>(m_WindowVSize);

	VectMouse= VectMouse * Ratio;

	return VectMouse;
}

GLC_Vector3d GLC_Viewport::mapNormalyzePosMouse(double Posx, double Posy) const
{
	double screenX= Posx * static_cast<double>(m_WindowHSize);
	double screenY= Posy * static_cast<double>(m_WindowVSize);
	return mapPosMouse(screenX, screenY);
}

//////////////////////////////////////////////////////////////////////
// Public OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_Viewport::initGl()
{
	m_pQGLWidget->qglClearColor(m_BackgroundColor);       // Background
	glClearDepth(1.0f);                                   // Depth Buffer Setup
	glShadeModel(GL_SMOOTH);                              // Enable Smooth Shading
	glEnable(GL_DEPTH_TEST);                              // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                               // The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    // Really Nice Perspective Calculation
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonOffset (1.0f, 1.0f);
}

void GLC_Viewport::glExecuteCam(void)
{
	renderImagePlane();
	m_pViewCam->glExecute();
}

void GLC_Viewport::updateProjectionMat(void)
{
	GLC_Context::current()->glcMatrixMode(GL_PROJECTION);						// select The Projection Matrix
	GLC_Context::current()->glcLoadIdentity();									// Reset The Projection Matrix

	if (m_UseParallelProjection)
	{
		const double ChampsVision = m_pViewCam->distEyeTarget() * m_ViewTangent;
		const double height= ChampsVision;
		const double with= ChampsVision * m_AspectRatio;
		const double left= -with * 0.5;
		const double right=  -left;
		const double bottom= - height * 0.5;
		const double top= -bottom;
		GLC_Context::current()->glcOrtho(left, right, bottom, top, m_dDistanceMini, m_DistanceMax);
	}
	else
	{
	    const double yMax= m_dDistanceMini * tan(m_ViewAngle * glc::PI / 360.0);
	    const double yMin= -yMax;
	    const double xMax= yMax * m_AspectRatio;
	    const double xMin= -xMax;
	    GLC_Context::current()->glcFrustum(xMin, xMax, yMin, yMax, m_dDistanceMini, m_DistanceMax);
	}

	// Save the projection matrix
	m_ProjectionMatrix= GLC_Context::current()->projectionMatrix();

	GLC_Context::current()->glcMatrixMode(GL_MODELVIEW);							// select The Modelview Matrix
}

void GLC_Viewport::forceAspectRatio(double ratio)
{
	m_AspectRatio= ratio;
	updateProjectionMat();
}

void GLC_Viewport::updateAspectRatio()
{
	// Update The Aspect Ratio Of The Window
	m_AspectRatio= static_cast<double>(m_WindowHSize)/static_cast<double>(m_WindowVSize);
}
GLC_Frustum GLC_Viewport::selectionFrustum(int x, int y) const
{
	const int halfSize= m_SelectionSquareSize / 2;
	// Calculate the 4 points of the selection
	//p1->p2
	//
	//p0  p3
	QList<int> coordinates;
	// Point 0
	coordinates << (x - halfSize) << (y + halfSize);
	// Point 1
	coordinates << (x - halfSize) << (y - halfSize);
	// Point 2
	coordinates << (x + halfSize) << (y - halfSize);
	// Point 3
	coordinates << (x + halfSize) << (y + halfSize);

	// Unproject the 4 point
	QList<GLC_Point3d> listOfPoint= unproject(coordinates);

	Q_ASSERT(4 == listOfPoint.size());
	// Create the four frustum planes
	GLC_Point3d eye= m_pViewCam->eye();
	const GLC_Plane leftPlane(listOfPoint.at(0), listOfPoint.at(1), eye);
	const GLC_Plane rightPlane(listOfPoint.at(3), eye , listOfPoint.at(2));
	const GLC_Plane upPlane(listOfPoint.at(2), eye, listOfPoint.at(1));
	const GLC_Plane bottomPlane(listOfPoint.at(0), eye, listOfPoint.at(3));

	GLC_Frustum selectionFrustum(m_Frustum);
	selectionFrustum.setLeftClippingPlane(leftPlane);
	selectionFrustum.setRightClippingPlane(rightPlane);
	selectionFrustum.setTopClippingPlane(upPlane);
	selectionFrustum.setBottomClippingPlane(bottomPlane);

	return selectionFrustum;
}

GLC_Point3d GLC_Viewport::unProject(int x, int y) const
{
	// Z Buffer component of the given coordinate is between 0 and 1
	GLfloat Depth;
	// read selected point
	glReadPixels(x, m_WindowVSize - y , 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Depth);

	// The current viewport opengl definition
	GLint Viewport[4];
	glGetIntegerv(GL_VIEWPORT, Viewport);

	// OpenGL ccordinate of selected point
	GLdouble pX, pY, pZ;
	glc::gluUnProject((GLdouble) x, (GLdouble) (m_WindowVSize - y) , Depth
		, m_pViewCam->modelViewMatrix().getData(), m_ProjectionMatrix.getData(), Viewport, &pX, &pY, &pZ);

	return GLC_Point3d(pX, pY, pZ);
}

QList<GLC_Point3d> GLC_Viewport::unproject(const QList<int>& list)const
{
	const int size= list.size();
	Q_ASSERT((size % 2) == 0);

	// The current viewport opengl definition
	GLint Viewport[4];
	glGetIntegerv(GL_VIEWPORT, Viewport);

	// Z Buffer component of the given coordinate is between 0 and 1
	GLfloat Depth;

	// Coordinate of readed points
	GLdouble pX, pY, pZ;
	QList<GLC_Point3d> unprojectedPoints;
	for (int i= 0; i < size; i+= 2)
	{
		const int x= list.at(i);
		const int y= m_WindowVSize - list.at(i + 1);
		glReadPixels(x, y , 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Depth);

		glc::gluUnProject(static_cast<GLdouble>(x), static_cast<GLdouble>(y) , Depth , m_pViewCam->modelViewMatrix().getData()
				, m_ProjectionMatrix.getData(), Viewport, &pX, &pY, &pZ);
		unprojectedPoints.append(GLC_Point3d(pX, pY, pZ));
	}

	return unprojectedPoints;
}

//////////////////////////////////////////////////////////////////////
// Private OpenGL Functions
//////////////////////////////////////////////////////////////////////

void GLC_Viewport::renderImagePlane()
{

	if(!GLC_State::isInSelectionMode())
	{
		if (m_pImagePlane != NULL)
		{
			m_pImagePlane->render();
		}
	}
}

void GLC_Viewport::render3DWidget()
{
	m_3DWidgetCollection.render(0, glc::WireRenderFlag);
	m_3DWidgetCollection.render(0, glc::TransparentRenderFlag);
}
//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

void GLC_Viewport::setWinGLSize(int HSize, int VSize)
{
	m_WindowHSize= HSize;
	m_WindowVSize= VSize;

	// from NeHe's Tutorial 3
	if (m_WindowVSize == 0)								// Prevent A Divide By Zero By
	{
		m_WindowVSize= 1;									// Making Height Equal One
	}

	glViewport(0,0,m_WindowHSize,m_WindowVSize);			// Reset The Current Viewport

	updateAspectRatio();

	updateProjectionMat();

	updateMinimumRatioSize();
}

GLC_uint GLC_Viewport::renderAndSelect(int x, int y)
{

	m_pQGLWidget->qglClearColor(Qt::black);
	GLC_State::setSelectionMode(true);
	// Draw the scene
	m_pQGLWidget->updateGL();
	GLC_State::setSelectionMode(false);

	return selectOnPreviousRender(x, y);
}

GLC_uint GLC_Viewport::selectOnPreviousRender(int x, int y)
{
	GLsizei width= m_SelectionSquareSize;
	GLsizei height= width;
	GLint newX= x - width / 2;
	GLint newY= (m_pQGLWidget->size().height() - y) - height / 2;
	if (newX < 0) newX= 0;
	if (newY < 0) newY= 0;

	return meaningfulIdInsideSquare(newX, newY, width, height);
}
GLC_uint GLC_Viewport::selectBody(GLC_3DViewInstance* pInstance, int x, int y)
{
	m_pQGLWidget->qglClearColor(Qt::black);
	GLC_State::setSelectionMode(true);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLC_Context::current()->glcLoadIdentity();

	glExecuteCam();

	// Draw the scene
	glDisable(GL_BLEND);
	GLC_Context::current()->glcEnableLighting(false);
	glDisable(GL_TEXTURE_2D);

	pInstance->renderForBodySelection();
	GLC_State::setSelectionMode(false);

	GLsizei width= 6;
	GLsizei height= width;
	GLint newX= x - width / 2;
	GLint newY= (m_pQGLWidget->size().height() - y) - height / 2;
	if (newX < 0) newX= 0;
	if (newY < 0) newY= 0;

	return meaningfulIdInsideSquare(newX, newY, width, height);
}

QPair<int, GLC_uint> GLC_Viewport::selectPrimitive(GLC_3DViewInstance* pInstance, int x, int y)
{
	QPair<int, GLC_uint> result;

	m_pQGLWidget->qglClearColor(Qt::black);
	GLC_State::setSelectionMode(true);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLC_Context::current()->glcLoadIdentity();

	glExecuteCam();

	// Draw the scene
	glDisable(GL_BLEND);
	GLC_Context::current()->glcEnableLighting(false);
	glDisable(GL_TEXTURE_2D);

	pInstance->renderForBodySelection();


	GLsizei width= 6;
	GLsizei height= width;
	GLint newX= x - width / 2;
	GLint newY= (m_pQGLWidget->size().height() - y) - height / 2;
	if (newX < 0) newX= 0;
	if (newY < 0) newY= 0;

	GLC_uint bodyId= meaningfulIdInsideSquare(newX, newY, width, height);
	if (bodyId == 0)
	{
		result.first= -1;
		result.second= 0;
	}
	else
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		result.first= pInstance->renderForPrimitiveSelection(bodyId);
		result.second= meaningfulIdInsideSquare(newX, newY, width, height);
	}
	GLC_State::setSelectionMode(false);
	return result;
}

QSet<GLC_uint> GLC_Viewport::selectInsideSquare(int x1, int y1, int x2, int y2)
{
	if (x1 > x2)
	{
		int xTemp= x1;
		x1= x2;
		x2= xTemp;
	}
	if (y2 > y1)
	{
		int yTemp= y1;
		y1= y2;
		y2= yTemp;
	}
	m_pQGLWidget->qglClearColor(Qt::black);
	GLC_State::setSelectionMode(true);
	// Draw the scene
	m_pQGLWidget->updateGL();
	GLC_State::setSelectionMode(false);

	GLsizei width= x2 - x1;
	GLsizei height= y1 - y2;
	GLint newX= x1;
	GLint newY= (m_pQGLWidget->size().height() - y1);
	if (newX < 0) newX= 0;
	if (newY < 0) newY= 0;

	return listOfIdInsideSquare(newX, newY, width, height);
}

GLC_uint GLC_Viewport::meaningfulIdInsideSquare(GLint x, GLint y, GLsizei width, GLsizei height)
{
	const int squareSize= width * height;
	const GLsizei arraySize= squareSize * 4; // 4 -> R G B A
	QVector<GLubyte> colorId(arraySize);

	// Get the array of pixels
	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, colorId.data());

	// Restore Background color
	m_pQGLWidget->qglClearColor(m_BackgroundColor);

	QHash<GLC_uint, int> idHash;
	QList<int> idWeight;

	// Find the most meaningful color
	GLC_uint returnId= 0;
	// There is nothing at the center
	int maxWeight= 0;
	int currentIndex= 0;
	for (int i= 0; i < squareSize; ++i)
	{
		GLC_uint id= glc::decodeRgbId(&colorId[i * 4]);
		if (idHash.contains(id))
		{
			const int currentWeight= ++(idWeight[idHash.value(id)]);
			if (maxWeight < currentWeight)
			{
				returnId= id;
				maxWeight= currentWeight;
			}
		}
		else if (id != 0)
		{
			idHash.insert(id, currentIndex++);
			idWeight.append(1);
			if (maxWeight < 1)
			{
				returnId= id;
				maxWeight= 1;
			}
		}
	}

	return returnId;
}

QSet<GLC_uint> GLC_Viewport::listOfIdInsideSquare(GLint x, GLint y, GLsizei width, GLsizei height)
{
	const int squareSize= width * height;
	const GLsizei arraySize= squareSize * 4; // 4 -> R G B A
	QVector<GLubyte> colorId(arraySize);

	// Get the array of pixels
	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, colorId.data());

	// Restore Background color
	m_pQGLWidget->qglClearColor(m_BackgroundColor);

	QSet<GLC_uint> idSet;

	// get the color inside square
	for (int i= 0; i < squareSize; ++i)
	{
		GLC_uint id= glc::decodeRgbId(&colorId[i * 4]);
		idSet << id;
	}

	return idSet;
}

void GLC_Viewport::updateMinimumRatioSize()
{
	int size= qMax(m_WindowHSize, m_WindowVSize);
	m_MinimumStaticRatioSize=  static_cast<double>(m_MinimumStaticPixelSize) / static_cast<double>(size) * 100.0;
	m_MinimumDynamicRatioSize= m_MinimumStaticRatioSize * 2.0;
	//qDebug() << "GLC_Viewport::updateMinimumRatioSize() m_MinimumRatioSize " << m_MinimumStaticRatioSize;
}

void GLC_Viewport::loadBackGroundImage(const QString& ImageFile)
{
	delete m_pImagePlane;
	m_pImagePlane= new GLC_ImagePlane(ImageFile);
}

void GLC_Viewport::loadBackGroundImage(const QImage& image)
{
	delete m_pImagePlane;
	m_pImagePlane= new GLC_ImagePlane(image);
}

void GLC_Viewport::deleteBackGroundImage()
{
	delete m_pImagePlane;
	m_pImagePlane= NULL;
}

void GLC_Viewport::setToOrtho(bool useOrtho)
{
	if (m_UseParallelProjection != useOrtho)
	{
		m_UseParallelProjection= useOrtho;
		updateProjectionMat();
	}

}

void GLC_Viewport::reframe(const GLC_BoundingBox& box)
{
	Q_ASSERT(!box.isEmpty());

	// Center view on the BoundingBox
	const GLC_Vector3d deltaVector(box.center() - m_pViewCam->target());
	m_pViewCam->translate(deltaVector);

	double cameraCover= box.boundingSphereRadius() * 2.0;

	// Compute Camera distance
	const double distance = cameraCover / m_ViewTangent;

	// Update Camera position
	m_pViewCam->setDistEyeTarget(distance);
}

bool GLC_Viewport::setDistMin(double DistMin)
{
	DistMin= fabs(DistMin);
	if (DistMin < m_DistanceMax)
	{
		m_dDistanceMini= DistMin;

		updateProjectionMat();	// Update OpenGL projection matrix

		return true;
	}
	else
	{
		qDebug("GLC_Viewport::SetDistMin : KO");
		return false;
	}

}

bool GLC_Viewport::setDistMax(double DistMax)
{
	DistMax= fabs(DistMax);
	if (DistMax > m_dDistanceMini)
	{
		m_DistanceMax= DistMax;

		// Update OpenGL projection matrix
		updateProjectionMat();

		return true;
	}
	else
	{
		qDebug("GLC_Viewport::SetDistMax : KO");
		return false;
	}
}

void GLC_Viewport::setDistMinAndMax(const GLC_BoundingBox& bBox)
{
	if(!bBox.isEmpty())
	{
		// The scene is not empty
		GLC_Matrix4x4 matComp(m_pViewCam->modelViewMatrix());

		// The bounding Box in Camera coordinate
		GLC_BoundingBox boundingBox(bBox);
		boundingBox.transform(matComp);

		// Increase size of the bounding box
		const double increaseFactor= 1.1;
		// Convert box distance in sphere distance
		const double center= fabs(boundingBox.center().z());
		const double radius= boundingBox.boundingSphereRadius();
		const double min= center - radius * increaseFactor;
		const double max= center + radius * increaseFactor;

		GLC_Point3d camEye(m_pViewCam->eye());
		camEye= matComp * camEye;

		if (min > 0.0)
		{
			// Outside bounding Sphere
			m_dDistanceMini= min;
			m_DistanceMax= max;
			//qDebug() << "distmin" << m_dCamDistMin;
			//qDebug() << "distmax" << m_dCamDistMax;
		}
		else
		{
			// Inside bounding Sphere
			m_dDistanceMini= qMin(0.01 * radius, m_pViewCam->distEyeTarget() / 4.0);
			m_DistanceMax= max;
			//qDebug() << "inside distmin" << m_dCamDistMin;
			//qDebug() << "inside distmax" << m_dCamDistMax;
		}
	}
	else
	{
		// The scene is empty
		m_dDistanceMini= m_pViewCam->distEyeTarget() / 2.0;
		m_DistanceMax= m_pViewCam->distEyeTarget();
	}

	// Update OpenGL projection matrix
	updateProjectionMat();
}

void GLC_Viewport::setBackgroundColor(QColor setColor)
{
	m_BackgroundColor= setColor;
	m_pQGLWidget->qglClearColor(m_BackgroundColor);
}

void GLC_Viewport::addClipPlane(GLenum planeGlEnum,GLC_Plane* pPlane)
{
	if (m_ClipPlanesHash.contains(planeGlEnum))
	{
		delete m_ClipPlanesHash.value(planeGlEnum);
		m_ClipPlanesHash.remove(planeGlEnum);
	}
	m_ClipPlanesHash.insert(planeGlEnum, pPlane);
}

void GLC_Viewport::removeClipPlane(GLenum planeGlEnum)
{
	if (m_ClipPlanesHash.contains(planeGlEnum))
	{
		delete m_ClipPlanesHash.value(planeGlEnum);
		m_ClipPlanesHash.remove(planeGlEnum);
	}
	else
	{
		qDebug() << "GLC_Viewport::removeClipPlane Clipp plane " << planeGlEnum << " Not found";
	}
}

void GLC_Viewport::removeAllClipPlane()
{
	// Delete clip planes
	QHash<GLenum, GLC_Plane*>::iterator iClip= m_ClipPlanesHash.begin();
	while (m_ClipPlanesHash.constEnd() != iClip)
	{
		delete iClip.value();
		++iClip;
	}
}

void GLC_Viewport::useClipPlane(bool flag)
{
	m_UseClipPlane= flag;
	if (m_UseClipPlane)
	{
		QHash<GLenum, GLC_Plane*>::iterator iClip= m_ClipPlanesHash.begin();
		while (m_ClipPlanesHash.constEnd() != iClip)
		{
			GLenum planeKey= iClip.key();
			GLC_Plane* pPlane= iClip.value();

			glClipPlane (planeKey, pPlane->data());
			glEnable (planeKey);
			++iClip;
		}
	}
	else
	{
		QHash<GLenum, GLC_Plane*>::iterator iClip= m_ClipPlanesHash.begin();
		while (m_ClipPlanesHash.constEnd() != iClip)
		{
			glDisable(iClip.key());
			++iClip;
		}
	}

}
