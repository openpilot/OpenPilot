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

//! \file glc_instance.cpp implementation of the GLC_3DViewInstance class.

#include "glc_3dviewinstance.h"
#include "../shading/glc_selectionmaterial.h"
#include "../viewport/glc_viewport.h"
#include <QMutexLocker>
#include "../glc_state.h"

//! A Mutex
QMutex GLC_3DViewInstance::m_Mutex;

//! The global default LOD
int GLC_3DViewInstance::m_GlobalDefaultLOD= 10;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Default constructor
GLC_3DViewInstance::GLC_3DViewInstance()
: GLC_Object()
, m_3DRep()
, m_pBoundingBox(NULL)
, m_AbsoluteMatrix()
, m_IsBoundingBoxValid(false)
, m_RenderProperties()
, m_IsVisible(true)
, m_DefaultLOD(m_GlobalDefaultLOD)
, m_ViewableFlag(GLC_3DViewInstance::FullViewable)
, m_ViewableGeomFlag()
{
	// Encode Color Id
	glc::encodeRgbId(m_Uid, m_colorId);

	//qDebug() << "GLC_3DViewInstance::GLC_3DViewInstance null instance ID = " << m_Uid;
	//qDebug() << "Number of instance" << (*m_pNumberOfInstance);
}

// Contruct instance with a geometry
GLC_3DViewInstance::GLC_3DViewInstance(GLC_Geometry* pGeom)
: GLC_Object()
, m_3DRep(pGeom)
, m_pBoundingBox(NULL)
, m_AbsoluteMatrix()
, m_IsBoundingBoxValid(false)
, m_RenderProperties()
, m_IsVisible(true)
, m_DefaultLOD(m_GlobalDefaultLOD)
, m_ViewableFlag(GLC_3DViewInstance::FullViewable)
, m_ViewableGeomFlag()
{
	// Encode Color Id
	glc::encodeRgbId(m_Uid, m_colorId);

	setName(m_3DRep.name());

	//qDebug() << "GLC_3DViewInstance::GLC_3DViewInstance ID = " << m_Uid;
	//qDebug() << "Number of instance" << (*m_pNumberOfInstance);
}

// Contruct instance with a 3DRep
GLC_3DViewInstance::GLC_3DViewInstance(const GLC_3DRep& rep)
: GLC_Object()
, m_3DRep(rep)
, m_pBoundingBox(NULL)
, m_AbsoluteMatrix()
, m_IsBoundingBoxValid(false)
, m_RenderProperties()
, m_IsVisible(true)
, m_DefaultLOD(m_GlobalDefaultLOD)
, m_ViewableFlag(GLC_3DViewInstance::FullViewable)
, m_ViewableGeomFlag()
{
	// Encode Color Id
	glc::encodeRgbId(m_Uid, m_colorId);

	setName(m_3DRep.name());

	//qDebug() << "GLC_3DViewInstance::GLC_3DViewInstance ID = " << m_Uid;
	//qDebug() << "Number of instance" << (*m_pNumberOfInstance);
}

// Copy constructor
GLC_3DViewInstance::GLC_3DViewInstance(const GLC_3DViewInstance& inputNode)
: GLC_Object(inputNode)
, m_3DRep(inputNode.m_3DRep)
, m_pBoundingBox(NULL)
, m_AbsoluteMatrix(inputNode.m_AbsoluteMatrix)
, m_IsBoundingBoxValid(inputNode.m_IsBoundingBoxValid)
, m_RenderProperties(inputNode.m_RenderProperties)
, m_IsVisible(inputNode.m_IsVisible)
, m_DefaultLOD(inputNode.m_DefaultLOD)
, m_ViewableFlag(inputNode.m_ViewableFlag)
, m_ViewableGeomFlag(inputNode.m_ViewableGeomFlag)
{
	// Encode Color Id
	glc::encodeRgbId(m_Uid, m_colorId);

	if (NULL != inputNode.m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox(*inputNode.m_pBoundingBox);
	}
}


