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
//! \file glc_selectionset.h interface for the GLC_SelectionSet class.

#ifndef GLC_SELECTIONSET_H_
#define GLC_SELECTIONSET_H_

#include <QHash>
#include <QList>
#include <QSet>

#include "glc_structoccurence.h"
#include "../glc_global.h"

#include "../glc_config.h"

class GLC_WorldHandle;

//////////////////////////////////////////////////////////////////////
//! \class GLC_SelectionSet
/*! \brief GLC_SelectionSet : GLC_StructOccurence and primitive selection set */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_SelectionSet
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

public:
	//! Construct the selection set associated to the given GLC_WorldHandle
	GLC_SelectionSet(GLC_WorldHandle* pWorld);
	virtual ~GLC_SelectionSet();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Return true if this selection set is empty
	bool isEmpty() const;

	//! Return the number of occurence in this selection set
	inline int size() const
	{return count();}

	//! Return the number of occurence in this selection set
	int count() const;

	//! Return the list of selected occurences
	QList<GLC_StructOccurence*> occurencesList() const;

	//! Return the list of occurences with selected primitive
	QList<GLC_StructOccurence*> occurencesListWithSelectedPrimitive() const;

	//! Return the set of primitive id of the given GLC_StructOccurence/GLC_3DviewInstance id
	QSet<GLC_uint> selectedPrimitivesId(GLC_uint instanceId) const;

	//! Return true if the given GLC_StructOccurence/GLC_3DviewInstance id has selected primitives
	bool hasPrimitiveSelected(GLC_uint instanceId) const;

	//! Return true if this selection set contains the given occurence
	bool contains(const GLC_StructOccurence* pOccurence) const
	{return contains(pOccurence->id());}

	//! Return true if this selection set contains the given occurence id
	bool contains(GLC_uint occurenceId) const
	{return m_OccurenceHash.contains(occurenceId);}

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Insert the given Occurence into the selection set and return true on success
	/*! The given occurence must belongs to this selection set's world*/
	bool insert(GLC_StructOccurence* pOccurence);

	//! Insert the given Occurence id into the selection set and return true on success
	/*! The given occurence id must belongs to this selection set's world*/
	bool insert(GLC_uint occurenceId);

	//! Remove the given occurence from the selection set and return true on success
	/*! The given occurence must belongs to this selection set's world*/
	bool remove(GLC_StructOccurence* pOccurence);

	//! Remove the given occurence from the selection set and return true on success
	/*! The given occurence id must belongs to this selection set's world*/
	bool remove(GLC_uint occurenceId);

	//! Clear this selection set
	void clear();

	//! Insert the given primitive id to the given Occurence return true on success
	/*! The given occurence must belongs to this selection set's world*/
	bool insertPrimitiveId(GLC_StructOccurence* pOccurence, GLC_uint primitiveId);

	//! Insert the given primitive id to the given Occurence id return true on success
	/*! The given occurence id must belongs to this selection set's world*/
	bool insertPrimitiveId(GLC_uint occurenceId, GLC_uint primitiveId);

	//! Insert the given set of primitive id to the given Occurence
	/*! The given occurence must belongs to this selection set's world*/
	void insertSetOfPrimitivesId(GLC_StructOccurence* pOccurence, const QSet<GLC_uint>& setOfPrimitivesId);

	//! Insert the given set of primitive id to the given Occurence id
	/*! The given occurence id must belongs to this selection set's world*/
	void insertSetOfPrimitivesId(GLC_uint occurenceId, const QSet<GLC_uint>& setOfPrimitivesId);

	//! Remove the given primitive id to the given Occurence return true on success
	/*! The given occurence must belongs to this selection set's world
	 *  If the set of primitive only contains the given primitive ID
	 *  the given occurence is removed from this selection set
	 */
	bool removePrimitiveId(GLC_StructOccurence* pOccurence, GLC_uint primitiveId);

	//! Remove the given primitive id to the given Occurence return true on success
	/*! The given occurence must belongs to this selection set's world
	 *  If the set of primitive only contains the given primitive ID
	 *  the given occurence is removed from this selection set
	 */
	bool removePrimitiveId(GLC_uint occurenceId, GLC_uint primitiveId);


//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! The worldHandle attached to this selection set
	GLC_WorldHandle* m_pWorldHandle;

	//! Hash table of selected occurence
	QHash<GLC_uint, GLC_StructOccurence*> m_OccurenceHash;

	//! Hash table of instance id to set of primitive id
	QHash<GLC_uint, QSet<GLC_uint> > m_InstanceToPrimitiveId;

};

#endif /* GLC_SELECTIONSET_H_ */
