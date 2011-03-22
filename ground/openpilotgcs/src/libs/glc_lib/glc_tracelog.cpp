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

#include "glc_tracelog.h"

#include <QDir>
#include "glc_global.h"
#include <QMutexLocker>

GLC_TraceLog* GLC_TraceLog::m_pTraceLog= NULL;
QMutex GLC_TraceLog::m_Mutex;
bool GLC_TraceLog::m_IsEnable= false;

GLC_TraceLog::GLC_TraceLog(const QString& fullLogFileName)
: GLC_Log(fullLogFileName)
{

}

GLC_TraceLog::~GLC_TraceLog()
{

}

GLC_TraceLog* GLC_TraceLog::instance(QString baseName)
{
	if (NULL == m_pTraceLog)
	{
		if (baseName.isEmpty())
		{
			QString fileName(QApplication::applicationName());
			if (fileName.isEmpty())
			{
				baseName= "GLC_lib_TraceLog";
			}
			else
			{
				baseName= fileName + "_TraceLog";
			}
		}
		QString logFileName(QDir::tempPath() + QDir::separator() + baseName);
		m_pTraceLog= new GLC_TraceLog(logFileName);
		m_pTraceLog->writeHeader();
	}
	return m_pTraceLog;
}

bool GLC_TraceLog::isEmpty()
{
	return (NULL == m_pTraceLog);
}

bool GLC_TraceLog::isEnable()
{
	return m_IsEnable;
}

void GLC_TraceLog::addTrace(const QStringList& traceDescription)
{
	if (m_IsEnable)
	{
		QMutexLocker locker(&m_Mutex);
		GLC_TraceLog::instance()->addSeparator();
		GLC_TraceLog::instance()->addCurrentTime();
		const int size= traceDescription.size();
		for (int i= 0; i < size; ++i)
		{
			GLC_TraceLog::instance()->add(traceDescription.at(i));
		}
	}
}
void GLC_TraceLog::close()
{
	QMutexLocker locker(&m_Mutex);
	delete m_pTraceLog;
	m_pTraceLog= NULL;
}

void GLC_TraceLog::setEnabled(bool enable)
{
	m_IsEnable= enable;
}

void GLC_TraceLog::writeHeader()
{
	QString currentLine;
	currentLine= "Trace Log file";
	GLC_Log::m_TextStream << currentLine << '\n';
	currentLine= "Application " + QCoreApplication::applicationName();
	GLC_Log::m_TextStream << currentLine << '\n';
	currentLine= QDate::currentDate().toString(Qt::ISODate);
	GLC_Log::m_TextStream << currentLine << '\n';
	GLC_Log::m_TextStream.flush();
}