// Assignement operator
GLC_3DViewInstance& GLC_3DViewInstance::operator=(const GLC_3DViewInstance& inputNode)
{
	if (this != &inputNode)
	{
		// Clear this instance
		clear();
		GLC_Object::operator=(inputNode);
		// Encode Color Id
		glc::encodeRgbId(m_Uid, m_colorId);

		m_3DRep= inputNode.m_3DRep;
		if (NULL != inputNode.m_pBoundingBox)
		{
			m_pBoundingBox= new GLC_BoundingBox(*inputNode.m_pBoundingBox);
		}
		m_AbsoluteMatrix= inputNode.m_AbsoluteMatrix;
		m_IsBoundingBoxValid= inputNode.m_IsBoundingBoxValid;
		m_RenderProperties= inputNode.m_RenderProperties;
		m_IsVisible= inputNode.m_IsVisible;
		m_DefaultLOD= inputNode.m_DefaultLOD;
		m_ViewableFlag= inputNode.m_ViewableFlag;
		m_ViewableGeomFlag= inputNode.m_ViewableGeomFlag;

		//qDebug() << "GLC_3DViewInstance::operator= :ID = " << m_Uid;
		//qDebug() << "Number of instance" << (*m_pNumberOfInstance);
	}

	return *this;
}

// Destructor
GLC_3DViewInstance::~GLC_3DViewInstance()
{
	clear();
}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Get the bounding box
GLC_BoundingBox GLC_3DViewInstance::boundingBox(void)
{
	GLC_BoundingBox resultBox;
	if (boundingBoxValidity())
	{
		resultBox= *m_pBoundingBox;
	}
	else if (!m_3DRep.isEmpty())
	{
		computeBoundingBox();
		m_IsBoundingBoxValid= true;
		resultBox= *m_pBoundingBox;
	}

	return resultBox;
}

//! Set the global default LOD value
void GLC_3DViewInstance::setGlobalDefaultLod(int lod)
{
	QMutexLocker locker(&m_Mutex);
	m_GlobalDefaultLOD= lod;
}

void GLC_3DViewInstance::setVboUsage(bool usage)
{
	m_3DRep.setVboUsage(usage);
}

// Clone the instance
GLC_3DViewInstance GLC_3DViewInstance::deepCopy() const
{

	GLC_3DRep* pRep= dynamic_cast<GLC_3DRep*>(m_3DRep.deepCopy());
	Q_ASSERT(NULL != pRep);
	GLC_3DRep newRep(*pRep);
	delete pRep;
	GLC_3DViewInstance cloneInstance(newRep);

	if (NULL != m_pBoundingBox)
	{
		cloneInstance.m_pBoundingBox= new GLC_BoundingBox(*m_pBoundingBox);
	}

	cloneInstance.m_AbsoluteMatrix= m_AbsoluteMatrix;
	cloneInstance.m_IsBoundingBoxValid= m_IsBoundingBoxValid;
	cloneInstance.m_RenderProperties= m_RenderProperties;
	cloneInstance.m_IsVisible= m_IsVisible;
	cloneInstance.m_ViewableFlag= m_ViewableFlag;
	return cloneInstance;
}

