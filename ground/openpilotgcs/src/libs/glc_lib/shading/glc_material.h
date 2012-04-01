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

//! \file glc_material.h interface for the GLC_Material class.

#ifndef GLC_MATERIAL_H_
#define GLC_MATERIAL_H_


#include "../glc_object.h"
#include "glc_texture.h"
#include <QHash>
#include <QColor>
#include <QSet>

#include "../glc_config.h"

class GLC_Geometry;

typedef QHash< GLC_uint, GLC_Geometry*> WhereUsed;

//////////////////////////////////////////////////////////////////////
//! \class GLC_Material
/*! \brief GLC_Material : OpenGL surface material properties */

/*! An GLC_Material specifies surface material properties */
//////////////////////////////////////////////////////////////////////


class GLC_LIB_EXPORT GLC_Material : public GLC_Object
{
	friend QDataStream &operator<<(QDataStream &, const GLC_Material &);
	friend QDataStream &operator>>(QDataStream &, GLC_Material &);

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
	GLC_Material(GLC_Texture* pTexture, const QString& name= QString());

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
	//! Return the class Chunk ID
	static quint32 chunckID();

	//! Return true if the material is used
	inline bool isUnused() const
	{return m_WhereUsed.isEmpty() && m_OtherUsage.isEmpty();}

	//! Return true is material has attached texture
	inline bool hasTexture() const
	{return m_pTexture != NULL;}

	//! Get Ambiant color
	QColor ambientColor() const;

	//! Get diffuse color
	QColor diffuseColor() const;

	//! Get specular color
	QColor specularColor() const;

	//! Get the emissive color
	QColor emissiveColor() const;

	//! Get Shininess
	inline GLfloat shininess() const
	{return m_Shininess;}

	//! Get the texture File Name
	QString textureFileName() const;

	//! Get Texture Id
	GLuint textureID() const;

	//! return true if the texture is loaded
	bool textureIsLoaded() const;

	//! Return true if the material is transparent
	inline bool isTransparent() const
	{return  m_Opacity < 1.0;}

	//! Return true if materials are equivalent
	bool operator==(const GLC_Material&) const;

	//! Return the material opacity
	inline double opacity() const
	{return m_DiffuseColor.alphaF();}

	//! Return the number of this material usage
	inline int numberOfUsage() const
	{return m_WhereUsed.size() + m_OtherUsage.size();}

	//! Return the texture handle
	inline GLC_Texture* textureHandle() const
	{return m_pTexture;}

	//! Return the material hash code
	uint hashCode() const;

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
	void setEmissiveColor(const QColor& lightEmission);

	//! Set Shininess
	inline void setShininess(GLfloat Shininess)
	{ m_Shininess= Shininess;}

	//! Set Texture
	void setTexture(GLC_Texture* pTexture);

	//! remove Material Texture
	void removeTexture();

	//! Add Geometry to the "where used" hash table
	/*! This method is thread safe*/
	bool addGLC_Geom(GLC_Geometry* pGeom);

	//! Remove Geometry to the "where used" hash table
	/*! This method is thread safe*/
	bool delGLC_Geom(GLC_uint Key);

	//! Add the id to the other used Set
	/*! This method is thread safe*/
	bool addUsage(GLC_uint);

	//! Remove the id to the other used Set
	/*! This method is thread safe*/
	bool delUsage(GLC_uint);

	//! Set the material opacity
	void setOpacity(const qreal);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Load the texture
	void glLoadTexture(QGLContext* pContext= NULL);

	//! Execute OpenGL Material
	virtual void glExecute();

	//! Execute OpenGL Material with overWrite transparency
	virtual void glExecute(float);

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
	QColor m_EmissiveColor;

	//! Shiness
	GLfloat m_Shininess;

	//! Hash table of geomtries which used this material
	WhereUsed m_WhereUsed;

	//! Set of id of other objects that uses this material
	QSet<GLC_uint> m_OtherUsage;

	//! Material's texture
	GLC_Texture* m_pTexture;

	//! Material opacity
	qreal m_Opacity;

	//! Class chunk id
	static quint32 m_ChunkId;

};

//! Non-member stream operator
QDataStream &operator<<(QDataStream &, const GLC_Material &);
QDataStream &operator>>(QDataStream &, GLC_Material &);

#endif //GLC_MATERIAL_H_
