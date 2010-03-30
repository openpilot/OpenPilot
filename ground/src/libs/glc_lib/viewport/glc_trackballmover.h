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

#ifndef GLC_TRACKBALLMOVER_H_
#define GLC_TRACKBALLMOVER_H_

#include "glc_mover.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_TrackBallMover
/*! \brief GLC_TrackBallMover : Track ball interactive manipulation */
//////////////////////////////////////////////////////////////////////
class GLC_TrackBallMover : public GLC_Mover
{
public:
	//! Default constructor
	GLC_TrackBallMover(GLC_Viewport*, const QList<GLC_RepMover*>& repsList= QList<GLC_RepMover*>());

	//! Copy constructor
	GLC_TrackBallMover(const GLC_TrackBallMover&);

	//! Destructor
	virtual ~GLC_TrackBallMover();

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return a clone of the mover
	virtual GLC_Mover* clone() const;
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Initialized the mover
	virtual void init(int, int);

	//! Move the camera
	virtual void move(int, int);

//@}

/////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////
private:
	//! Convert mouse View coordinate to tracking coordinate (Centred and betwen (-1,-1) and (1,1))
	GLC_Vector4d mapForTracking( double , double) const;

//////////////////////////////////////////////////////////////////////
// Private Members
//////////////////////////////////////////////////////////////////////
private:
	//! The ratio of the trackball size
	double m_Ratio;


};

#endif /* GLC_TRACKBALLMOVER_H_ */
