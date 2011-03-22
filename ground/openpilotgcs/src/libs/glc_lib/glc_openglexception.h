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

//! \file glc_openglexception.h Interface for the GLC_OpenGlException class.

#ifndef GLC_OPENGLEXCEPTION_H_
#define GLC_OPENGLEXCEPTION_H_


#include "glc_exception.h"

#include <QtOpenGL>

#include "glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_OpenGlException
/*! \brief GLC_OpenGlException : Class for all OpenGL error
 */
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_OpenGlException : public GLC_Exception
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_OpenGlException(const QString& message, GLenum glError);
	
	//! Destructor
	virtual ~GLC_OpenGlException() throw();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:	
	//! Return exception description
	virtual const char* what() const throw();
//@}

//////////////////////////////////////////////////////////////////////
// protected members
//////////////////////////////////////////////////////////////////////
protected:
	
	//! Opengl Error description
	QString m_GlErrorDescription;

};

#endif /*GLC_OPENGLEXCEPTION_H_*/
