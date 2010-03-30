/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Copyright (C) 2009 Pierre Soetewey
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

#ifndef GLC_DISTANCE_H_
#define GLC_DISTANCE_H_

#include <QList>
#include "../glc_object.h"
#include "../sceneGraph/glc_3dviewinstance.h"

//#include <../PQP/PQP.h>

class GLC_ExtendedMesh;
class PQP_Model;

//////////////////////////////////////////////////////////////////////
//! \class GLC_Distance
/*! \brief GLC_Distance is the minimum distance between to elements*/

/*! GLC_Distance is used to compute the minimum distance between to Elements.
 */
//////////////////////////////////////////////////////////////////////
class GLC_Distance : public GLC_Object
{
	struct DistanceResult
	{
		double m_Distance;
		GLC_Point4d m_Point1;
		GLC_Point4d m_Point2;
		GLC_uint m_InstanceId1;
		GLC_uint m_InstanceId2;
	};
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_Distance();

	//! Construct a distmin with 2 GLC_3DViewInstance
	GLC_Distance(const GLC_3DViewInstance&, const GLC_3DViewInstance&);

	//! Copy Constructor
	GLC_Distance(const GLC_Distance&);

	//! Default destructor
	virtual ~GLC_Distance();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Clear the 2 groups
	void clear();

	//! Add instance in group 1
	void addInstanceInGroup1(const GLC_3DViewInstance&);

	//! Add instances list in group 1
	void addInstancesInGroup1(const QList<GLC_3DViewInstance>&);

	//! Add instance in group 1
	void addInstanceInGroup2(const GLC_3DViewInstance&);

	//! Add instances list in group 1
	void addInstancesInGroup2(const QList<GLC_3DViewInstance>&);

	//! Compute the minimum distance between the 2 groups
	void computeMinimumDistance();

	//! Set the relative Error value
	inline void setRelativeError(double error)
	{m_RelativeError= error;}

	//! Set the absolute Error value
	inline void setAbsoluteError(double error)
	{m_RelativeError= error;}

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the minimum distance between the 2 groups
	inline double distMin() const
	{return m_DistanceMini;}

	//! Return First point of the distance
	inline GLC_Point4d point1() const
	{return m_Point1;}

	//! Return Second point of the distance
	inline GLC_Point4d point2() const
	{return m_Point2;}

	//! Return the relative error
	inline double relativeError() const
	{return m_RelativeError;}

	//! Return the absolute error
	inline double absoluteError() const
	{return m_AbsoluteError;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \private services functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:

	//! Return distance mini beween to instance
	DistanceResult minimumDistance(QList<GLC_3DViewInstance>&, QList<GLC_3DViewInstance>&) const;

	//! Get the PQP Point after matricial transformation
	void getPQPPoint(double&, double&, double&, const double, const double, const double, const GLC_Matrix4x4&) const;

	//! Add mesh triangles to PQP model
	void addMeshTrianglesToPQP(PQP_Model*, const QList<GLC_ExtendedMesh*>, const GLC_Matrix4x4&) const;

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! The first list of instance
	QList<GLC_3DViewInstance> m_ListOfInstances1;

	//! The Second list of instance
	QList<GLC_3DViewInstance> m_ListOfInstances2;

	//! The Minimum Distance point 1
	GLC_Point4d m_Point1;

	//! The Minimum Distance point 2
	GLC_Point4d m_Point2;

	//! The minimum distance
	double m_DistanceMini;

	//! The PQP relative error
	double m_RelativeError;

	//! The PQP abolute error
	double m_AbsoluteError;
};

#endif /* GLC_DISTANCE_H_ */
