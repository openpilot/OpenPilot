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

//! \file glc_box.cpp implementation of the GLC_Box class.

#include "glc_box.h"
#include "../glc_openglexception.h"
#include "../glc_state.h"

//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////

GLC_Box::GLC_Box(double dLx, double dLy, double dlz)
:GLC_VboGeom("Box", false)
, m_dLgX(dLx)
, m_dLgY(dLy)
, m_dLgZ(dlz)
, m_SimpleGeomEngine()
{

}
// Copy constructor
GLC_Box::GLC_Box(const GLC_Box& box)
:GLC_VboGeom(box)
, m_dLgX(box.m_dLgX)
, m_dLgY(box.m_dLgY)
, m_dLgZ(box.m_dLgZ)
, m_SimpleGeomEngine(box.m_SimpleGeomEngine)
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
GLC_Box::~GLC_Box()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Get X length
double GLC_Box::getLgX(void) const
{
	return m_dLgX;
}

// Get Y length
double GLC_Box::getLgY(void) const
{
	return m_dLgY;
}

// Get Z length
double GLC_Box::getLgZ(void) const
{
	return m_dLgZ;
}

// return the box bounding box
GLC_BoundingBox& GLC_Box::boundingBox(void)
{
	if (NULL == m_pBoundingBox)
	{
		m_pBoundingBox= new GLC_BoundingBox();
		GLC_Vector3d lower(-m_dLgX / 2.0, -m_dLgY / 2.0, -m_dLgZ / 2.0);
		GLC_Vector3d upper(m_dLgX / 2.0, m_dLgY / 2.0, m_dLgZ / 2.0);
		m_pBoundingBox->combine(lower);
		m_pBoundingBox->combine(upper);
	}
	return *m_pBoundingBox;
}

// Return a copy of the current geometry
GLC_VboGeom* GLC_Box::clone() const
{
	return new GLC_Box(*this);
}


// Get number of faces
unsigned int GLC_Box::numberOfFaces() const
{
	return m_SimpleGeomEngine.numberOfFaces();
}

