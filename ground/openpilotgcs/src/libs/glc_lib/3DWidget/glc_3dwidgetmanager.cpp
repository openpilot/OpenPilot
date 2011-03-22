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
//! \file glc_3dwidgetmanager.cpp Implementation of the GLC_3DWidgetManager class.

#include "glc_3dwidgetmanager.h"

GLC_3DWidgetManager::GLC_3DWidgetManager(GLC_Viewport* pViewport)
:m_pWidgetManagerHandle(new GLC_3DWidgetManagerHandle(pViewport))
{

}

GLC_3DWidgetManager::GLC_3DWidgetManager(const GLC_3DWidgetManager& widgetManager)
: m_pWidgetManagerHandle(widgetManager.m_pWidgetManagerHandle)
{
	m_pWidgetManagerHandle->increment();
}

GLC_3DWidgetManager::~GLC_3DWidgetManager()
{
	m_pWidgetManagerHandle->decrement();
	if (m_pWidgetManagerHandle->isOrphan())
	{
		delete m_pWidgetManagerHandle;
	}
}
