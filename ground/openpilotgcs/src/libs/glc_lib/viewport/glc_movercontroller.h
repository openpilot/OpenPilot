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

//! \file glc_movercontroller.h Interface for the GLC_MoverController class.

#ifndef GLC_MOVERCONTROLLER_H_
#define GLC_MOVERCONTROLLER_H_

#include "glc_mover.h"
#include <QObject>
#include <QHash>
#include <QtDebug>

#include "../glc_config.h"
#include "glc_userinput.h"

class QGLWidget;

//////////////////////////////////////////////////////////////////////
//! \class GLC_MoverController
/*! \brief GLC_MoverController : Control activation of interactive manipulation mover */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_MoverController : public QObject
{
	Q_OBJECT
public:
	//! The mover hash table
	typedef QHash<const int, GLC_Mover*> MoverHash;

	//! Standard mover Id
	enum MoverType
	{
		Pan= 1,
		Zoom= 2,
		TrackBall= 3,
		Target= 4,
		TurnTable= 5,
		Fly= 6,
		TSR= 7
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

	//! Return a handle on the current mover
	inline GLC_Mover* activeMover() const
	{return m_MoverHash.value(m_ActiveMoverId);}

	//! Return the mover of the given id
	inline GLC_Mover* getMover(const int id) const
	{return m_MoverHash.value(id);}

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
	void setActiveMover(const int id, const GLC_UserInput& userInput);

	//! Set no mover as active
	void setNoMover();

	//! Move with the active mover
	inline bool move(const GLC_UserInput& userInput)
	{
		Q_ASSERT(0 != m_ActiveMoverId);
		return m_MoverHash.value(m_ActiveMoverId)->move(userInput);
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
			m_MoverHash.value(m_ActiveMoverId)->renderRepresentation();
		}
	}

//@}
signals:
	//! Signal emitted if the view as to be repaint
	void repaintNeeded();

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
