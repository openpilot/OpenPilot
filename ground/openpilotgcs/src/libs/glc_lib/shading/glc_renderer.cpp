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

//! \file glc_renderer.cpp implementation of the GLC_Renderer class.

#include <QtDebug>

#include "../sceneGraph/glc_3dviewcollection.h"

#include "glc_renderer.h"

GLC_Renderer::GLC_Renderer()
: m_pCollection(NULL)
, m_IdToRenderProperties()
, m_IsCurrent(false)
{

}

GLC_Renderer::GLC_Renderer(GLC_3DViewCollection* pCollection)
: m_pCollection(pCollection)
, m_IdToRenderProperties()
, m_IsCurrent(false)
{

}

GLC_Renderer::GLC_Renderer(const GLC_Renderer& other)
: m_pCollection(other.m_pCollection)
, m_IdToRenderProperties(other.m_IdToRenderProperties)
, m_IsCurrent(false)
{

}

GLC_Renderer::~GLC_Renderer()
{
	clear();
}

bool GLC_Renderer::instanceRenderPropertiesIsAvailable(GLC_uint id) const
{
	return m_IdToRenderProperties.contains(id);
}

const GLC_RenderProperties& GLC_Renderer::renderPropertiesOfInstance(GLC_uint id) const
{
	Q_ASSERT(m_IdToRenderProperties.contains(id));
	return m_IdToRenderProperties.find(id).value();
}

void GLC_Renderer::clear()
{
	m_pCollection= NULL;
	m_IdToRenderProperties.clear();
}

GLC_Renderer& GLC_Renderer::operator=(const GLC_Renderer& other)
{
	if (this != &other)
	{
		m_pCollection= other.m_pCollection;
		m_IdToRenderProperties= other.m_IdToRenderProperties;
        // m_IsCurrent doesn't change
	}

	return *this;
}

void GLC_Renderer::setCollection(GLC_3DViewCollection* pCollection)
{
	if (pCollection != m_pCollection)
	{
		clear();
		m_pCollection= pCollection;
	}
}

void GLC_Renderer::setCurrent()
{
	if (NULL != m_pCollection)
	{
        Q_ASSERT(!m_IsCurrent);
        m_IsCurrent= true;
		QHash<GLC_uint, GLC_RenderProperties>::const_iterator iRender= m_IdToRenderProperties.constBegin();
		while (iRender != m_IdToRenderProperties.constEnd())
		{
			if (m_pCollection->contains(iRender.key()))
			{
				m_pCollection->instanceHandle(iRender.key())->renderPropertiesHandle()->operator =(iRender.value());
			}
			++iRender;
		}
		m_IdToRenderProperties.clear();
	}
}

void GLC_Renderer::unSetCurrent()
{
	if (NULL != m_pCollection)
	{
        Q_ASSERT(m_IdToRenderProperties.isEmpty());
        Q_ASSERT(m_IsCurrent);
        m_IsCurrent= false;
		QList<GLC_3DViewInstance*> instances= m_pCollection->instancesHandle();
		const int count= instances.count();
		for (int i= 0; i < count; ++i)
		{
			GLC_3DViewInstance* pInstance= instances.at(i);
			m_IdToRenderProperties.insert(pInstance->id(), *(pInstance->renderPropertiesHandle()));
		}
	}
}

void GLC_Renderer::addRenderPropertiesOfInstanceId(GLC_uint id)
{
	Q_ASSERT(NULL != m_pCollection);
	Q_ASSERT(m_pCollection->contains(id));
	m_IdToRenderProperties.insert(id, *(m_pCollection->instanceHandle(id)->renderPropertiesHandle()));
}
