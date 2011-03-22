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

#include <QString>
#include <QDateTime>

#include "../glc_config.h"

#ifndef GLC_REP_H_
#define GLC_REP_H_
//////////////////////////////////////////////////////////////////////
//! \class GLC_Rep
/*! \brief GLC_Rep : Abstract class for a reference represention*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Rep
{
public:
	enum Type
	{
		GLC_VBOGEOM= 1
	};
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Default constructor
	GLC_Rep();

	//! Copy constructor
	GLC_Rep(const GLC_Rep&);

	//! Assignement operator
	virtual GLC_Rep &operator=(const GLC_Rep&);

	//! Clone the representation
	virtual GLC_Rep* clone() const = 0;

	//! Return a deep copy of the representation
	virtual GLC_Rep* deepCopy() const = 0;

	//! Destructor
	virtual ~GLC_Rep();

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return true if the representation is the last
	inline bool isTheLast() const
	{return 1 == (*m_pNumberOfRepresentation);}

	//! Return true if representations are equals
	inline bool operator==(const GLC_Rep& rep)
	{
		return (rep.m_pNumberOfRepresentation == m_pNumberOfRepresentation);
	}

	//! Return the representation file name
	inline QString fileName() const
	{return (*m_pFileName);}

	//! Return the type of representation
	virtual int type() const =0;

	//! Return the name of the rep
	inline QString name() const
	{return (*m_pName);}

	//! Return true if the representation is empty
	virtual bool isEmpty() const= 0;

	//! Return true if the representation as been loaded
	inline bool isLoaded() const
	{return *m_pIsLoaded;}

	//! Return the rep file las modified date and time
	inline QDateTime lastModified() const
	{return *m_pDateTime;}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Set the representation FileName
	inline void setFileName(const QString& fileName)
	{(*m_pFileName)= fileName;}

	//! Set the representation Name
	inline void setName(const QString& name)
	{(*m_pName)= name;}

	//! Load the representation
	virtual bool load()= 0;

	//! UnLoad the representation
	virtual bool unload()= 0;

	//! Replace the representation
	virtual void replace(GLC_Rep*)= 0;

	//! Set the last modified date and time
	inline void setLastModified(const QDateTime& dateTime)
	{*m_pDateTime= dateTime;}

//@}
//////////////////////////////////////////////////////////////////////
// private services functions
//////////////////////////////////////////////////////////////////////
private:
	//! Clear current representation
	void clear();
//////////////////////////////////////////////////////////////////////
// protected members
//////////////////////////////////////////////////////////////////////
protected:

	//! Flag to know if the representation has been loaded
	bool* m_pIsLoaded;

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:

	//! Number of this representation
	int* m_pNumberOfRepresentation;

	//! The File Name of this representation
	QString* m_pFileName;

	//! The Name of the rep
	QString* m_pName;

	//! The Date and time of the rep
	QDateTime* m_pDateTime;

};

#endif /* GLC_REP_H_ */
