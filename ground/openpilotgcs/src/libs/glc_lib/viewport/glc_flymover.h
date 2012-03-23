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
//! \file glc_flymover.h Interface for the GLC_FlyMover class.

#ifndef GLC_FLYMOVER_H_
#define GLC_FLYMOVER_H_
#include "glc_mover.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_FlyMover
/*! \brief GLC_FlyMover : Fly Mode interactive manipulation */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_FlyMover : public GLC_Mover
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_FlyMover(GLC_Viewport*, const QList<GLC_RepMover*>& repsList= QList<GLC_RepMover*>());

	//! Copy constructor
	GLC_FlyMover(const GLC_FlyMover& flyMover);

	//! Destructor
	virtual ~GLC_FlyMover();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return a clone of the mover
	virtual GLC_Mover* clone() const;

	//! Return the turning rate in degres
	inline double turningRate() const
	{return m_TurnRate / glc::PI * 180.0;}

	//! Return the flying velocity
	inline double flyingVelocity() const
	{return m_Velocity;}
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Initialized the mover
	virtual void init(const GLC_UserInput& userInput);

	//! Move the camera
	virtual bool move(const GLC_UserInput& userInput);

	//! Ends this mover
	virtual void ends();

	//! Set the maximum turning rate in degre
	inline void setMaximumTurnRate(double turnRate)
	{m_TurnRate= turnRate;}

	//! Set the flying velocity
	void setFlyingVelocity(double velocity);

	//! increase the flying velocity
	void increaseVelocity(double factor);

//@}

protected:
    void timerEvent(QTimerEvent*);

/////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
private:
	//! Map the position of mouse for the fly mode
	GLC_Vector3d mapForFlying(double x, double y);

	//! Fly
	void fly();

//////////////////////////////////////////////////////////////////////
// Private Members
//////////////////////////////////////////////////////////////////////
private:
	//! THe turning rate
	double m_TurnRate;

	//! The timer id
	int m_TimerId;

	//! the timer interval
	int m_TimerInterval;

	//! fly velocity
	double m_Velocity;

};

#endif /* GLC_FLYMOVER_H_ */
