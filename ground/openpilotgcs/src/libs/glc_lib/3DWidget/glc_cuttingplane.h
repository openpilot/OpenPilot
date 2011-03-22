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
//! \file glc_cuttingplane.h Interface for the GLC_CuttingPlane class.

#ifndef GLC_CUTTINGPLANE_H_
#define GLC_CUTTINGPLANE_H_

#include "glc_3dwidget.h"
#include "../glc_config.h"

class GLC_AbstractManipulator;

//////////////////////////////////////////////////////////////////////
//! \class GLC_CuttingPlane
/*! \brief GLC_CuttingPlane :  3d cutting plane widget*/

/*! GLC_CuttingPlane */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_CuttingPlane : public GLC_3DWidget
{
	enum Manipulator
	{
		TranslationManipulator,
		RotationManipulator
	};
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct a 3d cutting plane widget
	GLC_CuttingPlane(const GLC_Point3d& center, const GLC_Vector3d& normal, double l1, double l2, GLC_3DWidgetManagerHandle*  pWidgetManagerHandle= NULL);

	//! Construct a 3d cutting plane with the given cutting plane
	GLC_CuttingPlane(const GLC_CuttingPlane& cuttingPlane);

	//! Destructor
	virtual ~GLC_CuttingPlane();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return this cutting plane center
	inline GLC_Point3d center() const
	{return m_Center;}

	//! Return this cutting plane normal
	inline GLC_Vector3d normal() const
	{return m_Normal;}

	//! Return this plane color
	inline QColor color() const
	{return m_Color;}

	//! Return this plane opacity
	inline double opacity() const
	{return m_Opacity;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Copy the given cutting plane in this cutting plane and return a reference on this cutting plane
	virtual GLC_CuttingPlane& operator=(const GLC_CuttingPlane& cuttingPlane);

	//! Update the lenght of this cutting plane
	void updateLength(double l1, double l2);

	//! Set this plane color
	inline void setColor(const QColor& color)
	{m_Color= color;}

	//! Set this plane opacity
	inline void setOpacity(double opacity)
	{m_Opacity= opacity;}

	//! Update widget representation
	virtual void updateWidgetRep();

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

	//! The mouse is over this widget and a mousse button is pressed
	virtual glc::WidgetEventFlag mousePressed(const GLC_Point3d&, Qt::MouseButton, GLC_uint id);

	//! The mouse is over this widget and a mousse button is released
	virtual glc::WidgetEventFlag mouseReleased(Qt::MouseButton);

	//! This widget is selected and the mousse move with a pressed buttons
	virtual glc::WidgetEventFlag mouseMove(const GLC_Point3d&, Qt::MouseButtons, GLC_uint id);

//@}

//////////////////////////////////////////////////////////////////////
// Protected services function
//////////////////////////////////////////////////////////////////////
protected:
	//! Create the 3DView instance of this 3d widget
	virtual void create3DviewInstance();

	//! Reset the view state of this 3DWidget
	virtual void resetViewState();

//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////
private:
	//! Move the manipulator 3D representation
	void moveManipulatorRep(const GLC_Point3d& pos);

	//! Create the rotation navigator of the given instance index
	GLC_AbstractManipulator* rotationNavigator(int index);

//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////
private:
	//! The cutting plane center
	GLC_Point3d m_Center;

	//! The cutting plane Normal
	GLC_Vector3d m_Normal;

	//! The cutting plane compostion matrix
	GLC_Matrix4x4 m_CompMatrix;

	//! The cutting plane size
	double m_L1, m_L2;

	//! The cutting plane color
	QColor m_Color;

	//! The cutting plane opacity
	double m_Opacity;

	//! The manipulator offset
	double m_ManipulatorOffsetFactor;

	//! The manipulator scale factor
	double m_ScaleFactor;

	//! Index of the instance in selection
	int m_SelectionIndex;

	//! current manipulator enum
	Manipulator m_CurrentManipulator;

	//! The current manipulator of this cutting plane
	GLC_AbstractManipulator* m_pCurrentManipulator;

	//! The current manipulator position
	GLC_Point3d m_CurrentNavigatorPosition;


};

#endif /* GLC_CUTTINGPLANE_H_ */
