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
//! \file glc_3dwidget.h Interface for the GLC_3DWidget class.

#ifndef GLC_3DWIDGET_H_
#define GLC_3DWIDGET_H_
#include <QObject>
#include <QList>
#include "../glc_config.h"
#include "../glc_global.h"
#include "glc_3dwidgetmanagerhandle.h"

class GLC_3DViewInstance;

//////////////////////////////////////////////////////////////////////
//! \class GLC_3DWidget
/*! \brief GLC_3DWidget : The base class for 3D user interface class */

/*! GLC_3DWidget  The 3D widget has a 3d representation and react on user
 * interactions*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_3DWidget : public QObject
{
	Q_OBJECT

	typedef QList<GLC_uint> InstanceIdList;

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct a 3d widget with the given 3DWidget manager handle
	GLC_3DWidget(GLC_3DWidgetManagerHandle*  pWidgetManagerHandle= NULL);

	//! Construct a 3d widget form the given 3d widget
	GLC_3DWidget(const GLC_3DWidget& widget);

	//! Destructor
	virtual ~GLC_3DWidget();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return this widget id
	inline GLC_uint id() const
	{return m_Uid;}

	//! Return true if this 3d widget is equal to the given 3d widget
	bool operator==(const GLC_3DWidget& widget) const;

	//! Return true if the given instance id belongs to this widget
	inline bool instanceBelongTo(GLC_uint id) const
	{return m_InstanceIdList.contains(id);}

	//! Return the widget manager of this 3d widget
	inline const GLC_3DWidgetManagerHandle* widgetManagerHandle() const
	{return m_pWidgetManagerHandle;}

	//! Return true if this widget has a 3DWidgetManager
	inline bool has3DWidgetManager() const
	{return (NULL == m_pWidgetManagerHandle);}

	//! Return true if otho is used
	inline bool useOrtho() const
	{return m_pWidgetManagerHandle->useOrtho();}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Copy the given widget in this widget and return a reference on this widget
	virtual GLC_3DWidget& operator=(const GLC_3DWidget& widget);

	//! Set the given widgetManager to this widget
	void setWidgetManager(GLC_3DWidgetManagerHandle* pWidgetManagerHandle);

	//! Update widget representation
	virtual void updateWidgetRep(){};

	//! Set the visibility of 3DView Instance of this widget
	void setVisible(bool visible);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Interaction Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! This widget as been selected
	virtual glc::WidgetEventFlag select(const GLC_Point3d&, GLC_uint id);

	//! This widget as been unselected
	virtual glc::WidgetEventFlag unselect(const GLC_Point3d&, GLC_uint id);

	//! The mouse is over this widget
	virtual glc::WidgetEventFlag mouseOver(const GLC_Point3d&, GLC_uint id);

	//! The mouse is over this widget and a mousse button is pressed
	virtual glc::WidgetEventFlag mousePressed(const GLC_Point3d&, Qt::MouseButton, GLC_uint id);

	//! The mouse is over this widget and a mousse button is released
	virtual glc::WidgetEventFlag mouseReleased(Qt::MouseButton);

	//! This widget is selected and the mousse move with a pressed buttons
	virtual glc::WidgetEventFlag mouseMove(const GLC_Point3d&, Qt::MouseButtons, GLC_uint id);

//@}

signals:
	//! Sub class must emit this signal if they changed
	void asChanged();

//////////////////////////////////////////////////////////////////////
// Protected services function
//////////////////////////////////////////////////////////////////////
protected:
	//! Create the 3DView instance of this 3d widget
	virtual void create3DviewInstance()= 0;

	//! Return true if this 3DWidget hasen't 3d instance
	inline bool isEmpty() const
	{return m_InstanceIdList.isEmpty();}

	//! Add 3D view instance in the widget manager handle
	void add3DViewInstance(const GLC_3DViewInstance& instance);

	//! Return the 3D view instance handle from the given index
	inline GLC_3DViewInstance* instanceHandle(int index)
	{return m_pWidgetManagerHandle->instanceHandle(m_InstanceIdList.at(index));}

	//! Remove instance of this 3d widget from the 3D widget manager handle
	void remove3DViewInstance();

	//! Set the specified 3D view instance visibility
	void set3DViewInstanceVisibility(int index, bool visibility);

	//! Return the index of the given instance id
	inline int indexOfIntsanceId(GLC_uint id)
	{return m_InstanceIdList.indexOf(id);}

	//! Reset the view state of this 3DWidget
	virtual void resetViewState()= 0;

//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////
private:

//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////
private:
	//! The id of this 3d widget
	/*! Generated on GLC_3DWidget construction*/
	GLC_uint m_Uid;

	//! Pointer to this widget manager handle
	GLC_3DWidgetManagerHandle* m_pWidgetManagerHandle;

	//! The List of this widget instance id
	QList<GLC_uint> m_InstanceIdList;

};

#endif /* GLC_3DWIDGET_H_ */
