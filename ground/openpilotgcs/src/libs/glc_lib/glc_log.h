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

//! \file glc_log.h interface for the GLC_Log class.

#ifndef GLC_LOG_H_
#define GLC_LOG_H_

#include <QString>
#include <QTemporaryFile>
#include <QTextStream>
#include "glc_config.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Log
/*! \brief GLC_Log : Base class for GLC_lib log*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_Log
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor */
//@{
//////////////////////////////////////////////////////////////////////
protected:
	//! Construct a log of the given base fileName
	GLC_Log(const QString& baseLogFileName);
public:
	//! Destructor
	virtual ~GLC_Log();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return this log full file name
	QString fullFileName() const;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
protected:
	//! Add the given string to this log
	void add(const QString& line);

	//! Add blank line to this log
	inline void addBlankLine()
	{add(QString());}

	//! Add a separator in the log
	void addSeparator();

	//! Add current time in log
	void addCurrentTime();


//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
protected:

	//! This log file
	QTemporaryFile* m_pFile;

	//! This log textStream
	QTextStream m_TextStream;

};

#endif /* GLC_LOG_H_ */
