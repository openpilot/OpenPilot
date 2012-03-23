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

//! \file glc_texture.h interface for the GLC_Texture class.

#ifndef GLC_TEXTURE_H_
#define GLC_TEXTURE_H_

#include <QFile>
#include <QtOpenGL>

#include "../glc_config.h"

/////////////////////////////////////////////////////////////////////
//! \class GLC_Texture
/*! \brief GLC_Texture : Image texture */

/*! Image texture define a texture map in 2 D coordinate system*/
//////////////////////////////////////////////////////////////////////


class GLC_LIB_EXPORT GLC_Texture
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

public:
	//! Default constructor
	GLC_Texture();

	//! Constructor with fileName
	GLC_Texture(const QString&);

	//! Constructor with QFile
	GLC_Texture(const QFile&);

	//! Constructor with QImage
	GLC_Texture(const QImage&, const QString& fileName= QString());

	//! Copy constructor
	GLC_Texture(const GLC_Texture& TextureToCopy);

	//! Overload "=" operator
	GLC_Texture& operator=(const GLC_Texture&);

	//! Default Destructor
	virtual ~GLC_Texture();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the QGLContext of the texture
	inline QGLContext* context() const
	{return m_pQGLContext;}

	//! Return the texture File Name
	inline QString fileName() const
	{return m_FileName;}

	//! Return OpenGL Texture Id
	inline GLuint GL_ID() const
	{return m_GlTextureID;}

	//! Return true if the texture is loaded
	inline bool isLoaded() const
	{return (m_GlTextureID != 0);}

	//! Return the texture size
	inline QSize size() const
	{return m_TextureSize;}

	//! Return the maximum texture size
	static QSize maxSize()
	{return m_MaxTextureSize;}

	//! Return true if texture are the same
	bool operator==(const GLC_Texture&) const;

	//! Return true if the texture has alpha channel
	inline bool hasAlphaChannel() const
	{ return m_HasAlphaChannel;}

	//! Return the an image of the texture
	inline QImage imageOfTexture() const
	{ return m_textureImage;}


//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	// Set the maximum texture size
	static void setMaxTextureSize(const QSize&);

//@}
//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Load the texture
	void glLoadTexture(QGLContext* pContext= NULL);
	//! Bind texture in 2D mode
	void glcBindTexture(void);


//////////////////////////////////////////////////////////////////////
/*! @name Private services functions */
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Load the image of this texture from the given fileName and return resutling image
	QImage loadFromFile(const QString& fileName);

	//! Remove this Opengl texture id
	void removeThisOpenGLTextureId();

	//! Add this Opengl texture id
	void addThisOpenGLTextureId();

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////

private:
	//! OpenGL Context
	QGLContext *m_pQGLContext;

	//! Texture Name
	QString m_FileName;

	//! OpenGL Texture ID
	GLuint	m_GlTextureID;

	//! QImage off the texture
	QImage m_textureImage;

	//! Size of the texture
	QSize m_TextureSize;

	//! Flag to know if the texture has alpha channel
	bool m_HasAlphaChannel;

	//! Static member used to check texture size
	static QSize m_MaxTextureSize;
	static const QSize m_MinTextureSize;

	//! Static hash table to manage OpenGL testure ID
	static QHash<GLuint, int> m_TextureIdUsage;
};

//! Non-member stream operator
QDataStream &operator<<(QDataStream &, const GLC_Texture &);
QDataStream &operator>>(QDataStream &, GLC_Texture &);


#endif //GLC_TEXTURE_H_
