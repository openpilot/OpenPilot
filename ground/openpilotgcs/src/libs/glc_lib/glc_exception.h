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

//! \file glc_exception.h Interface for the GLC_Exception class.

#ifndef GLC_EXCEPTION_H_
#define GLC_EXCEPTION_H_

#include <exception>
#include <QString>
#include "glc_errorlog.h"

#include "glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Exception
/*! \brief GLC_Exception : Base Class for all GLC_Exception Class
 */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Exception : public std::exception
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_Exception(const QString &message);
	
	//! Destructor
	virtual ~GLC_Exception() throw();
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
	
	//! Error description
	QString m_ErrorDescription;
};
	
#endif /*GLC_EXCEPTION_H_*/
