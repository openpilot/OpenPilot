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
, m_MatPos()
, m_IsBoundingBoxValid(false)
, m_IsSelected(false)
, m_PolyFace(GL_FRONT_AND_BACK)
, m_PolyMode(GL_FILL)
, m_IsVisible(true)
, m_colorId()
, m_DefaultLOD(m_GlobalDefaultLOD)
{
	// Encode Color Id
	encodeIdInRGBA();

	//qDebug() << "GLC_3DViewInstance::GLC_3DViewInstance null instance ID = " << m_Uid;
	//qDebug() << "Number of instance" << (*m_pNumberOfInstance);
}

// Contruct instance with a geometry
GLC_3DViewInstance::GLC_3DViewInstance(GLC_VboGeom* pGeom)
: GLC_Object()
, m_3DRep(pGeom)
, m_pBoundingBox(NULL)
, m_MatPos()
, m_IsBoundingBoxValid(false)
, m_IsSelected(false)
, m_PolyFace(GL_FRONT_AND_BACK)
, m_PolyMode(GL_FILL)
, m_IsVisible(true)
, m_colorId()
, m_DefaultLOD(m_GlobalDefaultLOD)
{
	// Encode Color Id
	encodeIdInRGBA();

	setName(m_3DRep.name());

	//qDebug() << "GLC_3DViewInstance::GLC_3DViewInstance ID = " << m_Uid;
	//qDebug() << "Number of instance" << (*m_pNumberOfInstance);
}

// Contruct instance with a 3DRep
GLC_3DViewInstance::GLC_3DViewInstance(const GLC_3DRep& rep)
: GLC_Object()
, m_3DRep(rep)
, m_pBoundingBox(NULL)
, m_MatPos()
, m_IsBoundingBoxValid(false)
, m_IsSelected(false)
, m_PolyFace(GL_FRONT_AND_BACK)
, m_PolyMode(GL_FILL)
, m_IsVisible(true)
, m_colorId()
, m_DefaultLOD(m_GlobalDefaultLOD)
{
	// Encode Color Id
	encodeIdInRGBA();

	setName(m_3DRep.name());

	//qDebug() << "GLC_3DViewInstance::GLC_3DViewInstance ID = " << m_Uid;
	//qDebug() << "Number of instance" << (*m_pNumberOfInstance);
}

