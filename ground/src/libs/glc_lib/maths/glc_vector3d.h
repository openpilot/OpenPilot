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

//! \file glc_vector3d.h interface for the GLC_Vector3d class.

#ifndef GLC_VECTOR3D_H_
#define GLC_VECTOR3D_H_

#include "glc_utils_maths.h"
#include "glc_vector3df.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Vector3d
/*! \brief GLC_Vector3d is a 3 dimensions Vector*/

/*! GLC_Vector3d is used to represent 3D position and vectors.
 * */
//////////////////////////////////////////////////////////////////////

class GLC_Vector3d
{
	friend class GLC_Vector4d;
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	/*! Default constructor
	*  Value is set to
	* \n X = 0.0
	* \n Y =  0.0
	* \n Z =  0.0
	*/
	inline GLC_Vector3d()
	{
		vector[0]= 0.0;
		vector[1]= 0.0;
		vector[2]= 0.0;
	}

	/*! Standard constructor With x, y, z*/
	inline GLC_Vector3d(const double &dX, const double &dY, const double &dZ)
	{
		vector[0]= dX;
		vector[1]= dY;
		vector[2]= dZ;
	}

	/*! Copy constructor
	 * Sample use
	 * \code
	 * NewVect = new GLC_Vector3d(OldVect);
	 * \endcode
	 */
	inline GLC_Vector3d(const GLC_Vector3d &Vect)
	{
		vector[0]= Vect.vector[0];
		vector[1]= Vect.vector[1];
		vector[2]= Vect.vector[2];
	}
	/*! Copy constructor from a float vector
	 * Sample use
	 * \code
	 * NewVect = new GLC_Vector3d(OldVectf);
	 * \endcode
	 */
	inline GLC_Vector3d(const GLC_Vector3df &Vect)
	{
		vector[0]= static_cast<double>(Vect.dVector[0]);
		vector[1]= static_cast<double>(Vect.dVector[1]);
		vector[2]= static_cast<double>(Vect.dVector[2]);
	}

//@}


//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! X Compound
	inline GLC_Vector3d& setX(const double &dX)
	{
		vector[0]= dX;
		return *this;
	}

	//! Y Compound
	inline GLC_Vector3d& setY(const double &dY)
	{
		vector[1]= dY;
		return *this;
	}

	//! Z Compound
	inline GLC_Vector3d& setZ(const double &dZ)
	{
		vector[2]= dZ;
		return *this;
	}

	//! All Compound
	inline GLC_Vector3d& setVect(const double &dX, const double &dY, const double &dZ)
	{
		vector[0]= dX;
		vector[1]= dY;
		vector[2]= dZ;

		return *this;
	}

	//! From another Vector
	GLC_Vector3d& setVect(const GLC_Vector3d &Vect)
	{
		vector[0]= Vect.vector[0];
		vector[1]= Vect.vector[1];
		vector[2]= Vect.vector[2];
		return *this;
	}

	//! Invert Vector
	inline GLC_Vector3d& setInv(void)
	{
		vector[0]= - vector[0];
		vector[1]= - vector[1];
		vector[2]= - vector[2];
		return *this;
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return X Compound
	inline double X(void) const
	{
		return vector[0];
	}
	//! Return Y Compound
	inline double Y(void) const
	{
		return vector[1];
	}
	//! Return Z Compound
	inline double Z(void) const
	{
		return vector[2];
	}
	//! Return a pointer to vector data
	inline const double *data(void) const
	{
		return vector;
	}
	//! Return true if the vector is null
	inline bool isNull(void) const
	{
		return qFuzzyCompare(vector[0], 0.0) and qFuzzyCompare(vector[1], 0.0)
		and qFuzzyCompare(vector[2], 0.0);
	}

//@}

//////////////////////////////////////////////////////////////////////
//name Private attributes
//////////////////////////////////////////////////////////////////////
private:
	/*! Vector array definition \n
	*	vector[0]	X \n
	*	vector[1]	Y \n
	*	vector[2]	Z \n
	*/
	double vector[3];

}; //class GLC_Vector3d

//! Define GLC_Point3D
typedef GLC_Vector3d GLC_Point3d;

#endif /*GLC_VECTOR3D_H_*/
