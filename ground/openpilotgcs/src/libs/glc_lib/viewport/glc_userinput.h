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

//! \file glc_userinput.h Interface for the GLC_UserInput class.

#ifndef GLC_USERINPUT_H_
#define GLC_USERINPUT_H_

#include "../maths/glc_vector2d.h"

#include "../glc_config.h"

class GLC_LIB_EXPORT GLC_UserInput
{
public:
	GLC_UserInput(int x= 0, int y= 0);
	virtual ~GLC_UserInput();


//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! return the x position
	inline int x() const
	{return m_X;}

	//! Return the y position
	inline int y() const
	{return m_Y;}

	//! Return normalyze x touch center
	inline double normalyzeXTouchCenter() const
	{return m_NormalyzeX;}

	//! Return normalyze x touch center
	inline double normalyzeYTouchCenter() const
	{return m_NormalyzeX;}

	//! Return the translation
	inline GLC_Vector2d translation() const
	{return m_Translation;}

	//! Return the rotation angle
	inline double rotationAngle() const
	{return m_Rotation;}

	//! Return the scale factor
	inline double scaleFactor() const
	{return m_ScaleFactor;}

	//! Return true if the transformation has been set
	inline bool transformationIsSet() const
	{return m_TransformationIsSet;}
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set the position
	void setPosition(int x, int y);

	//! Set the normalyze position of the center of touch
	void setNormalyzeTouchCenterPosition(double x, double y);

	//! Set the transformation
	void setTransformation(const GLC_Vector2d& translation, double rotation= 0.0, double scaleFactor= 1.0);

	//! Set translation
	inline void setTranslation(const GLC_Vector2d& translation)
	{m_Translation= translation;}

	//! Set rotation
	inline void setRotation(double rotation)
	{m_Rotation= rotation;}

	//! Set scaling
	inline void setScaleFactor(double scaleFactor)
	{m_ScaleFactor= scaleFactor;}
//@}

//////////////////////////////////////////////////////////////////////
// Private Members
//////////////////////////////////////////////////////////////////////
private:
	//! the x position of the user input
	int m_X;

	//! The y position of the user input
	int m_Y;

	//Normalize position of center of touchs
	double m_NormalyzeX;
	double m_NormalyzeY;

	// Transformation data
	//! Translation vector
	GLC_Vector2d m_Translation;

	//! Rotation angle
	double m_Rotation;

	//! Scale factor
	double m_ScaleFactor;

	//! Flag to know if a transformation has been set
	bool m_TransformationIsSet;

};

#endif /* GLC_USERINPUT_H_ */
