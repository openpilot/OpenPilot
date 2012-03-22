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
//! \file glc_context.h interface for the GLC_Context class.

#ifndef GLC_CONTEXT_H_
#define GLC_CONTEXT_H_

#include <QtOpenGL>
#include <QGLContext>
#include <QGLFormat>
#include <QSharedPointer>
#include <QtDebug>

#include "glc_config.h"
#include "maths/glc_matrix4x4.h"
#include "glc_contextshareddata.h"
#include "glc_uniformshaderdata.h"

class GLC_ContextSharedData;

// OpenGL ES define
#if defined(QT_OPENGL_ES_2)
#define GLC_OPENGL_ES_2 1

#define GL_MODELVIEW					0x1700
#define GL_PROJECTION					0x1701
#endif


//#define GLC_OPENGL_ES_2 1

//////////////////////////////////////////////////////////////////////
//! \class GLC_Context
/*! \brief GLC_Context : Encapsulates OpenGL rendering context*/

/*! The GLC_Context class store all GLC state associated to an OpenGL rendering context.
 * This class is also used to simplified OpenGL and OpenGL-ES interoperability
 */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Context : public QGLContext
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

public:
	GLC_Context(const QGLFormat& format);
	virtual ~GLC_Context();

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the current context
	static GLC_Context* current();

	//! Return the model view matrix
	inline GLC_Matrix4x4 modelViewMatrix() const
	{Q_ASSERT(m_MatrixStackHash.contains(GL_MODELVIEW)); return m_MatrixStackHash.value(GL_MODELVIEW)->top();}

	//! Return the projection matrix
	inline GLC_Matrix4x4 projectionMatrix() const
	{Q_ASSERT(m_MatrixStackHash.contains(GL_PROJECTION)); return m_MatrixStackHash.value(GL_PROJECTION)->top();}

	//! Return lighting enable state
	inline bool lightingIsEnable() const
	{return m_LightingIsEnable.top();}
//@}
//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set the matrix mode
	void glcMatrixMode(GLenum mode);

	//! Replace the current matrix with the identity
	void glcLoadIdentity();

	//! push and pop the current matrix stack
	void glcPushMatrix();
	void glcPopMatrix();

	//! Replace the current matrix with the specified matrix
	void glcLoadMatrix(const GLC_Matrix4x4& matrix);

	//! Multiply the current matrix with the specified matrix
	void glcMultMatrix(const GLC_Matrix4x4& matrix);

	//! Multiply the current matrix by a translation matrix
	inline void glcTranslated(double x, double y, double z)
	{glcMultMatrix(GLC_Matrix4x4(x, y, z));}

	//! Multiply the current matrix by a general scaling matrix
	void glcScaled(double x, double y, double z);

	//! Multiply the current matrix with an orthographic matrix
	void glcOrtho(double left, double right, double bottom, double top, double nearVal, double farVal);

	//! Multiply the current matrix by a perspective matrix
	void glcFrustum(double left, double right, double bottom, double top, double nearVal, double farVal);

	//! Enable lighting
	void glcEnableLighting(bool enable);

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Make this context the current one
	virtual void makeCurrent();

	//! Make no context to be the current one
	virtual void doneCurrent();

	//! Update uniform variable
	inline void updateUniformVariables()
	{m_UniformShaderData.updateAll(this);}

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Private services Functions*/
//@{
//////////////////////////////////////////////////////////////////////
protected:
//@{

	virtual bool chooseContext(const QGLContext* shareContext= 0);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Private services Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:
//@{

	//! Init this context state
	void init();
//@}


//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:

	//! The current matrix mode
	GLenum m_CurrentMatrixMode;

	//! Mapping between matrixMode and matrix stack
	QHash<GLenum, QStack<GLC_Matrix4x4>* > m_MatrixStackHash;

	//! The context shared data
	QSharedPointer<GLC_ContextSharedData> m_ContextSharedData;

	//! The uniform data of the current shader
	GLC_UniformShaderData m_UniformShaderData;

	//! The current context
	static GLC_Context* m_pCurrentContext;

	//! Enable lighting state
	QStack<bool> m_LightingIsEnable;

	//! Lights enable state
	QHash<GLenum, bool> m_LightsEnableState;

};

#endif /* GLC_CONTEXT_H_ */
