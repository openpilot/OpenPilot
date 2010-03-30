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

//! \file glc_texture.cpp implementation of the GLC_Texture class.


#include "glc_texture.h"
#include "../glc_exception.h"

#include <QtDebug>

// The default maximum texture size
QSize GLC_Texture::m_MaxTextureSize(676, 676);

// The Minimum texture size
const QSize GLC_Texture::m_MinTextureSize(10, 10);
//////////////////////////////////////////////////////////////////////
// Constructor Destructor
//////////////////////////////////////////////////////////////////////

// Constructor with fileName
GLC_Texture::GLC_Texture(const QGLContext *pContext, const QString &Filename)
: m_pQGLContext(const_cast<QGLContext*>(pContext))
, m_Name(Filename)
, m_GlTextureID(0)
, m_pTextureImage(new QImage(m_Name))
, m_TextureSize()
, m_HasAlphaChannel(m_pTextureImage->hasAlphaChannel())
{
	if (m_pTextureImage->isNull())
	{
		QString ErrorMess("GLC_Texture::GLC_Texture open image : ");
		ErrorMess.append(m_Name).append(" Failed");
		qDebug() << ErrorMess;
		GLC_Exception e(ErrorMess);
		throw(e);
	}
}
// Constructor with QFile
GLC_Texture::GLC_Texture(const QGLContext *pContext, const QFile &file)
: m_pQGLContext(const_cast<QGLContext*>(pContext))
, m_Name(file.fileName())
, m_GlTextureID(0)
, m_pTextureImage(new QImage(m_Name))
, m_TextureSize()
, m_HasAlphaChannel(m_pTextureImage->hasAlphaChannel())
{
	if (m_pTextureImage->isNull())
	{
		QString ErrorMess("GLC_Texture::GLC_Texture open image : ");
		ErrorMess.append(m_Name).append(" Failed");
		qDebug() << ErrorMess;
		GLC_Exception e(ErrorMess);
		throw(e);
	}
}

// Constructor with QImage
GLC_Texture::GLC_Texture(const QGLContext* pContext, const QImage& image)
: m_pQGLContext(const_cast<QGLContext*>(pContext))
, m_Name()
, m_GlTextureID(0)
, m_pTextureImage(new QImage(image))
, m_TextureSize()
, m_HasAlphaChannel(m_pTextureImage->hasAlphaChannel())
{
	Q_ASSERT(not m_pTextureImage->isNull());
}

GLC_Texture::GLC_Texture(const GLC_Texture &TextureToCopy)
: m_pQGLContext(TextureToCopy.m_pQGLContext)
, m_Name(TextureToCopy.m_Name)
, m_GlTextureID(0)
, m_pTextureImage(new QImage(m_Name))
, m_TextureSize(TextureToCopy.m_TextureSize)
, m_HasAlphaChannel(m_pTextureImage->hasAlphaChannel())
{
	if (m_pTextureImage->isNull())
	{
		QString ErrorMess("GLC_Texture::GLC_Texture open image : ");
		ErrorMess.append(m_Name).append(" Failed");
		qDebug() << ErrorMess;
		GLC_Exception e(ErrorMess);
		throw(e);
	}

}

GLC_Texture::~GLC_Texture()
{
	//qDebug() << "GLC_Texture::~GLC_Texture Texture ID : " << m_GlTextureID;
	if (m_GlTextureID != 0)
	{
		m_pQGLContext->deleteTexture(m_GlTextureID);
		m_GlTextureID= 0;
	}
	else
	{
		delete m_pTextureImage;
		m_pTextureImage = NULL;
	}
}
//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////

// Return true if texture are the same
bool GLC_Texture::operator==(const GLC_Texture& texture) const
{
	bool result;
	if (this == &texture)
	{
		result= true;
	}
	else
	{
		result= m_Name == texture.m_Name;
	}
	return result;
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

// Set the maximum texture size
void GLC_Texture::setMaxTextureSize(const QSize& size)
{
	if ((size.height() > m_MinTextureSize.height()) and (size.width() > m_MinTextureSize.width()))
	{
		m_MaxTextureSize= size;
	}
	else
	{
		qDebug() << "GLC_Texture::setMaxTextureSize m_MaxTextureSize set to m_MinTextureSize";
		m_MaxTextureSize= m_MinTextureSize;
	}
}

//////////////////////////////////////////////////////////////////////
// Private OpenGL functions
//////////////////////////////////////////////////////////////////////
// Load the texture
void GLC_Texture::glLoadTexture(void)
{
	if (m_GlTextureID == 0)
	{
		// Test image size
		if ((m_pTextureImage->height() > m_MaxTextureSize.height())
				or (m_pTextureImage->width() > m_MaxTextureSize.width()))
		{
			QImage rescaledImage;
			if(m_pTextureImage->height() > m_pTextureImage->width())
			{
				rescaledImage= m_pTextureImage->scaledToHeight(m_MaxTextureSize.height(), Qt::SmoothTransformation);
			}
			else
			{
				rescaledImage= m_pTextureImage->scaledToWidth(m_MaxTextureSize.width(), Qt::SmoothTransformation);
			}
			m_TextureSize= rescaledImage.size();
			m_GlTextureID= m_pQGLContext->bindTexture(rescaledImage);

		}
		else
		{
			m_TextureSize= m_pTextureImage->size();
			m_GlTextureID= m_pQGLContext->bindTexture(*m_pTextureImage);

		}
		delete m_pTextureImage;
		m_pTextureImage= NULL;
		//qDebug() << "GLC_Texture::glcBindTexture Texture ID = " << m_GlTextureID;
	}
}

// Bind texture in 2D mode
void GLC_Texture::glcBindTexture(void)
{
	if (m_GlTextureID == 0)
	{
		glLoadTexture();
	}
	glBindTexture(GL_TEXTURE_2D, m_GlTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

