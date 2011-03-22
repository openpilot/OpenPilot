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

#ifndef GLC_TRACELOG_H_
#define GLC_TRACELOG_H_

#include "glc_log.h"
#include "glc_config.h"
#include <QStringList>
#include <QMutex>

//////////////////////////////////////////////////////////////////////
//! \class GLC_TraceLog
/*! \brief GLC_TraceLog : handle GLC_lib trace log*/
//////////////////////////////////////////////////////////////////////
class GLC_LIB_EXPORT GLC_TraceLog : public GLC_Log
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor */
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Private constructor
	GLC_TraceLog(const QString& fullLogFileName);
public:
	//! Destructor
	virtual ~GLC_TraceLog();
//@}
//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return the unique instance of trace log file
	static GLC_TraceLog* instance(QString baseName= QString());

	//! Return true if the log is empty
	static bool isEmpty();

	//! Return true if the trace log is enable
	static bool isEnable();

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Add error into the log
	static void addTrace(const QStringList& traceDescription);

	//! Close the log file
	static void close();

	//! Set enable
	static void setEnabled(bool enable);

//@}


//////////////////////////////////////////////////////////////////////
/*! \name Private services Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Write trace Log header
	void writeHeader();

//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////
private:
	//! The unique trace log instance
	static GLC_TraceLog* m_pTraceLog;

	//! The mutex of this unique log
	static QMutex m_Mutex;

	//! the trace log enable status
	static bool m_IsEnable;

};

#endif /* GLC_TRACELOG_H_ */
