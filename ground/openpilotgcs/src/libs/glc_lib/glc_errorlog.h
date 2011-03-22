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
//! \file glc_errorlog.h interface for the GLC_ErrorLog class.

#ifndef GLC_ERRORLOG_H_
#define GLC_ERRORLOG_H_

#include "glc_log.h"
#include "glc_config.h"
#include <QStringList>
#include <QMutex>

//////////////////////////////////////////////////////////////////////
//! \class GLC_ErrorLog
/*! \brief GLC_ErrorLog : handl GLC_lib error log*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_ErrorLog : public GLC_Log
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor */
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Private constructor
	GLC_ErrorLog(const QString& fullLogFileName);
public:
	//! Destructor
	virtual ~GLC_ErrorLog();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the unique instance of error log file
	static GLC_ErrorLog* instance();

	//! Return true if the log is empty
	static bool isEmpty();

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Add error into the log
	static void addError(const QStringList& errorDescription);

	//! Close the log file
	static void close();

//@}


//////////////////////////////////////////////////////////////////////
/*! \name Private services Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Write error Log header
	void writeHeader();

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! The unique error log instance
	static GLC_ErrorLog* m_pErrorLog;

	//! The mutex of this unique log
	static QMutex m_Mutex;

};

#endif /* GLC_ERRORLOG_H_ */
