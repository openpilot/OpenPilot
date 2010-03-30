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

//! \file glc_repmover.h Interface for the GLC_RepMover class.

#ifndef GLC_REPMOVER_H_
#define GLC_REPMOVER_H_

#include <QColor>
#include "../maths/glc_vector4d.h"
#include "../maths/glc_matrix4x4.h"


class GLC_Viewport;

//////////////////////////////////////////////////////////////////////
//! \class GLC_RepMover
/*! \brief GLC_RepMover : Base class for all interactive manipulation representation*/
//////////////////////////////////////////////////////////////////////
class GLC_RepMover
{
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
	virtual void setMainColor(const QColor& color)
	{m_MainColor= color;}

	//! Init the representation
	virtual void init(const GLC_Vector4d&, const GLC_Matrix4x4&){}

	//! Init the representation
	virtual void update(const GLC_Matrix4x4&){}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Representation OpenGL Execution
	void glExecute();

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
};

#endif /* GLC_REPMOVER_H_ */
