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

//! \file glc_circle.cpp implementation of the GLC_Circle class.

#include "glc_circle.h"
#include "../glc_openglexception.h"
#include "../glc_state.h"

using namespace glc;
//////////////////////////////////////////////////////////////////////
// Constructor destructor
//////////////////////////////////////////////////////////////////////

GLC_Circle::GLC_Circle(const double &dRadius, double Angle)
:GLC_VboGeom("Circle", true)
, m_Radius(dRadius)
, m_nDiscret(GLC_DISCRET)
, m_dAngle(Angle)
, m_Step(0)
, m_SimpleGeomEngine()
{

}

GLC_Circle::GLC_Circle(const GLC_Circle& sourceCircle)
:GLC_VboGeom(sourceCircle)
, m_Radius(sourceCircle.m_Radius)
, m_nDiscret(sourceCircle.m_nDiscret)
, m_dAngle(sourceCircle.m_dAngle)
, m_SimpleGeomEngine(sourceCircle.m_SimpleGeomEngine)
{
	// Copy inner material hash
	MaterialHash::const_iterator i= m_MaterialHash.begin();
	MaterialHash newMaterialHash;
    while (i != m_MaterialHash.constEnd())
    {
        // update inner material use table
    	i.value()->delGLC_Geom(id());
    	GLC_Material* pNewMaterial= new GLC_Material(*(i.value()));
    	newMaterialHash.insert(pNewMaterial->id(), pNewMaterial);
    	pNewMaterial->addGLC_Geom(this);
         ++i;
    }
    m_MaterialHash= newMaterialHash;
}
GLC_Circle::~GLC_Circle()
{

}
//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// return the circle bounding box
GLC_BoundingBox& GLC_Circle::boundingBox(void)
{
	if (NULL == m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox();

		GLC_Point3d lower(-m_Radius, -m_Radius, -2.0 * EPSILON);
		GLC_Point3d upper(m_Radius, m_Radius, 2.0 * EPSILON);
		m_pBoundingBox->combine(lower);
		m_pBoundingBox->combine(upper);
	}

	return *m_pBoundingBox;
}

// Return a copy of the current geometry
GLC_VboGeom* GLC_Circle::clone() const
{
	return new GLC_Circle(*this);
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set Circle diameter
void GLC_Circle::setDiameter(double D)
{
	Q_ASSERT(not qFuzzyCompare(D, 0.0));
	setRadius(D / 2);
}

// Set Circle Radius
void GLC_Circle::setRadius(double R)
{
	Q_ASSERT(not qFuzzyCompare(R, 0.0));
	if (not qFuzzyCompare(R - m_Radius, 0.0))
	{	// Radius is changing
		m_Radius= R;
		m_GeometryIsValid= false;
		if (NULL != m_pBoundingBox)
		{
			delete m_pBoundingBox;
			m_pBoundingBox= NULL;
		}
	}
}

// Set Circle discret
void GLC_Circle::setDiscretion(int TargetDiscret)
{
	Q_ASSERT(TargetDiscret > 0);
	if (TargetDiscret != m_nDiscret)
	{
		m_nDiscret= TargetDiscret;
		if (m_nDiscret < 6) m_nDiscret= 6;
		m_GeometryIsValid= false;
	}
}

// Set Circle Angle
void GLC_Circle::setAngle(double AngleRadians)	// Angle in Radians
{
	Q_ASSERT((not qFuzzyCompare(AngleRadians, 0.0)) && (AngleRadians < 2 * PI));
	if (not qFuzzyCompare(AngleRadians - m_dAngle, 0.0))
	{	// Angle is changing
			m_dAngle= AngleRadians;
			m_GeometryIsValid= false;
			if (NULL != m_pBoundingBox)
			{
				delete m_pBoundingBox;
				m_pBoundingBox= NULL;
			}
	}
}

//////////////////////////////////////////////////////////////////////
// OpenGL Functions
//////////////////////////////////////////////////////////////////////

// Circle drawing
void GLC_Circle::glDraw(bool)
{
	const bool vboIsUsed= GLC_State::vboUsed();
	if (vboIsUsed)
	{
		m_SimpleGeomEngine.createVBOs();
		m_SimpleGeomEngine.useVBOs(true);
	}

	if (!m_GeometryIsValid)
	{
		VertexVector* pVertexVector= m_SimpleGeomEngine.vertexVectorHandle();
		// Calculate number of step
		m_Step= static_cast<GLuint>(static_cast<double>(m_nDiscret) * (m_dAngle / (2 * glc::PI)));
		if (m_Step < 2) m_Step= 2;

		// Vertex Vector
		const GLsizeiptr size= (m_Step + 1) * sizeof(GLC_Vertex);
		// Resize the Vertex vector
		pVertexVector->resize(m_Step + 1);
		// Fill Vertex Vector
		for (GLuint i= 0; i <= m_Step; ++i)
		{
			(*pVertexVector)[i].x= static_cast<float>(m_Radius * cos(i * m_dAngle / m_Step));
			(*pVertexVector)[i].y= static_cast<float>(m_Radius * sin(i * m_dAngle / m_Step));
		}

		// Index Vector
		const GLsizeiptr IndexSize = (m_Step + 1) * sizeof(GLuint);
		// Resize index vector
		QVector<GLuint>* pIndexVector= m_SimpleGeomEngine.indexVectorHandle();
		pIndexVector->resize(m_Step + 1);
		// Fill index vector
		for (GLuint i= 0; i <= m_Step; ++i)
		{
			(*pIndexVector)[i]= i;
		}

		if (vboIsUsed)
		{
			// Create VBO
			glBufferData(GL_ARRAY_BUFFER, size, pVertexVector->data(), GL_STATIC_DRAW);
			// Create IBO
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexSize, pIndexVector->data(), GL_STATIC_DRAW);
			// Clear Vector
			pVertexVector->clear();
			pIndexVector->clear();
		}
	}

	if (vboIsUsed)
	{
		// Use VBO
		glVertexPointer(2, GL_FLOAT, sizeof(GLC_Vertex), BUFFER_OFFSET(0));
		glEnableClientState(GL_VERTEX_ARRAY);
		//glDrawRangeElements(GL_LINE_STRIP, 0, m_Step + 1, m_Step + 1, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
		glDrawElements(GL_LINE_STRIP, m_Step + 1, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
		glDisableClientState(GL_VERTEX_ARRAY);
	}
	else
	{
		// Use Vertex Array
		glVertexPointer(2, GL_FLOAT, sizeof(GLC_Vertex), m_SimpleGeomEngine.vertexVectorHandle()->data());
		glEnableClientState(GL_VERTEX_ARRAY);
		glDrawElements(GL_LINE_STRIP, m_Step + 1, GL_UNSIGNED_INT, m_SimpleGeomEngine.indexVectorHandle()->data());
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	if (vboIsUsed)
	{
		m_SimpleGeomEngine.useVBOs(false);
	}


	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_Circle::GlDraw ", error);
		throw(OpenGlException);
	}
}
