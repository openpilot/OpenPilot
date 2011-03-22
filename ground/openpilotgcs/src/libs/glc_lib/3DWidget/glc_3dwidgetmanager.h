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
//! \file glc_3dwidgetmanager.h Interface for the GLC_3DWidgetManager class.

#ifndef GLC_3DWIDGETMANAGER_H_
#define GLC_3DWIDGETMANAGER_H_

#include "glc_3dwidgetmanagerhandle.h"

#include "../glc_config.h"

class QMouseEvent;

class GLC_LIB_EXPORT GLC_3DWidgetManager
{

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Construct the 3d widget manager associted to the given Viewport
	GLC_3DWidgetManager(GLC_Viewport* pViewport);

	//! Construct a 3d widget manager from the given 3D widget manager
	GLC_3DWidgetManager(const GLC_3DWidgetManager& widgetManager);

	//! Destructor
	~GLC_3DWidgetManager();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the boundingBox of 3dwidget
	inline GLC_BoundingBox boundingBox()
	{return m_pWidgetManagerHandle->boundingBox();}

	//! Return true if this manager contains the given widget id
	inline bool contains3DWidget(GLC_uint id) const
	{return m_pWidgetManagerHandle->contains3DWidget(id);}

	//! Return the 3DWidget of the given widget id
	inline GLC_3DWidget* widget(GLC_uint id) const
	{return m_pWidgetManagerHandle->widget(id);}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Add the given 3D widget into this manager
	inline void add3DWidget(GLC_3DWidget* p3DWidget)
	{m_pWidgetManagerHandle->add3DWidget(p3DWidget);}

	//! Remove the 3D widget with the given id from this manager
	/*! Associated 3D view instance are removed*/
	inline void remove3DWidget(GLC_uint id)
	{m_pWidgetManagerHandle->remove3DWidget(id);}

	//! Clear all widget from this manager
	inline void clear()
	{m_pWidgetManagerHandle->clear();}

	//! Set the visibility of the given 3D widget id
	inline void setWidgetVisible(GLC_uint id, bool visible)
	{m_pWidgetManagerHandle->setWidgetVisible(id, visible);}

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Interaction Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Recieve Mouse double click event with the given instance id Return true if the event is catch
	inline glc::WidgetEventFlag mouseDoubleClickEvent(QMouseEvent * pEvent)
	{return m_pWidgetManagerHandle->mouseDoubleClickEvent(pEvent);}

	//! Recieve Mouse move event with the given instance id Return true if the event is catch
	inline glc::WidgetEventFlag mouseMoveEvent(QMouseEvent * pEvent)
	{return m_pWidgetManagerHandle->mouseMoveEvent(pEvent);}

	//! Recieve Mouse press event with the given instance id Return true if the event is catch
	inline glc::WidgetEventFlag mousePressEvent(QMouseEvent * pEvent)
	{return m_pWidgetManagerHandle->mousePressEvent(pEvent);}

	//! Recieve Mouse release event with the given instance id Return true if the event is catch
	inline glc::WidgetEventFlag mouseReleaseEvent(QMouseEvent * pEvent)
	{return m_pWidgetManagerHandle->mouseReleaseEvent(pEvent);}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Render the 3DWidget of this manager
	inline void render()
	{m_pWidgetManagerHandle->render();}

//@}

//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////
private:
	//! The widget manager handle
	GLC_3DWidgetManagerHandle* m_pWidgetManagerHandle;

};

#endif /* GLC_3DWIDGETMANAGER_H_ */