// Instanciate the instance
GLC_3DViewInstance GLC_3DViewInstance::instanciate()
{
	GLC_3DViewInstance instance(*this);
	instance.m_Uid= glc::GLC_GenID();
	// Encode Color Id
	glc::encodeRgbId(m_Uid, m_colorId);

	return instance;
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////


// Set the instance Geometry
bool GLC_3DViewInstance::addGeometry(GLC_Geometry* pGeom)
{
	if (m_3DRep.contains(pGeom))
	{
		return false;
	}
	else
	{
		m_3DRep.addGeom(pGeom);
		return true;
	}
}

// Instance translation
GLC_3DViewInstance& GLC_3DViewInstance::translate(double Tx, double Ty, double Tz)
{
	multMatrix(GLC_Matrix4x4(Tx, Ty, Tz));

	return *this;
}


// Move instance with a 4x4Matrix
GLC_3DViewInstance& GLC_3DViewInstance::multMatrix(const GLC_Matrix4x4 &MultMat)
{
	m_AbsoluteMatrix= MultMat * m_AbsoluteMatrix;
	m_IsBoundingBoxValid= false;

	return *this;
}

// Replace the instance Matrix
GLC_3DViewInstance& GLC_3DViewInstance::setMatrix(const GLC_Matrix4x4 &SetMat)
{
	m_AbsoluteMatrix= SetMat;
	m_IsBoundingBoxValid= false;

	return *this;
}

// Reset the instance Matrix
GLC_3DViewInstance& GLC_3DViewInstance::resetMatrix(void)
{
	m_AbsoluteMatrix.setToIdentity();
	m_IsBoundingBoxValid= false;

	return *this;
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Display the instance
void GLC_3DViewInstance::render(glc::RenderFlag renderFlag, bool useLod, GLC_Viewport* pView)
{
	//qDebug() << "GLC_3DViewInstance::render render properties= " << m_RenderProperties.renderingMode();
	if (m_3DRep.isEmpty()) return;
	const int bodyCount= m_3DRep.numberOfBody();

	if (bodyCount != m_ViewableGeomFlag.size())
	{
		m_ViewableGeomFlag.fill(true, bodyCount);
	}

	m_RenderProperties.setRenderingFlag(renderFlag);

	// Save current OpenGL Matrix
	GLC_Context::current()->glcPushMatrix();
	OpenglVisProperties();

	// Change front face orientation if this instance absolute matrix is indirect
	if (m_AbsoluteMatrix.type() == GLC_Matrix4x4::Indirect)
	{
		glFrontFace(GL_CW);
	}
	if(GLC_State::isInSelectionMode())
	{
		glColor3ubv(m_colorId); // D'ont use Alpha component
	}

	if (useLod && (NULL != pView))
	{
		for (int i= 0; i < bodyCount; ++i)
		{
			if (m_ViewableGeomFlag.at(i))
			{
				const int lodValue= choseLod(m_3DRep.geomAt(i)->boundingBox(), pView, useLod);
				if (lodValue <= 100)
				{
					m_3DRep.geomAt(i)->setCurrentLod(lodValue);
					m_RenderProperties.setCurrentBodyIndex(i);
					m_3DRep.geomAt(i)->render(m_RenderProperties);
				}
			}
		}
	}
	else
	{
		for (int i= 0; i < bodyCount; ++i)
		{
			if (m_ViewableGeomFlag.at(i))
			{
				int lodValue= 0;
				if (GLC_State::isPixelCullingActivated() && (NULL != pView))
				{
					lodValue= choseLod(m_3DRep.geomAt(i)->boundingBox(), pView, useLod);
				}

				if (lodValue <= 100)
				{
					m_3DRep.geomAt(i)->setCurrentLod(m_DefaultLOD);
					m_RenderProperties.setCurrentBodyIndex(i);
					m_3DRep.geomAt(i)->render(m_RenderProperties);
				}
			}
		}
	}
	// Restore OpenGL Matrix
	GLC_Context::current()->glcPopMatrix();

	// Restore front face orientation if this instance absolute matrix is indirect
	if (m_AbsoluteMatrix.type() == GLC_Matrix4x4::Indirect)
	{
		glFrontFace(GL_CCW);
	}

}

// Display the instance in Body selection mode
void GLC_3DViewInstance::renderForBodySelection()
{
	Q_ASSERT(GLC_State::isInSelectionMode());
	if (m_3DRep.isEmpty()) return;

	// Save previous rendering mode and set the rendering mode to BodySelection
	glc::RenderMode previousRenderMode= m_RenderProperties.renderingMode();
	m_RenderProperties.setRenderingMode(glc::BodySelection);

	// Save current OpenGL Matrix
	GLC_Context::current()->glcPushMatrix();
	OpenglVisProperties();

	GLubyte colorId[4];
	const int size= m_3DRep.numberOfBody();
	for (int i= 0; i < size; ++i)
	{
		GLC_Geometry* pGeom= m_3DRep.geomAt(i);
		glc::encodeRgbId(pGeom->id(), colorId);
		glColor3ubv(colorId);
		pGeom->setCurrentLod(m_DefaultLOD);
		m_RenderProperties.setCurrentBodyIndex(i);
		pGeom->render(m_RenderProperties);
	}

	// Restore rendering mode
	m_RenderProperties.setRenderingMode(previousRenderMode);
	// Restore OpenGL Matrix
	GLC_Context::current()->glcPopMatrix();
}

// Display the instance in Primitive selection mode and return the body index
int GLC_3DViewInstance::renderForPrimitiveSelection(GLC_uint bodyId)
{
	Q_ASSERT(GLC_State::isInSelectionMode());
	if (m_3DRep.isEmpty()) return -1;
	// Save previous rendering mode and set the rendering mode to BodySelection
	glc::RenderMode previousRenderMode= m_RenderProperties.renderingMode();
	m_RenderProperties.setRenderingMode(glc::PrimitiveSelection);

	// Save current OpenGL Matrix
	GLC_Context::current()->glcPushMatrix();
	OpenglVisProperties();

	const int size= m_3DRep.numberOfBody();
	int i= 0;
	bool continu= true;
	while ((i < size) && continu)
	{
		GLC_Geometry* pGeom= m_3DRep.geomAt(i);
		if (pGeom->id() == bodyId)
		{
			pGeom->setCurrentLod(0);
			pGeom->render(m_RenderProperties);
			continu= false;
		}
		else ++i;
	}

	m_RenderProperties.setRenderingMode(previousRenderMode);

	// Restore OpenGL Matrix
	GLC_Context::current()->glcPopMatrix();

	return i;
}



//////////////////////////////////////////////////////////////////////
// private services functions
//////////////////////////////////////////////////////////////////////


// compute the instance bounding box
// m_pGeomList should be not null
void GLC_3DViewInstance::computeBoundingBox(void)
{
	if (m_3DRep.isEmpty()) return;

	if (m_pBoundingBox != NULL)
	{
		delete m_pBoundingBox;
		m_pBoundingBox= NULL;
	}
	m_pBoundingBox= new GLC_BoundingBox();
	const int size= m_3DRep.numberOfBody();
	for (int i= 0; i < size; ++i)
	{
		m_pBoundingBox->combine(m_3DRep.geomAt(i)->boundingBox());
	}

	m_pBoundingBox->transform(m_AbsoluteMatrix);
}

// Clear current instance
void GLC_3DViewInstance::clear()
{

	delete m_pBoundingBox;
	m_pBoundingBox= NULL;

	// invalidate the bounding box
	m_IsBoundingBoxValid= false;

}

// Compute LOD
int GLC_3DViewInstance::choseLod(const GLC_BoundingBox& boundingBox, GLC_Viewport* pView, bool useLod)
{
	if (NULL == pView) return 0;
	double pixelCullingRatio= 0.0;
	if (useLod)
	{
		pixelCullingRatio= pView->minimumDynamicPixelCullingRatio();
	}
	else
	{
		pixelCullingRatio= pView->minimumStaticPixelCullingRatio();
	}

	const double diameter= boundingBox.boundingSphereRadius() * 2.0 * m_AbsoluteMatrix.scalingX();
	GLC_Vector3d center(m_AbsoluteMatrix * boundingBox.center());

	const double dist= (center - pView->cameraHandle()->eye()).length();
	const double cameraCover= dist * pView->viewTangent();

	double ratio= diameter / cameraCover * 100.0;
	if (ratio > 100.0) ratio= 100.0;
	ratio= 100.0 - ratio;

	if ((ratio > (100.0 - pixelCullingRatio)) && GLC_State::isPixelCullingActivated()) ratio= 110.0;
	else if(useLod && (ratio > 50.0))
	{
		ratio= (ratio - 50.0) / 50.0 * 100.0;
		if (ratio < static_cast<double>(m_DefaultLOD)) ratio= static_cast<double>(m_DefaultLOD);
	}
	else
	{
		ratio= static_cast<double>(m_DefaultLOD);
	}

	return static_cast<int>(ratio);
}


