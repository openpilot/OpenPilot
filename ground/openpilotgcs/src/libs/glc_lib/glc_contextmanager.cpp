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
//! \file glc_contextmanager.cpp implementation of the GLC_ContextManager class.

#include <QtDebug>

#include "glc_contextmanager.h"
#include "glc_state.h"

GLC_ContextManager* GLC_ContextManager::m_pContextManager= NULL;

GLC_ContextManager::GLC_ContextManager()
: m_pCurrentContext(NULL)
, m_SetOfContext()
{


}

GLC_ContextManager::~GLC_ContextManager()
{

}

//////////////////////////////////////////////////////////////////////
// Get Functions
//////////////////////////////////////////////////////////////////////
GLC_ContextManager* GLC_ContextManager::instance()
{
	if (NULL == m_pContextManager)
	{
		m_pContextManager= new GLC_ContextManager();
	}

	return m_pContextManager;
}

GLC_Context* GLC_ContextManager::currentContext() const
{
	return m_pCurrentContext;
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
void GLC_ContextManager::addContext(GLC_Context* pContext)
{
	Q_ASSERT(!m_SetOfContext.contains(pContext));
	m_SetOfContext.insert(pContext);
}

void GLC_ContextManager::remove(GLC_Context* pContext)
{
	Q_ASSERT(m_SetOfContext.contains(pContext));
	m_SetOfContext.remove(pContext);
	if (m_pCurrentContext == pContext)
	{
		m_pCurrentContext= NULL;
	}
}

void GLC_ContextManager::setCurrent(GLC_Context* pContext)
{

	Q_ASSERT((NULL == pContext) || m_SetOfContext.contains(pContext));
	m_pCurrentContext= pContext;
}


