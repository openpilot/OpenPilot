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

//! \file glc_texture.h interface for the GLC_Texture class.

#ifndef GLC_TEXTURE_H_
#define GLC_TEXTURE_H_

#include <QFile>
#include <QtOpenGL>

/////////////////////////////////////////////////////////////////////
//! \class GLC_Texture
/*! \brief GLC_Texture : Image texture */

/*! Image texture define a texture map in 2 D coordinate system*/
//////////////////////////////////////////////////////////////////////


class GLC_Texture
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

public:
	//! Constructor with fileName
	GLC_Texture(const QGLContext*, const QString&);

	//! Constructor with QFile
	GLC_Texture(const QGLContext*, const QFile&);

	//! Constructor with QImage
	GLC_Texture(const QGLContext*, const QImage&);

	//! Copy constructor
	GLC_Texture(const GLC_Texture& TextureToCopy);

	//! Default Destructor
	virtual ~GLC_Texture();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the texture File Name
	inline QString fileName() const {return m_Name;}

	//! Return OpenGL Texture Id
	inline GLuint GL_ID() const {return m_GlTextureID;}

	//! Return true if the texture is loaded
	inline bool isLoaded() const {return (m_GlTextureID != 0);}

	//! Return the texture size
	inline QSize size() const {return m_TextureSize;}

	//! Return the maximum texture size
	static QSize maxSize() {return m_MaxTextureSize;}

	//! Return true if texture are the same
	bool operator==(const GLC_Texture&) const;

	//! Return true if the texture has alpha channel
	inline bool hasAlphaChannel() const
	{ return m_HasAlphaChannel;}


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
	void glLoadTexture(void);
	//! Bind texture in 2D mode
	void glcBindTexture(void);
//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////

private:
	//! OpenGL Context
	QGLContext *m_pQGLContext;

	//! Texture Name
	QString m_Name;

	//! OpenGL Texture ID
	GLuint	m_GlTextureID;

	//! QImage off the texture
	QImage *m_pTextureImage;

	//! Size of the texture
	QSize m_TextureSize;

	//! Flag to know if the texture has alpha channel
	bool m_HasAlphaChannel;

	//! Static member used to check texture size
	static QSize m_MaxTextureSize;
	static const QSize m_MinTextureSize;

};

#endif //GLC_TEXTURE_H_
