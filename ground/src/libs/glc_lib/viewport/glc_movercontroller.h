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

//! \file glc_movercontroller.h Interface for the GLC_MoverController class.

#ifndef GLC_MOVERCONTROLLER_H_
#define GLC_MOVERCONTROLLER_H_

#include "glc_mover.h"
#include <QHash>
#include <QtDebug>

//////////////////////////////////////////////////////////////////////
//! \class GLC_MoverController
/*! \brief GLC_MoverController : Control activation of interactive manipulation mover */
//////////////////////////////////////////////////////////////////////
class GLC_MoverController
{
public:
	//! The mover hash table
	typedef QHash<const int, GLC_Mover*> MoverHash;

	//! Standard mover Id
	enum
	{
		Pan= 1,
		Zoom= 2,
		TrackBall= 3,
		Target= 4,
		TurnTable= 5
	};

public:
	//! Default Constructor
	GLC_MoverController();

	//! Copy Constructor
	GLC_MoverController(const GLC_MoverController&);

	//! Destructor
	virtual ~GLC_MoverController();

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Return true if there is an active mover
	inline bool hasActiveMover() const
	{ return (m_ActiveMoverId != 0);}

	//! Return the active mover id
	inline int activeMoverId() const
	{return m_ActiveMoverId;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Assign another mover controller
	GLC_MoverController& operator = (const GLC_MoverController&);

	//! Add a mover to the controller
	void addMover(GLC_Mover*, const int);

	//! Remove mover from the controller
	void removeMover(const int);

	//! Set the specified mover as active
	inline void setActiveMover(const int id, int x, int y)
	{
		Q_ASSERT(m_MoverHash.contains(id));
		m_ActiveMoverId= id;
		m_MoverHash.value(m_ActiveMoverId)->init(x, y);
	}

	//! Set no mover as active
	inline void setNoMover()
	{ m_ActiveMoverId= 0;}

	//! Move with the active mover
	inline void move(int x, int y)
	{
		Q_ASSERT(0 != m_ActiveMoverId);
		m_MoverHash.value(m_ActiveMoverId)->move(x, y);
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Mover representations list display
	inline void drawActiveMoverRep()
	{
		if(0 != m_ActiveMoverId)
		{
			m_MoverHash.value(m_ActiveMoverId)->glExecute();
		}
	}

//@}

//////////////////////////////////////////////////////////////////////
// Private Members
//////////////////////////////////////////////////////////////////////
private:
	//! The active mover id
	int m_ActiveMoverId;

	//! Hash table of mover
	MoverHash m_MoverHash;
};

#endif /* GLC_MOVERCONTROLLER_H_ */
