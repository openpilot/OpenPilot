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
//! \file glc_attributes.h interface for the GLC_Attributes class.

#ifndef GLC_ATTRIBUTES_H_
#define GLC_ATTRIBUTES_H_

#include <QString>
#include <QList>
#include <QHash>

#include "../glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Attributes
/*! \brief GLC_Attributes : User attributes of instance and reference */
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Attributes
{

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default Constructor
	GLC_Attributes();

	//! Copy Constructor
	GLC_Attributes(const GLC_Attributes&);

	//! Overload "=" operator
	GLC_Attributes& operator=(const GLC_Attributes&);

	//! Destructor
	virtual ~GLC_Attributes();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return true if attributes is empty
	inline bool isEmpty() const
	{return m_AttributesHash.isEmpty();}

	//! Return the size of attributes
	inline int size() const
	{return m_AttributesHash.size();}

	//! Return true if the specified attribute exist
	bool contains(const QString& name) const
	{return m_AttributesHash.contains(name);}

	//! Return the list of attribute name
	inline QList<QString> names() const
	{return m_AttributesList;}

	//! Return the value of the specified attributes
	/*! Return NULL String if attribute doesn't exist*/
	inline QString value(const QString& name) const
	{return m_AttributesHash.value(name);}

	//! Return the name of the specified attributes index
	/*! Return empty String if attribute doesn't exist*/
	inline QString name(int at) const
	{return m_AttributesList.value(at);}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Insert an attribute (if the attribute exists, it's updated)
	inline void insert(const QString& name, const QString& value)
	{
		if ((!m_AttributesHash.contains(name))) m_AttributesList.append(name);
		m_AttributesHash.insert(name, value);
	}

	//! Remove an attribute
	inline void remove(const QString& name)
	{
		Q_ASSERT(m_AttributesHash.contains(name));
		m_AttributesHash.remove(name);
		m_AttributesList.removeOne(name);
	}

	//! Clear the content of this attribute
	inline void clear()
	{
		m_AttributesHash.clear();
		m_AttributesList.clear();
	}

//@}

//////////////////////////////////////////////////////////////////////
/*! @name Operator Overload */
//@{
//////////////////////////////////////////////////////////////////////
public:

	//! Equal operator overload
	inline bool operator==(const GLC_Attributes& attr) const
	{return m_AttributesHash == attr.m_AttributesHash;}

//@}
//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! Attributes Hash table
	QHash<QString, QString> m_AttributesHash;

	//! the list of attribute name
	QList<QString> m_AttributesList;

};

#endif /* GLC_ATTRIBUTES_H_ */
