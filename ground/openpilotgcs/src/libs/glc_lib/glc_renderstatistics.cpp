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
//! \file glc_renderstatistics.cpp implementation of the GLC_RenderStatistics class.

#include "glc_renderstatistics.h"

// Static variables initialisation
bool GLC_RenderStatistics::m_IsActivated= false;
unsigned int GLC_RenderStatistics::m_LastRenderGeometryCount= 0;
unsigned long GLC_RenderStatistics::m_LastRenderPolygonCount= 0;

GLC_RenderStatistics::GLC_RenderStatistics()
{


}

GLC_RenderStatistics::~GLC_RenderStatistics()
{

}

//////////////////////////////////////////////////////////////////////
// Get methods
//////////////////////////////////////////////////////////////////////
bool GLC_RenderStatistics::activated()
{
	return m_IsActivated;
}

unsigned int GLC_RenderStatistics::bodyCount()
{
	return m_LastRenderGeometryCount;
}

unsigned long GLC_RenderStatistics::triangleCount()
{
	return m_LastRenderPolygonCount;
}

//////////////////////////////////////////////////////////////////////
// Set methods
//////////////////////////////////////////////////////////////////////
void GLC_RenderStatistics::setActivationFlag(bool flag)
{
	m_IsActivated= flag;
}

void GLC_RenderStatistics::reset()
{
	m_LastRenderGeometryCount= 0;
	m_LastRenderPolygonCount= 0;
}

void GLC_RenderStatistics::addBodies(unsigned int bodies)
{
	if (m_IsActivated)
	{
		m_LastRenderGeometryCount+= bodies;
	}
}

void GLC_RenderStatistics::addTriangles(unsigned int triangles)
{
	if (m_IsActivated)
	{
		m_LastRenderPolygonCount+= triangles;
	}
}
