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

//! \file glc_light.h interface for the GLC_Light class.

#ifndef GLC_LIGHT_H_
#define GLC_LIGHT_H_

#include <QColor>
#include "../glc_object.h"
#include "../maths/glc_vector4d.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Light
/*! \brief GLC_Light : OpenGL Point Light*/

/*! An GLC_Light is an OpenGL point Light source at a 3D location\n
 * Point light is omnidirectional and have color*/
//////////////////////////////////////////////////////////////////////

class GLC_Light :
	public GLC_Object
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct a default GLC_Light
	/*! By default, ambient color is black, diffuse Color is white and specular color is white*/
	GLC_Light();
	//* Construct GLC_Light with specified diffuse color
	GLC_Light(const QColor& );

	//! Delete OpenGL list
	virtual ~GLC_Light(void);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return a 4D GLC_Point4d representing light position
	inline GLC_Point4d position(void) const {return m_Position;}

	//! Return the QColor of the light's ambient color
	inline QColor ambientColor() {return m_AmbientColor;}

	//! Return the QColor of the light's Diffuse color
	inline QColor diffuseColor() { return m_DiffuseColor;}

	//! Return the QColor of the light's Specular color
	inline QColor specularColor() {return m_SpecularColor;}

	//! Return true if the light used two sided ilumination
	inline bool isTwoSided() const {return m_TwoSided;}
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set lihgt's position by a 4D point
	void setPosition(const GLC_Point4d &);

	//! Set lihgt's position by a 3 GLfloat
	void setPosition(GLfloat, GLfloat, GLfloat);

	//! Set light's ambiant color by a QColor
	void setAmbientColor(const QColor &);

	//! Set light's diffuse color by a QColor
	void setDiffuseColor(const QColor &);

	//! Set light's specular color by a QColor
	void setSpecularColor(const QColor &);

	//! Set Mode
	inline void setTwoSided(const bool mode)
	{
		m_TwoSided= mode;
		m_ListIsValid = false;
	}
//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Enable the light
	inline void enable(void) {glEnable(m_LightID);}

	// Disable the light
	inline void disable(void) {glDisable(m_LightID);}

	//! Execute OpenGL light
	virtual void glExecute(GLenum Mode= GL_COMPILE_AND_EXECUTE);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////

private:
	//! OpenGL light set up
	void glDraw(void);

	//! Display List creation
	void creationList(GLenum Mode);

private:

	//! Delete OpenGL Display list
	void deleteList(void);

//@}

//////////////////////////////////////////////////////////////////////
// Private Members
//////////////////////////////////////////////////////////////////////

private:
	//! OpenGL light ID
	GLenum m_LightID;

	//! OpenGL Display list ID
	GLuint m_ListID;

	//! OpenGL list validity
	bool m_ListIsValid;

	//! Light ambiant color
	QColor m_AmbientColor;
	//! Light diffuse color
	QColor m_DiffuseColor;
	//! Light specular color
	QColor m_SpecularColor;

	//! Light position
	GLC_Point4d m_Position;

	//! Lighting mode
	bool m_TwoSided;

};
#endif //GLC_LIGHT_H_
