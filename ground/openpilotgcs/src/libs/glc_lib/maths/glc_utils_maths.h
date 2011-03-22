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
//! \file glc_utils_maths.h Mathematic constants.
/*! \brief Definition of usefull constants*/

#ifndef GLC_UTILS_MATHS_H_
#define GLC_UTILS_MATHS_H_

// Standard C math library
#include <math.h>
namespace glc
{
	/*! \def EPSILON
	 *  \brief Define precison of comparaison*/

	const double EPSILON= 1e-10;

	/*! \def PI
	 * \brief Define the magic number PI */
	const double PI= acos(-1.0);

	//! Convert the given degre angle in radian
	inline double toRadian(double angle)
	{return PI * angle / 180.0;}
};

#endif /*GLC_UTILS_MATHS_H_*/
