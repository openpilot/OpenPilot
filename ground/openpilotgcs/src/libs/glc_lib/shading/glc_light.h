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

//! \file glc_light.h interface for the GLC_Light class.

#ifndef GLC_LIGHT_H_
#define GLC_LIGHT_H_

#include <QColor>
#include <QSet>
#include <QHash>

#include "../glc_object.h"
#include "../maths/glc_vector3d.h"

#include "../glc_config.h"

class QGLContext;

//////////////////////////////////////////////////////////////////////
//! \class GLC_Light
/*! \brief GLC_Light : OpenGL Point Light*/

/*! An GLC_Light is an OpenGL point Light source at a 3D location\n
 * Point light is omnidirectional and have color*/
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_Light : public GLC_Object
{
public:
	//! Light Type enum
	enum LightType
	{
		LightPosition= 0,
		LightDirection= 1,
		LightSpot= 2
	};
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct a default GLC_Light
	/*! By default, ambient color is black, diffuse Color is white and specular color is white*/
	GLC_Light(const QGLContext* pContext= NULL, const QColor& color= Qt::white);

	//! Construct a default GLC_Light
	/*! By default, ambient color is black, diffuse Color is white and specular color is white*/
	GLC_Light(LightType lightType, const QGLContext* pContext= NULL, const QColor& color= Qt::white);

	//! Copy constructor
	GLC_Light(const GLC_Light& light);

	//! Delete OpenGL list
	virtual ~GLC_Light(void);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the maximum number light
	static int maxLightCount();

	//! Return the number of builtable light
	static int builtAbleLightCount(QGLContext* pContext);

	//! Return a GLC_Point3d representing light position
	inline GLC_Point3d position(void) const
	{return m_Position;}

	//! Return the QColor of the light's ambient color
	inline QColor ambientColor() const
	{return m_AmbientColor;}

	//! Return the QColor of the light's Diffuse color
	inline QColor diffuseColor() const
	{ return m_DiffuseColor;}

	//! Return the QColor of the light's Specular color
	inline QColor specularColor() const
	{return m_SpecularColor;}

	//! Return true if the light used two sided ilumination
	inline bool isTwoSided() const
	{return m_TwoSided;}

	//! Return the type of this light
	inline LightType type() const
	{return m_LightType;}

	//! Return the OpenGL ID of this light
	inline GLenum openglID() const
	{return m_LightID;}

	//! Return this light const attenuation
	inline GLfloat constantAttenuation() const
	{return m_ConstantAttenuation;}

	//! Return this light linear attenuation
	inline GLfloat linearAttenuation() const
	{return m_LinearAttenuation;}

	//! Return this light quadratic attenuation
	inline GLfloat quadraticAttenuation() const
	{return m_QuadraticAttenuation;}

	//! Return this light spot direction
	inline GLC_Vector3d spotDirection() const
	{return m_SpotDirection;}

	//! Return this light spot cutoff angle
	inline GLfloat spotCutoffAngle() const
	{return m_SpotCutoffAngle;}

	//! Return this light spot exponent
	inline GLfloat spotEponent() const
	{return m_SpotExponent;}
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Init Max number of light for this light context
	void initForThisContext();

	//! Set lihgt's position by a point
	void setPosition(const GLC_Point3d &pos);

	//! Set lihgt's position by a 3 GLfloat
	void setPosition(GLfloat x, GLfloat y, GLfloat z);

	//! Set light's ambiant color by a QColor
	void setAmbientColor(const QColor &color);

	//! Set light's diffuse color by a QColor
	void setDiffuseColor(const QColor &color);

	//! Set light's specular color by a QColor
	void setSpecularColor(const QColor &color);

	//! Set Mode
	void setTwoSided(const bool mode);

	//! Set this light constant attenuation
	void setConstantAttenuation(GLfloat constantAttenuation);

	//! Set this light linear attenuation
	void setLinearAttenuation(GLfloat linearAttenuation);

	//! Set this light quadratic attenuation
	void setQuadraticAttenuation(GLfloat quadraticAttenuation);

	//! Set this light spot direction
	void setSpotDirection(const GLC_Vector3d& direction);

	//! Set this light spot cutoff angle
	void setSpotCutoffAngle(GLfloat cutoffAngle);

	//! Set this light spot exponent
	void setSpotEponent(GLfloat exponent);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	// Disable the light
	void disable();

	//! Execute OpenGL light
	virtual void glExecute();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////

private:


//@}

//////////////////////////////////////////////////////////////////////
// Private services fonction
//////////////////////////////////////////////////////////////////////
private:
	//! Add context new light
	void addNewLight();

	//! Remove contetx light
	void removeThisLight();

//////////////////////////////////////////////////////////////////////
// Private Members
//////////////////////////////////////////////////////////////////////
private:
	//! OpenGL light ID
	GLenum m_LightID;

	//! The Light type
	LightType m_LightType;

	//! Light ambiant color
	QColor m_AmbientColor;
	//! Light diffuse color
	QColor m_DiffuseColor;
	//! Light specular color
	QColor m_SpecularColor;

	//! Light position
	GLC_Point3d m_Position;

	//! The spot light direction
	GLC_Vector3d m_SpotDirection;

	//! The spot exponent
	GLfloat m_SpotExponent;

	//! The spot cutoff angle
	GLfloat m_SpotCutoffAngle;

	//! Constant attenation
	GLfloat m_ConstantAttenuation;

	//! Linear attenuation
	GLfloat m_LinearAttenuation;

	//! Quadratic attenuation
	GLfloat m_QuadraticAttenuation;

	//! Lighting mode
	bool m_TwoSided;

	// Static member
	//! Maximum number of light
	static GLint m_MaxLight;

	//! The context of this light
	QGLContext* m_pContext;

	//! Flag to know if this light is valid
	bool m_IsValid;

	//! Mapping between context and light set
	static QHash<const QGLContext*, QSet<GLenum> > m_ContextToFreeLightSet;
};
#endif //GLC_LIGHT_H_
