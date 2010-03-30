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

//! \file glc_material.h interface for the GLC_Material class.

#ifndef GLC_MATERIAL_H_
#define GLC_MATERIAL_H_


#include "../glc_object.h"
#include "glc_texture.h"
#include <QHash>
#include <QColor>

class GLC_VboGeom;

typedef QHash< GLC_uint, GLC_VboGeom*> CWhereUsed;

//////////////////////////////////////////////////////////////////////
//! \class GLC_Material
/*! \brief GLC_Material : OpenGL surface material properties */

/*! An GLC_Material specifies surface material properties */
//////////////////////////////////////////////////////////////////////


class GLC_Material :
	public GLC_Object
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Construct Colored GLC_Material
	//! Default constructor
	GLC_Material();

	/*! By default, Ambiant Color is dark grey*/
	GLC_Material(const QColor &);

	/*! By default, Ambiant Color is dark grey*/
	GLC_Material(const QString& name, const GLfloat *);

	//! Construct textured GLC_Material
	GLC_Material(GLC_Texture* pTexture, const char *pName);

	//! Copy constructor
	/*! Hast usage table are not copying
	 */
	GLC_Material(const GLC_Material &InitMaterial);

	//! Remove material where used geometry
	virtual ~GLC_Material(void);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return true if the material is used
	bool isUnused() const {return m_WhereUsed.isEmpty();}

	//! Return true is material has attached texture
	bool hasTexture()
	{
		return m_pTexture != NULL;
	}
	//! Get Ambiant color
	QColor ambientColor() const;

	//! Get diffuse color
	QColor diffuseColor() const;

	//! Get specular color
	QColor specularColor() const;

	//! Get the emissive color
	QColor lightEmission() const;

	//! Get Shininess
	GLfloat shininess() const
	{
		return m_fShininess;
	}

	//! Get the texture File Name
	QString textureFileName() const;

	//! Get Texture Id
	GLuint textureID() const;

	//! return true if the texture is loaded
	bool textureIsLoaded() const;

	//! Return true if the material is transparent
	inline bool isTransparent() const {return  m_Transparency < 1.0;}

	//! Return true if material are the same
	bool operator==(const GLC_Material&) const;

	//! Return the material alpha
	inline double alpha() const
	{return m_DiffuseColor.alphaF();}

	//! Return the number of this material usage
	inline int numberOfUsage() const
	{return m_WhereUsed.size();}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Assignement operator
	/*! The Hash Table WhereUse
	 *  is not modified
	 */
	inline GLC_Material &operator=(const GLC_Material& mat)
	{
		setMaterial(&mat);
		return *this;
	}

	//! Set Material properties
	/*! The Hash Table WhereUse
	 *  is not modified
	 */
	 void setMaterial(const GLC_Material*);

	//! Set Ambiant Color
	void setAmbientColor(const QColor& ambientColor);

	//! Set Diffuse color
	void setDiffuseColor(const QColor& diffuseColor);

	//! Set Specular color
	void setSpecularColor(const QColor& specularColor);

	//! Set Emissive
	void setLightEmission(const QColor& lightEmission);

	//! Set Shininess
	void setShininess(GLfloat Shininess)
	{
		m_fShininess= Shininess;
	}

	//! Set Texture
	void setTexture(GLC_Texture* pTexture);

	//! remove Material Texture
	void removeTexture();

	//! Add Geometry to the "where used" hash table
	bool addGLC_Geom(GLC_VboGeom* pGeom);

	//! Remove Geometry to the "where used" hash table
	bool delGLC_Geom(GLC_uint Key);

	//! Set the material transparency
	void setTransparency(const qreal);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Load the texture
	void glLoadTexture(void);

	//! Execute OpenGL Material
	virtual void glExecute();

//@}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
private:
	//! Init Ambiant Color
	void initDiffuseColor(void);

	//! Init other color
	void initOtherColor(void);


//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////

private:

	//! Ambiant Color
	QColor m_AmbientColor;
	//! Diffuse Color
	QColor m_DiffuseColor;
	//! Specular Color
	QColor m_SpecularColor;

	//! emmisive lighting
	QColor m_LightEmission;

	//! Shiness
	GLfloat m_fShininess;

	//! Where Used Hash table
	CWhereUsed m_WhereUsed;

	//! material's texture
	GLC_Texture* m_pTexture;

	//! material transparency
	qreal m_Transparency;

};

#endif //GLC_MATERIAL_H_
