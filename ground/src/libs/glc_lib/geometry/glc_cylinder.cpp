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

//! \file glc_cylinder.cpp implementation of the GLC_Cylinder class.

#include "glc_cylinder.h"
#include "../glc_openglexception.h"
#include "../shading/glc_selectionmaterial.h"
#include "../glc_state.h"

#include <QVector>

using namespace glc;
//////////////////////////////////////////////////////////////////////
// Constructor destructor
//////////////////////////////////////////////////////////////////////


GLC_Cylinder::GLC_Cylinder(double dRadius, double dLength)
:GLC_VboGeom("Cylinder", false)
, m_Radius(dRadius)
, m_Length(dLength)
, m_Discret(GLC_POLYDISCRET)	// Default discretion
, m_EndedIsCaped(true)			// Cylinder ended are closed
, m_SimpleGeomEngine()
{
	Q_ASSERT((m_Radius > 0.0) && (m_Length > 0.0));
}

GLC_Cylinder::GLC_Cylinder(const GLC_Cylinder& sourceCylinder)
:GLC_VboGeom(sourceCylinder)
, m_Radius(sourceCylinder.m_Radius)
, m_Length(sourceCylinder.m_Length)
, m_Discret(sourceCylinder.m_Discret)
, m_EndedIsCaped(sourceCylinder.m_EndedIsCaped)
, m_SimpleGeomEngine(sourceCylinder.m_SimpleGeomEngine)
{
	Q_ASSERT((m_Radius > 0.0) && (m_Length > 0.0) && (m_Discret > 0));

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
GLC_Cylinder::~GLC_Cylinder()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// return the cylinder bounding box
GLC_BoundingBox& GLC_Cylinder::boundingBox(void)
{
	if (NULL == m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox();
		GLC_Point3d lower(-m_Radius, -m_Radius, 0.0);
		GLC_Point3d upper(m_Radius, m_Radius, m_Length);
		m_pBoundingBox->combine(lower);
		m_pBoundingBox->combine(upper);
	}
	return *m_pBoundingBox;
}

// Return a copy of the current geometry
GLC_VboGeom* GLC_Cylinder::clone() const
{
	return new GLC_Cylinder(*this);
}


// Get number of faces
unsigned int GLC_Cylinder::numberOfFaces() const
{
	return m_SimpleGeomEngine.numberOfFaces();
}

// Get number of vertex
unsigned int GLC_Cylinder::numberOfVertex() const
{
	return m_SimpleGeomEngine.numberOfVertex();
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
// Set Cylinder length
void GLC_Cylinder::setLength(double Length)
{
	Q_ASSERT(Length > 0.0);
	m_Length= Length;

	if (NULL != m_pBoundingBox)
	{
		delete m_pBoundingBox;
		m_pBoundingBox= NULL;
	}
	m_GeometryIsValid= false;
}

// Set Cylinder radius
void GLC_Cylinder::setRadius(double Radius)
{
	Q_ASSERT(Radius > 0.0);
	m_Radius= Radius;

	if (NULL != m_pBoundingBox)
	{
		delete m_pBoundingBox;
		m_pBoundingBox= NULL;
	}
	m_GeometryIsValid= false;
}

// Set Discretion
void GLC_Cylinder::setDiscretion(int TargetDiscret)
{
	Q_ASSERT(TargetDiscret > 0);
	if (TargetDiscret != m_Discret)
	{
		m_Discret= TargetDiscret;
		if (m_Discret < 6) m_Discret= 6;

		m_GeometryIsValid= false;
	}
}

// End Caps
void GLC_Cylinder::setEndedCaps(bool CapsEnded)
{
	if (m_EndedIsCaped != CapsEnded)
	{
		m_EndedIsCaped= CapsEnded;
	}
}

//////////////////////////////////////////////////////////////////////
// Private Opengl functions
//////////////////////////////////////////////////////////////////////

// Dessin du GLC_Cylinder
void GLC_Cylinder::glDraw(bool)
{
	const bool vboIsUsed= GLC_State::vboUsed();
	if (vboIsUsed)
	{
		m_SimpleGeomEngine.createVBOs();
		m_SimpleGeomEngine.useVBOs(true);
	}

	if (!m_GeometryIsValid)
	{
		double cosArray[m_Discret + 1];
		double sinArray[m_Discret + 1];
		for (int i= 0; i <= m_Discret; ++i)
		{
			cosArray[i]= m_Radius * cos(i * (2 * PI) / m_Discret);
			sinArray[i]= m_Radius * sin(i * (2 * PI) / m_Discret);
		}

		// Vertex Vector
		const GLsizei dataNbr= (m_Discret + 1) * 4;
		// Resize the Vertex vector
		VertexVector* pVertexVector= m_SimpleGeomEngine.vertexVectorHandle();
		pVertexVector->resize(dataNbr);

		for (GLsizei i= 0; i < (dataNbr / 4); ++i)
		{
			// Bottom
			(*pVertexVector)[i].x= static_cast<GLfloat>(cosArray[i]);
			(*pVertexVector)[i].y= static_cast<GLfloat>(sinArray[i]);
			(*pVertexVector)[i].z= 0.0f;
			GLC_Vector4d normal(cosArray[i], sinArray[i], 0.0);
			normal.setNormal(1.0);
			(*pVertexVector)[i].nx= static_cast<GLfloat>(normal.X());
			(*pVertexVector)[i].ny= static_cast<GLfloat>(normal.Y());
			(*pVertexVector)[i].nz= 0.0f;
			(*pVertexVector)[i].s= static_cast<float>(i) / static_cast<float>(m_Discret);
			(*pVertexVector)[i].t= 0.0f;

			// Top
			(*pVertexVector)[i + (m_Discret + 1)].x= (*pVertexVector)[i].x;
			(*pVertexVector)[i + (m_Discret + 1)].y= (*pVertexVector)[i].y;
			(*pVertexVector)[i + (m_Discret + 1)].z= static_cast<GLfloat>(m_Length);
			(*pVertexVector)[i + (m_Discret + 1)].nx= (*pVertexVector)[i].nx;
			(*pVertexVector)[i + (m_Discret + 1)].ny= (*pVertexVector)[i].ny;
			(*pVertexVector)[i + (m_Discret + 1)].nz= 0.0f;
			(*pVertexVector)[i + (m_Discret + 1)].s= (*pVertexVector)[i].s;
			(*pVertexVector)[i + (m_Discret + 1)].t= 1.0f;

			// Bottom Cap
			(*pVertexVector)[i + 2 * (m_Discret + 1)].x= (*pVertexVector)[i].x;
			(*pVertexVector)[i + 2 * (m_Discret + 1)].y= (*pVertexVector)[i].y;
			(*pVertexVector)[i + 2 * (m_Discret + 1)].z= 0.0f;
			(*pVertexVector)[i + 2 * (m_Discret + 1)].nx= 0.0f;
			(*pVertexVector)[i + 2 * (m_Discret + 1)].ny= 0.0f;
			(*pVertexVector)[i + 2 * (m_Discret + 1)].nz= -1.0f;
			(*pVertexVector)[i + 2 * (m_Discret + 1)].s= (*pVertexVector)[i].s;
			(*pVertexVector)[i + 2 * (m_Discret + 1)].t= 1.0f;

			// Top Cap
			(*pVertexVector)[i + 3 * (m_Discret + 1)].x= (*pVertexVector)[i].x;
			(*pVertexVector)[i + 3 * (m_Discret + 1)].y= (*pVertexVector)[i].y;
			(*pVertexVector)[i + 3 * (m_Discret + 1)].z= static_cast<GLfloat>(m_Length);
			(*pVertexVector)[i + 3 * (m_Discret + 1)].nx= 0.0f;
			(*pVertexVector)[i + 3 * (m_Discret + 1)].ny= 0.0f;
			(*pVertexVector)[i + 3 * (m_Discret + 1)].nz= 1.0f;
			(*pVertexVector)[i + 3 * (m_Discret + 1)].s= (*pVertexVector)[i].s;
			(*pVertexVector)[i + 3 * (m_Discret + 1)].t= 1.0f;
		}

		// IBO Vector
		const GLsizei indexNbr= 2 * (m_Discret + 1) + 2 * m_Discret;
		// Resize index vector
		QVector<GLuint>* pIndexVector= m_SimpleGeomEngine.indexVectorHandle();
		pIndexVector->resize(indexNbr);

		GLsizei j= 0;
		for (GLsizei i= 0; i < 2 * (m_Discret + 1); i+=2, ++j)
		{
			(*pIndexVector)[i]= j;
			(*pIndexVector)[i + 1]= j + (m_Discret + 1);
		}

		// Caps end
		j= 2 * (m_Discret + 1) + m_Discret / 2;
		GLsizei k= j - 1;
		GLsizei max = 2 * (m_Discret + 1) + m_Discret;
		for (GLsizei i= 2 * (m_Discret + 1); i < max; i+= 2, ++j, --k)
		{
			(*pIndexVector)[i]= j;
			if (i < (max - 1))
				(*pIndexVector)[i + 1]= k;
		}

		j= 3 * (m_Discret + 1) + m_Discret / 2;
		k= j - 1;
		for (GLsizei i= max; i < indexNbr; i+= 2, ++j, --k)
		{
			(*pIndexVector)[i]= j;
			if (i < (indexNbr - 1))
				(*pIndexVector)[i + 1]= k;
		}

		if (vboIsUsed)
		{
			// Create VBO
			const GLsizeiptr dataSize= dataNbr * sizeof(GLC_Vertex);
			glBufferData(GL_ARRAY_BUFFER, dataSize, pVertexVector->data(), GL_STATIC_DRAW);
			// Create IBO
			const GLsizeiptr indexSize = indexNbr * sizeof(GLuint);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, pIndexVector->data(), GL_STATIC_DRAW);
		}

	}

	if (vboIsUsed)
	{
		// Use VBO
		glVertexPointer(3, GL_FLOAT, sizeof(GLC_Vertex), BUFFER_OFFSET(0));
		glNormalPointer(GL_FLOAT, sizeof(GLC_Vertex), BUFFER_OFFSET(12));
		glTexCoordPointer(2, GL_FLOAT, sizeof(GLC_Vertex), BUFFER_OFFSET(24));

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		GLuint max= (m_Discret + 1) * 2;
		// Draw cylinder
		//glDrawRangeElements(GL_TRIANGLE_STRIP, 0, max, max, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
		glDrawElements(GL_TRIANGLE_STRIP, max, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

		// Fill ended if needed
		if (m_EndedIsCaped)
		{
			// Draw bottom cap
			//glDrawRangeElements(GL_TRIANGLE_STRIP, 0, m_Discret, m_Discret, GL_UNSIGNED_INT, BUFFER_OFFSET((max) * sizeof(unsigned int)));
			glDrawElements(GL_TRIANGLE_STRIP, m_Discret, GL_UNSIGNED_INT, BUFFER_OFFSET((max) * sizeof(unsigned int)));
			max+= m_Discret;
			// Draw top cap
			//glDrawRangeElements(GL_TRIANGLE_STRIP, 0, m_Discret, m_Discret, GL_UNSIGNED_INT, BUFFER_OFFSET((max) * sizeof(unsigned int)));
			glDrawElements(GL_TRIANGLE_STRIP, m_Discret, GL_UNSIGNED_INT, BUFFER_OFFSET((max) * sizeof(unsigned int)));
		}

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	else
	{
		// Use Vertex Array
		float* pVertexData= (float *) m_SimpleGeomEngine.vertexVectorHandle()->data();
		glVertexPointer(3, GL_FLOAT, sizeof(GLC_Vertex), pVertexData);
		glNormalPointer(GL_FLOAT, sizeof(GLC_Vertex), &pVertexData[3]);
		glTexCoordPointer(2, GL_FLOAT, sizeof(GLC_Vertex), &pVertexData[6]);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		GLuint max= (m_Discret + 1) * 2;
		// Draw cylinder
		glDrawElements(GL_TRIANGLE_STRIP, max, GL_UNSIGNED_INT, m_SimpleGeomEngine.indexVectorHandle()->data());

		// Fill ended if needed
		if (m_EndedIsCaped)
		{
			// Draw bottom cap
			glDrawElements(GL_TRIANGLE_STRIP, m_Discret, GL_UNSIGNED_INT, &m_SimpleGeomEngine.indexVectorHandle()->data()[max]);
			max+= m_Discret;
			// Draw top cap
			glDrawElements(GL_TRIANGLE_STRIP, m_Discret, GL_UNSIGNED_INT, &m_SimpleGeomEngine.indexVectorHandle()->data()[max]);
		}

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	if (vboIsUsed)
	{
		m_SimpleGeomEngine.useVBOs(false);
	}

	// OpenGL error handler
	GLenum error= glGetError();
	if (error != GL_NO_ERROR)
	{
		GLC_OpenGlException OpenGlException("GLC_Cylinder::GlDraw ", error);
		throw(OpenGlException);
	}

}
