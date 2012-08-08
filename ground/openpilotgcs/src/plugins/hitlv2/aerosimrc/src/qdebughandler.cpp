/**
 ******************************************************************************
 *
 * @file       qdebughandler.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2012.
 * @addtogroup 3rdParty Third-party integration
 * @{
 * @addtogroup AeroSimRC AeroSimRC proxy plugin
 * @{
 * @brief AeroSimRC simulator to HITL proxy plugin
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "qdebughandler.h"

void myQDebugHandler(QtMsgType type, const char *msg)
{
    static bool firstRun = true;
    QString txt;

    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        break;
    }

    QFile outFile("dbglog.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    QTime time;

    if (firstRun) {
        ts << endl << endl;
        firstRun = false;
    }

    ts << time.currentTime().toString("hh:mm:ss.zzz") << " " << txt << endl;

    if (type == QtFatalMsg)
        abort();
}