// Copy constructor
GLC_3DViewInstance::GLC_3DViewInstance(const GLC_3DViewInstance& inputNode)
: GLC_Object(inputNode)
, m_3DRep(inputNode.m_3DRep)
, m_pBoundingBox(NULL)
, m_MatPos(inputNode.m_MatPos)
, m_IsBoundingBoxValid(inputNode.m_IsBoundingBoxValid)
, m_IsSelected(inputNode.m_IsSelected)
, m_PolyFace(inputNode.m_PolyFace)
, m_PolyMode(inputNode.m_PolyMode)
, m_IsVisible(inputNode.m_IsVisible)
, m_colorId()
, m_DefaultLOD(inputNode.m_DefaultLOD)
{
	// Encode Color Id
	encodeIdInRGBA();

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
		encodeIdInRGBA();

		m_3DRep= inputNode.m_3DRep;
		if (NULL != inputNode.m_pBoundingBox)
		{
			m_pBoundingBox= new GLC_BoundingBox(*inputNode.m_pBoundingBox);
		}
		m_MatPos= inputNode.m_MatPos;
		m_IsBoundingBoxValid= inputNode.m_IsBoundingBoxValid;
		m_IsSelected= inputNode.m_IsSelected;
		m_PolyFace= inputNode.m_PolyFace;
		m_PolyMode= inputNode.m_PolyMode;
		m_IsVisible= inputNode.m_IsVisible;
		m_DefaultLOD= inputNode.m_DefaultLOD;

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
	else if (not m_3DRep.isEmpty())
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

// Clone the instance
GLC_3DViewInstance GLC_3DViewInstance::deepCopy() const
{

	GLC_3DRep* pRep= dynamic_cast<GLC_3DRep*>(m_3DRep.deepCopy());
	GLC_3DRep newRep(*pRep);
	delete pRep;
	GLC_3DViewInstance cloneInstance(newRep);

	if (NULL != m_pBoundingBox)
	{
		cloneInstance.m_pBoundingBox= new GLC_BoundingBox(*m_pBoundingBox);
	}

	cloneInstance.m_MatPos= m_MatPos;
	cloneInstance.m_IsBoundingBoxValid= m_IsBoundingBoxValid;
	cloneInstance.m_IsSelected= m_IsSelected;
	cloneInstance.m_PolyFace= m_PolyFace;
	cloneInstance.m_PolyMode= m_PolyMode;
	cloneInstance.m_IsVisible= m_IsVisible;
	return cloneInstance;
}

// Instanciate the instance
GLC_3DViewInstance GLC_3DViewInstance::instanciate()
{
	GLC_3DViewInstance instance(*this);
	instance.m_Uid= glc::GLC_GenID();
	// Encode Color Id
	encodeIdInRGBA();

	return instance;
}

// Return the GLC_uint decoded ID from RGBA encoded ID
GLC_uint GLC_3DViewInstance::decodeRgbId(const GLubyte* pcolorId)
{
	GLC_uint returnId= 0;
	returnId|= (GLC_uint)pcolorId[0] << (0 * 8);
	returnId|= (GLC_uint)pcolorId[1] << (1 * 8);
	returnId|= (GLC_uint)pcolorId[2] << (2 * 8);
	// Only get first 24 bits
	//returnId|= (GLC_uint)pcolorId[3] << (3 * 8);

	return returnId;
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////


// Set the instance Geometry
bool GLC_3DViewInstance::setGeometry(GLC_VboGeom* pGeom)
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
	m_MatPos= MultMat * m_MatPos;
	m_IsBoundingBoxValid= false;

	return *this;
}

// Replace the instance Matrix
GLC_3DViewInstance& GLC_3DViewInstance::setMatrix(const GLC_Matrix4x4 &SetMat)
{
	m_MatPos= SetMat;
	m_IsBoundingBoxValid= false;

	return *this;
}

// Reset the instance Matrix
GLC_3DViewInstance& GLC_3DViewInstance::resetMatrix(void)
{
	m_MatPos.setToIdentity();
	m_IsBoundingBoxValid= false;

	return *this;
}

// Polygon's display style
void GLC_3DViewInstance::setPolygonMode(GLenum Face, GLenum Mode)
{
	// Change the polygon mode only if there is a change
	if ((m_PolyFace != Face) || (m_PolyMode != Mode))
	{
		m_PolyFace= Face;
		m_PolyMode= Mode;
	}
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Display the instance
void GLC_3DViewInstance::glExecute(bool transparent, bool useLod, GLC_Viewport* pView)
{
	if (m_3DRep.isEmpty()) return;
	// Save current OpenGL Matrix
	glPushMatrix();
	glVisProperties();
	if(GLC_State::isInSelectionMode())
	{
		glColor3ubv(m_colorId); // D'ont use Alpha component
	}
	const int size= m_3DRep.numberOfBody();
	if (useLod and (NULL != pView))
	{
		for (int i= 0; i < size; ++i)
		{
			const int lodValue= choseLod(m_3DRep.geomAt(i)->boundingBox(), pView);
			if (lodValue <= 100)
			{
				m_3DRep.geomAt(i)->setCurrentLod(lodValue);
				m_3DRep.geomAt(i)->glExecute(m_IsSelected, transparent);
			}
		}
	}
	else
	{
		for (int i= 0; i < size; ++i)
		{
			int lodValue= 0;
			if (GLC_State::isPixelCullingActivated())
			{
				lodValue= choseLod(m_3DRep.geomAt(i)->boundingBox(), pView);
			}

			if (lodValue <= 100)
			{
				m_3DRep.geomAt(i)->setCurrentLod(m_DefaultLOD);
				m_3DRep.geomAt(i)->glExecute(m_IsSelected, transparent);
			}

		}
	}
	// Restore OpenGL Matrix
	glPopMatrix();
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

	m_pBoundingBox->transform(m_MatPos);
}

// Clear current instance
void GLC_3DViewInstance::clear()
{

	delete m_pBoundingBox;
	m_pBoundingBox= NULL;

	// invalidate the bounding box
	m_IsBoundingBoxValid= false;

}

//! Encode Id to RGBA color
void GLC_3DViewInstance::encodeIdInRGBA()
{
	m_colorId[0]= static_cast<GLubyte>((m_Uid >> (0 * 8)) & 0xFF);
	m_colorId[1]= static_cast<GLubyte>((m_Uid >> (1 * 8)) & 0xFF);
	m_colorId[2]= static_cast<GLubyte>((m_Uid >> (2 * 8)) & 0xFF);
	m_colorId[3]= static_cast<GLubyte>((m_Uid >> (3 * 8)) & 0xFF);
}

// Compute LOD
int GLC_3DViewInstance::choseLod(const GLC_BoundingBox& boundingBox, GLC_Viewport* pView)
{
	if (NULL == pView) return 0;
	const double diameter= boundingBox.boundingSphereRadius() * 2.0 * m_MatPos.scalingX();
	GLC_Vector4d center(m_MatPos * boundingBox.center());

	const double dist= (center - pView->cameraHandle()->eye()).norm();
	const double cameraCover= dist * pView->viewTangent();
	double ratio= diameter / cameraCover * 150.0;

	if (ratio > 100.0) ratio= 100.0;
	ratio= 100.0 - ratio;
	if ((ratio > 98.0) and GLC_State::isPixelCullingActivated()) ratio= 110.0;
	if (ratio < static_cast<double>(m_DefaultLOD)) ratio= static_cast<double>(m_DefaultLOD);
	//qDebug() << "RATIO = " << static_cast<int>(ratio);

	return static_cast<int>(ratio);
}