// Get number of vertex
unsigned int GLC_Box::numberOfVertex() const
{
	return m_SimpleGeomEngine.numberOfVertex();
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set X length
void GLC_Box::setLgX(double LgX)
{
	Q_ASSERT(LgX > 0);
	m_dLgX= LgX;
	m_GeometryIsValid= false;
	if (NULL != m_pBoundingBox)
	{
		delete m_pBoundingBox;
		m_pBoundingBox= NULL;
	}
}

// Set Y length
void GLC_Box::setLgY(double LgY)
{
	Q_ASSERT(LgY > 0);
	m_dLgY= LgY;
	m_GeometryIsValid= false;
	if (NULL != m_pBoundingBox)
	{
		delete m_pBoundingBox;
		m_pBoundingBox= NULL;
	}
}

// Set Z length
void GLC_Box::setLgZ(double LgZ)
{
	Q_ASSERT(LgZ > 0);
	m_dLgZ= LgZ;
	m_GeometryIsValid= false;
	if (NULL != m_pBoundingBox)
	{
		delete m_pBoundingBox;
		m_pBoundingBox= NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Private OpenGL functions
//////////////////////////////////////////////////////////////////////

// Box Set Up
void GLC_Box::glDraw(bool)
{
	const bool vboIsUsed= GLC_State::vboUsed();
	if (vboIsUsed)
	{
		m_SimpleGeomEngine.createVBOs();
		m_SimpleGeomEngine.useVBOs(true);
	}

	if (!m_GeometryIsValid)
	{
		// Resize the Vertex vector
		VertexVector* pVertexVector= m_SimpleGeomEngine.vertexVectorHandle();
		pVertexVector->resize(24); // 3 normals per vertex and 8 Vertex => 3 x 8 = 24

		const GLfloat lgX= static_cast<const GLfloat>(m_dLgX / 2.0);
		const GLfloat lgY= static_cast<const GLfloat>(m_dLgY / 2.0);
		const GLfloat lgZ= static_cast<const GLfloat>(m_dLgZ / 2.0);

		// Vertex Vector
		// Face 1
		(*pVertexVector)[0].x= -lgX; (*pVertexVector)[0].y= -lgY; (*pVertexVector)[0].z= lgZ;
		(*pVertexVector)[0].nx= 0.0f; (*pVertexVector)[0].ny= 0.0f; (*pVertexVector)[0].nz= 1.0f;
		(*pVertexVector)[0].s= 0.0f; (*pVertexVector)[0].t= 0.0f;

		(*pVertexVector)[1].x= lgX; (*pVertexVector)[1].y= -lgY; (*pVertexVector)[1].z= lgZ;
		(*pVertexVector)[1].nx= 0.0f; (*pVertexVector)[1].ny= 0.0f; (*pVertexVector)[1].nz= 1.0f;
		(*pVertexVector)[1].s= 1.0f; (*pVertexVector)[1].t= 0.0f;

		(*pVertexVector)[2].x= lgX; (*pVertexVector)[2].y= lgY; (*pVertexVector)[2].z= lgZ;
		(*pVertexVector)[2].nx= 0.0f; (*pVertexVector)[2].ny= 0.0f; (*pVertexVector)[2].nz= 1.0f;
		(*pVertexVector)[2].s= 1.0f; (*pVertexVector)[2].t= 1.0f;

		(*pVertexVector)[3].x= -lgX; (*pVertexVector)[3].y= lgY; (*pVertexVector)[3].z= lgZ;
		(*pVertexVector)[3].nx= 0.0f; (*pVertexVector)[3].ny= 0.0f; (*pVertexVector)[3].nz= 1.0f;
		(*pVertexVector)[3].s= 0.0f; (*pVertexVector)[3].t= 1.0f;

		// Face 2
		(*pVertexVector)[4].x= lgX; (*pVertexVector)[4].y= -lgY; (*pVertexVector)[4].z= lgZ;
		(*pVertexVector)[4].nx= 1.0f; (*pVertexVector)[4].ny= 0.0f; (*pVertexVector)[4].nz= 0.0f;
		(*pVertexVector)[4].s= 0.0f; (*pVertexVector)[4].t= 0.0f;

		(*pVertexVector)[5].x= lgX; (*pVertexVector)[5].y= -lgY; (*pVertexVector)[5].z= -lgZ;
		(*pVertexVector)[5].nx= 1.0f; (*pVertexVector)[5].ny= 0.0f; (*pVertexVector)[5].nz= 0.0f;
		(*pVertexVector)[5].s= 1.0f; (*pVertexVector)[5].t= 0.0f;

		(*pVertexVector)[6].x= lgX; (*pVertexVector)[6].y= lgY; (*pVertexVector)[6].z= -lgZ;
		(*pVertexVector)[6].nx= 1.0f; (*pVertexVector)[6].ny= 0.0f; (*pVertexVector)[6].nz= 0.0f;
		(*pVertexVector)[6].s= 1.0f; (*pVertexVector)[6].t= 1.0f;

		(*pVertexVector)[7].x= lgX; (*pVertexVector)[7].y= lgY; (*pVertexVector)[7].z= lgZ;
		(*pVertexVector)[7].nx= 1.0f; (*pVertexVector)[7].ny= 0.0f; (*pVertexVector)[7].nz= 0.0f;
		(*pVertexVector)[7].s= 0.0f; (*pVertexVector)[7].t= 1.0f;

		// Face 3
		(*pVertexVector)[8].x= -lgX; (*pVertexVector)[8].y= -lgY; (*pVertexVector)[8].z= -lgZ;
		(*pVertexVector)[8].nx= -1.0f; (*pVertexVector)[8].ny= 0.0f; (*pVertexVector)[8].nz= 0.0f;
		(*pVertexVector)[8].s= 0.0f; (*pVertexVector)[8].t= 0.0f;

		(*pVertexVector)[9].x= -lgX; (*pVertexVector)[9].y= -lgY; (*pVertexVector)[9].z= lgZ;
		(*pVertexVector)[9].nx= -1.0f; (*pVertexVector)[9].ny= 0.0f; (*pVertexVector)[9].nz= 0.0f;
		(*pVertexVector)[9].s= 1.0f; (*pVertexVector)[9].t= 0.0f;

		(*pVertexVector)[10].x= -lgX; (*pVertexVector)[10].y= lgY; (*pVertexVector)[10].z= lgZ;
		(*pVertexVector)[10].nx= -1.0f; (*pVertexVector)[10].ny= 0.0f; (*pVertexVector)[10].nz= 0.0f;
		(*pVertexVector)[10].s= 1.0f; (*pVertexVector)[10].t= 1.0f;

		(*pVertexVector)[11].x= -lgX; (*pVertexVector)[11].y= lgY; (*pVertexVector)[11].z= -lgZ;
		(*pVertexVector)[11].nx= -1.0f; (*pVertexVector)[11].ny= 0.0f; (*pVertexVector)[11].nz= 0.0f;
		(*pVertexVector)[11].s= 0.0f; (*pVertexVector)[11].t= 1.0f;

		// Face 4
		(*pVertexVector)[12].x= lgX; (*pVertexVector)[12].y= -lgY; (*pVertexVector)[12].z= -lgZ;
		(*pVertexVector)[12].nx= 0.0f; (*pVertexVector)[12].ny= 0.0f; (*pVertexVector)[12].nz= -1.0f;
		(*pVertexVector)[12].s= 0.0f; (*pVertexVector)[12].t= 0.0f;

		(*pVertexVector)[13].x= -lgX; (*pVertexVector)[13].y= -lgY; (*pVertexVector)[13].z= -lgZ;
		(*pVertexVector)[13].nx= 0.0f; (*pVertexVector)[13].ny= 0.0f; (*pVertexVector)[13].nz= -1.0f;
		(*pVertexVector)[13].s= 1.0f; (*pVertexVector)[13].t= 0.0f;

		(*pVertexVector)[14].x= -lgX; (*pVertexVector)[14].y= lgY; (*pVertexVector)[14].z= -lgZ;
		(*pVertexVector)[14].nx= 0.0f; (*pVertexVector)[14].ny= 0.0f; (*pVertexVector)[14].nz= -1.0f;
		(*pVertexVector)[14].s= 1.0f; (*pVertexVector)[14].t= 1.0f;

		(*pVertexVector)[15].x= lgX; (*pVertexVector)[15].y= lgY; (*pVertexVector)[15].z= -lgZ;
		(*pVertexVector)[15].nx= 0.0f; (*pVertexVector)[15].ny= 0.0f; (*pVertexVector)[15].nz= -1.0f;
		(*pVertexVector)[15].s= 0.0f; (*pVertexVector)[15].t= 1.0f;

		// Face 5
		(*pVertexVector)[16].x= -lgX; (*pVertexVector)[16].y= lgY; (*pVertexVector)[16].z= lgZ;
		(*pVertexVector)[16].nx= 0.0f; (*pVertexVector)[16].ny= 1.0f; (*pVertexVector)[16].nz= 0.0f;
		(*pVertexVector)[16].s= 0.0f; (*pVertexVector)[16].t= 0.0f;

		(*pVertexVector)[17].x= lgX; (*pVertexVector)[17].y= lgY; (*pVertexVector)[17].z= lgZ;
		(*pVertexVector)[17].nx= 0.0f; (*pVertexVector)[17].ny= 1.0f; (*pVertexVector)[17].nz= 0.0f;
		(*pVertexVector)[17].s= 1.0f; (*pVertexVector)[17].t= 0.0f;

		(*pVertexVector)[18].x= lgX; (*pVertexVector)[18].y= lgY; (*pVertexVector)[18].z= -lgZ;
		(*pVertexVector)[18].nx= 0.0f; (*pVertexVector)[18].ny= 1.0f; (*pVertexVector)[18].nz= 0.0f;
		(*pVertexVector)[18].s= 1.0f; (*pVertexVector)[18].t= 1.0f;

		(*pVertexVector)[19].x= -lgX; (*pVertexVector)[19].y= lgY; (*pVertexVector)[19].z= -lgZ;
		(*pVertexVector)[19].nx= 0.0f; (*pVertexVector)[19].ny= 1.0f; (*pVertexVector)[19].nz= 0.0f;
		(*pVertexVector)[19].s= 0.0f; (*pVertexVector)[19].t= 1.0f;

		// Face 6
		(*pVertexVector)[20].x= -lgX; (*pVertexVector)[20].y= -lgY; (*pVertexVector)[20].z= -lgZ;
		(*pVertexVector)[20].nx= 0.0f; (*pVertexVector)[20].ny= -1.0f; (*pVertexVector)[20].nz= 0.0f;
		(*pVertexVector)[20].s= 0.0f; (*pVertexVector)[20].t= 0.0f;

		(*pVertexVector)[21].x= lgX; (*pVertexVector)[21].y= -lgY; (*pVertexVector)[21].z= -lgZ;
		(*pVertexVector)[21].nx= 0.0f; (*pVertexVector)[21].ny= -1.0f; (*pVertexVector)[21].nz= 0.0f;
		(*pVertexVector)[21].s= 1.0f; (*pVertexVector)[21].t= 0.0f;

		(*pVertexVector)[22].x= lgX; (*pVertexVector)[22].y= -lgY; (*pVertexVector)[22].z= lgZ;
		(*pVertexVector)[22].nx= 0.0f; (*pVertexVector)[22].ny= -1.0f; (*pVertexVector)[22].nz= 0.0f;
		(*pVertexVector)[22].s= 1.0f; (*pVertexVector)[22].t= 1.0f;

		(*pVertexVector)[23].x= -lgX; (*pVertexVector)[23].y= -lgY; (*pVertexVector)[23].z= lgZ;
		(*pVertexVector)[23].nx= 0.0f; (*pVertexVector)[23].ny= -1.0f; (*pVertexVector)[23].nz= 0.0f;
		(*pVertexVector)[23].s= 0.0f; (*pVertexVector)[23].t= 1.0f;


		// Index Vector
		const GLsizei indexNbr= 36; // 6 face * 2 * 3 Vertexs
		// Fill index Vector
		QVector<GLuint>* pIndexVector= m_SimpleGeomEngine.indexVectorHandle();
		pIndexVector->clear();
		(*pIndexVector) << 0 << 1 << 2 << 2 << 3 << 0;		// Face 1
		(*pIndexVector) << 4 << 5 << 6 << 6 << 7 << 4;		// Face 2
		(*pIndexVector) << 8 << 9 << 10 << 10 << 11 << 8;		// Face 3
		(*pIndexVector) << 12 << 13 << 14 << 14 << 15 << 12;		// Face 4
		(*pIndexVector) << 16 << 17 << 18 << 18 << 19 << 16;		// Face 5
		(*pIndexVector) << 20 << 21 << 22 << 22 << 23 << 20;		// Face 6

		if (vboIsUsed)
		{
			const GLsizeiptr size= 24 * sizeof(GLC_Vertex);
			glBufferData(GL_ARRAY_BUFFER, size, pVertexVector->data(), GL_STATIC_DRAW);

			const GLsizeiptr IndexSize = indexNbr * sizeof(GLuint);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexSize, pIndexVector->data(), GL_STATIC_DRAW);
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

		//glDrawRangeElements(GL_TRIANGLES, 0, 36, 36, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

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

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, m_SimpleGeomEngine.indexVectorHandle()->data());

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
		GLC_OpenGlException OpenGlException("GLC_Box::GlDraw ", error);
		throw(OpenGlException);
	}
}
