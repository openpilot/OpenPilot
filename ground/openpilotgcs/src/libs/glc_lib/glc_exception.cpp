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

//! \file glc_exception.cpp implementation of the GLC_Exception class.

#include "glc_exception.h"

//////////////////////////////////////////////////////////////////////
// Constructor destructor
//////////////////////////////////////////////////////////////////////

GLC_Exception::GLC_Exception(const QString &message)
: m_ErrorDescription(message)
{
	GLC_ErrorLog::addError(QStringList(m_ErrorDescription));
}
GLC_Exception::~GLC_Exception() throw()
{
	
}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////
//! Return exception description
const char* GLC_Exception::what() const throw()
{
	QString exceptionmsg("GLC_Exception : ");
	exceptionmsg.append(m_ErrorDescription);
	return exceptionmsg.toAscii().data();
}
