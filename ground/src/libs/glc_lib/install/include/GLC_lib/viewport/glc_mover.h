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

//! \file glc_mover.h Interface for the GLC_Mover class.

#ifndef GLC_MOVER_H_
#define GLC_MOVER_H_

#include "glc_repmover.h"
#include "../maths/glc_vector4d.h"

#include <QList>

class GLC_Viewport;

//////////////////////////////////////////////////////////////////////
//! \class GLC_Mover
/*! \brief GLC_Mover : Base class for all interactive manipulation */
//////////////////////////////////////////////////////////////////////
class GLC_Mover
{
public:
	//! Default constructor
	GLC_Mover(GLC_Viewport*, const QList<GLC_RepMover*>&);

	//! Copy constructor
	GLC_Mover(const GLC_Mover&);

	//! Destructor
	virtual ~GLC_Mover();


//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return a clone of the mover
	virtual GLC_Mover* clone() const= 0;
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Initialized the mover
	virtual void init(int, int)= 0;

	//! Move the camera
	virtual void move(int, int)= 0;

	//! Set the mover representation list
	void setRepresentationsList(const QList<GLC_RepMover*>&);

	//! Init representation
	void initRepresentation(const GLC_Vector4d&, const GLC_Matrix4x4&);

	//! Update representation
	void updateRepresentation(const GLC_Matrix4x4&);


//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Mover representations list display
	void glExecute();

//@}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
private:
	//! Clear mover representation
	void clearMoverRepresentation();

//////////////////////////////////////////////////////////////////////
// Private Members
//////////////////////////////////////////////////////////////////////
private:
	//! The mover representations list
	QList<GLC_RepMover*> m_RepMoverList;

//////////////////////////////////////////////////////////////////////
// Protected Members
//////////////////////////////////////////////////////////////////////
protected:

	//! The previous mover value
	GLC_Vector4d m_PreviousVector;

	//! The Viewport
	GLC_Viewport* m_pViewport;
};

#endif /* GLC_MOVER_H_ */
