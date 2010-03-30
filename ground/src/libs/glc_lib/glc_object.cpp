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

//! \file GLC_Object.cpp Implementation of the GLC_Object class.

#include "glc_object.h"

//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////

GLC_Object::GLC_Object(const QString& name)
: m_QUuid(QUuid::createUuid())
, m_Uid(glc::GLC_GenID())	// Object ID
, m_Name(name)			// Object Name
{

}
// Copy constructor
GLC_Object::GLC_Object(const GLC_Object& sourceObject)
: m_QUuid(sourceObject.m_QUuid)
, m_Uid(sourceObject.m_Uid)
, m_Name(sourceObject.m_Name)
{
}

GLC_Object::~GLC_Object()
{

}


//////////////////////////////////////////////////////////////////////
// Get function
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Set function
//////////////////////////////////////////////////////////////////////


// Assignement operator
GLC_Object &GLC_Object::operator=(const GLC_Object& object)
{
	m_QUuid= object.m_QUuid;
	m_Uid= object.m_Uid;
	m_Name= object.m_Name;
	return *this;
}


