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

//! \file glc_vector3df.h interface for the GLC_Vector3df class.

#ifndef GLC_VECTOR3DF_H_
#define GLC_VECTOR3DF_H_
#include "glc_utils_maths.h"

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Vector3df
/*! \brief GLC_Vector3df is a 3 dimensions Vector*/

/*! GLC_Vector3df is used to represent 3D position and vectors.
 * */
//////////////////////////////////////////////////////////////////////

class GLC_LIB_EXPORT GLC_Vector3df
{
	friend class GLC_Vector4d;
	friend class GLC_Vector3d;
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
	inline GLC_Vector3df()
	{
		m_Vector[0]= 0.0f;
		m_Vector[1]= 0.0f;
		m_Vector[2]= 0.0f;
	}

	//! Standard constructor With x, y, z
	inline GLC_Vector3df(const float &dX, const float &dY, const float &dZ)
	{
		setVect(dX, dY, dZ);
	}

	/*! Copy constructor
	 * Sample use
	 * \code
	 * NewVect = new GLC_Vector3d(OldVect);
	 * \endcode
	 */
	inline GLC_Vector3df(const GLC_Vector3df &Vect)
	{
		m_Vector[0]= Vect.m_Vector[0];
		m_Vector[1]= Vect.m_Vector[1];
		m_Vector[2]= Vect.m_Vector[2];
	}
//@}


//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! X Compound
	inline GLC_Vector3df& setX(const float &dX)
	{
		m_Vector[0]= dX;
		return *this;
	}

	//! Y Compound
	inline GLC_Vector3df& setY(const float &dY)
	{
		m_Vector[1]= dY;
		return *this;
	}

	//! Z Compound
	inline GLC_Vector3df& setZ(const float &dZ)
	{
		m_Vector[2]= dZ;
		return *this;
	}

	//! All Compound
	inline GLC_Vector3df& setVect(const float &dX, const float &dY, const float &dZ)
	{
		m_Vector[0]= dX;
		m_Vector[1]= dY;
		m_Vector[2]= dZ;

		return *this;
	}

	//! From another Vector
	GLC_Vector3df& setVect(const GLC_Vector3df &Vect)
	{
		m_Vector[0]= Vect.m_Vector[0];
		m_Vector[1]= Vect.m_Vector[1];
		m_Vector[2]= Vect.m_Vector[2];
		return *this;
	}

	//! Invert Vector
	inline GLC_Vector3df& setInv(void)
	{
		m_Vector[0]= - m_Vector[0];
		m_Vector[1]= - m_Vector[1];
		m_Vector[2]= - m_Vector[2];
		return *this;
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return X Compound
	inline float x(void) const
	{
		return m_Vector[0];
	}
	//! Return Y Compound
	inline float y(void) const
	{
		return m_Vector[1];
	}
	//! Return Z Compound
	inline float z(void) const
	{
		return m_Vector[2];
	}
	//! Return a pointer to vector data
	inline const float *data(void) const
	{
		return m_Vector;
	}
	//! Return true if the vector is null
	inline bool isNull(void) const
	{
		return qFuzzyCompare(m_Vector[0], 0.0f) && qFuzzyCompare(m_Vector[1], 0.0f)
		&& qFuzzyCompare(m_Vector[2], 0.0f);
	}

//@}

//////////////////////////////////////////////////////////////////////
//name Private attributes
//////////////////////////////////////////////////////////////////////
private:
	/*! Vector array definition \n
	*	data[0]	X \n
	*	data[1]	Y \n
	*	data[2]	Z \n
	*/
	float m_Vector[3];

}; //class GLC_Vector3d

//! Define GLC_Point3D
typedef GLC_Vector3df GLC_Point3df;

#endif /*GLC_VECTOR3DF_H_*/
