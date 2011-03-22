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

//! \file glc_repmover.h Interface for the GLC_RepMover class.

#ifndef GLC_REPMOVER_H_
#define GLC_REPMOVER_H_

#include <QColor>
#include "../maths/glc_vector3d.h"
#include "../maths/glc_matrix4x4.h"
#include "../shading/glc_renderproperties.h"
#include "../glc_config.h"

class GLC_Viewport;

//////////////////////////////////////////////////////////////////////
//! \class GLC_RepMover
/*! \brief GLC_RepMover : Base class for all interactive manipulation representation*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_RepMover
{
public:
	struct RepMoverInfo
	{
		QVector<GLC_Matrix4x4> m_MatrixInfo;
		QVector<GLC_Vector3d> m_VectorInfo;
		QVector<double> m_DoubleInfo;
		QVector<int> m_IntInfo;
		QVector<QString> m_StringInfo;
	};

public:
	//! Default constructor
	GLC_RepMover(GLC_Viewport*);

	//! Copy constructor
	GLC_RepMover(const GLC_RepMover&);

	//! Destructor
	virtual ~GLC_RepMover();


//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the main Color
	inline QColor mainColor()
	{return m_MainColor;}

	//! Return a clone of the repmover
	virtual GLC_RepMover* clone() const= 0;
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
	//! Set representation main color
	virtual void setMainColor(const QColor& color);

	//! Set representation wire thickness
	virtual void setThickness(double thickness);

	//! Init the representation
	virtual void init(){}

	//! Update the representation
	virtual void update(){}

	//! Set the repMoverInfo of this rep
	void setRepMoverInfo(RepMoverInfo* pRepMoverInfo);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Representation OpenGL Execution
	void render();

protected:
	//! Virtual interface for OpenGL Geometry set up.
	virtual void glDraw()= 0;

//@}

//////////////////////////////////////////////////////////////////////
// Protected Members
//////////////////////////////////////////////////////////////////////
protected:
	//! The viewport
	GLC_Viewport* m_pViewport;

	//! The rep main color
	QColor m_MainColor;

	//! The rep wire thickness
	double m_Thickness;

	//! The rep rendering properties
	GLC_RenderProperties m_RenderProperties;

	//! The repmover info of this rep
	RepMoverInfo* m_pRepMoverInfo;
};

#endif /* GLC_REPMOVER_H_ */
