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

//! \file glc_errorlog.h implementation of the GLC_ErrorLog class.

#include "glc_errorlog.h"
#include <QDir>
#include "glc_global.h"
#include <QMutexLocker>

GLC_ErrorLog* GLC_ErrorLog::m_pErrorLog= NULL;
QMutex GLC_ErrorLog::m_Mutex;

GLC_ErrorLog::GLC_ErrorLog(const QString& fullLogFileName)
: GLC_Log(fullLogFileName)
{

}

GLC_ErrorLog::~GLC_ErrorLog()
{

}

GLC_ErrorLog* GLC_ErrorLog::instance()
{
	if (NULL == m_pErrorLog)
	{
		QString fileName(QApplication::applicationName());
		if (fileName.isEmpty())
		{
			fileName= "GLC_lib_ErrLog";
		}
		else
		{
			fileName= fileName + "_ErrLog";
		}
		QString logFileName(QDir::tempPath() + QDir::separator() + fileName);
		m_pErrorLog= new GLC_ErrorLog(logFileName);
		m_pErrorLog->writeHeader();
	}
	return m_pErrorLog;
}

bool GLC_ErrorLog::isEmpty()
{
	return (NULL == m_pErrorLog);
}

void GLC_ErrorLog::addError(const QStringList& errorDescription)
{
	QMutexLocker locker(&m_Mutex);
	GLC_ErrorLog::instance()->addSeparator();
	GLC_ErrorLog::instance()->addCurrentTime();
	const int size= errorDescription.size();
	for (int i= 0; i < size; ++i)
	{
		GLC_ErrorLog::instance()->add(errorDescription.at(i));
	}
}
void GLC_ErrorLog::close()
{
	QMutexLocker locker(&m_Mutex);
	delete m_pErrorLog;
	m_pErrorLog= NULL;
}

void GLC_ErrorLog::writeHeader()
{
	QString currentLine;
	currentLine= "Error Log file";
	GLC_Log::m_TextStream << currentLine << '\n';
	currentLine= "Application " + QCoreApplication::applicationName();
	GLC_Log::m_TextStream << currentLine << '\n';
	currentLine= QDate::currentDate().toString(Qt::ISODate);
	GLC_Log::m_TextStream << currentLine << '\n';
	GLC_Log::m_TextStream.flush();
}
