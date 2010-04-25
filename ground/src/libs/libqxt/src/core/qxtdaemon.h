/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/

#ifndef QXTDAEMON_H
#define QXTDAEMON_H

#include "qxtglobal.h"
#include <QObject>
#include <QString>
#include <QCoreApplication>

QT_FORWARD_DECLARE_CLASS(QFile)

class QXT_CORE_EXPORT QxtDaemon : public QObject
{
    Q_OBJECT
public:
    QxtDaemon(QString applicationName = QCoreApplication::applicationName());
    bool daemonize(bool pidfile = true);
    static int pid();
    bool changeUser(QString name);

#ifndef QXT_DOXYGEN_RUN

    static void messageHandler(QtMsgType type, const char *msg);
#ifdef Q_OS_UNIX
    static void signalHandler(int signal);
#endif

#endif

private:
    QString m_name;
    QFile *logfile;
Q_SIGNALS:
    void signal(int);
    void hangup();
    void terminate();
};

#endif // QXT_DAEMON_H
