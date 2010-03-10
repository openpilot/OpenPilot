/**
 ******************************************************************************
 *
 * @file       qtlocalpeer.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   
 * @{
 * 
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

#include "qtlocalpeer.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QTime>

#if defined(Q_OS_WIN)
#include <QtCore/QLibrary>
#include <QtCore/qt_windows.h>
typedef BOOL(WINAPI*PProcessIdToSessionId)(DWORD,DWORD*);
static PProcessIdToSessionId pProcessIdToSessionId = 0;
#endif

#if defined(Q_OS_UNIX)
#include <time.h>
#include <unistd.h>
#endif

namespace SharedTools {

const char *QtLocalPeer::ack = "ack";

QtLocalPeer::QtLocalPeer(QObject *parent, const QString &appId)
    : QObject(parent), id(appId)
{
    if (id.isEmpty())
        id = QCoreApplication::applicationFilePath();  //### On win, check if this returns .../argv[0] without casefolding; .\MYAPP == .\myapp on Win

    QByteArray idc = id.toUtf8();
    quint16 idNum = qChecksum(idc.constData(), idc.size());
    //### could do: two 16bit checksums over separate halves of id, for a 32bit result - improved uniqeness probability. Every-other-char split would be best.

    socketName = QLatin1String("qtsingleapplication-")
                 + QString::number(idNum, 16);
#if defined(Q_OS_WIN)
    if (!pProcessIdToSessionId) {
        QLibrary lib("kernel32");
        pProcessIdToSessionId = (PProcessIdToSessionId)lib.resolve("ProcessIdToSessionId");
    }
    if (pProcessIdToSessionId) {
        DWORD sessionId = 0;
        pProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
        socketName += QLatin1Char('-') + QString::number(sessionId, 16);
    }
#else
    socketName += QLatin1Char('-') + QString::number(::getuid(), 16);
#endif

    server = new QLocalServer(this);
    QString lockName = QDir(QDir::tempPath()).absolutePath()
                       + QLatin1Char('/') + socketName
                       + QLatin1String("-lockfile");
    lockFile.setFileName(lockName);
    lockFile.open(QIODevice::ReadWrite);
}

bool QtLocalPeer::isClient()
{
    if (lockFile.isLocked())
        return false;

    if (!lockFile.lock(QtLockedFile::WriteLock, false))
        return true;

    if (!QLocalServer::removeServer(socketName))
        qWarning("QtSingleCoreApplication: could not cleanup socket");
    bool res = server->listen(socketName);
    if (!res)
        qWarning("QtSingleCoreApplication: listen on local socket failed, %s", qPrintable(server->errorString()));
    QObject::connect(server, SIGNAL(newConnection()), SLOT(receiveConnection()));
    return false;
}

bool QtLocalPeer::sendMessage(const QString &message, int timeout)
{
    if (!isClient())
        return false;

    QLocalSocket socket;
    bool connOk = false;
    for (int i = 0; i < 2; i++) {
        // Try twice, in case the other instance is just starting up
        socket.connectToServer(socketName);
        connOk = socket.waitForConnected(timeout/2);
        if (connOk || i)
            break;
        int ms = 250;
#if defined(Q_OS_WIN)
        Sleep(DWORD(ms));
#else
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
#endif
    }
    if (!connOk)
        return false;

    QByteArray uMsg(message.toUtf8());
    QDataStream ds(&socket);
    ds.writeBytes(uMsg.constData(), uMsg.size());
    bool res = socket.waitForBytesWritten(timeout);
    res &= socket.waitForReadyRead(timeout); // wait for ack
    res &= (socket.read(qstrlen(ack)) == ack);
    return res;
}

void QtLocalPeer::receiveConnection()
{
    QLocalSocket* socket = server->nextPendingConnection();
    if (!socket)
        return;

    // Why doesn't Qt have a blocking stream that takes care of this shait???
    while (socket->bytesAvailable() < static_cast<int>(sizeof(quint32)))
        socket->waitForReadyRead();
    QDataStream ds(socket);
    QByteArray uMsg;
    quint32 remaining;
    ds >> remaining;
    uMsg.resize(remaining);
    int got = 0;
    char* uMsgBuf = uMsg.data();
    //qDebug() << "RCV: remaining" << remaining;
    do {
        got = ds.readRawData(uMsgBuf, remaining);
        remaining -= got;
        uMsgBuf += got;
        //qDebug() << "RCV: got" << got << "remaining" << remaining;
    } while (remaining && got >= 0 && socket->waitForReadyRead(2000));
    //### error check: got<0
    if (got < 0) {
        qWarning() << "QtLocalPeer: Message reception failed" << socket->errorString();
        delete socket;
        return;
    }
    // ### async this
    QString message = QString::fromUtf8(uMsg.constData(), uMsg.size());
    socket->write(ack, qstrlen(ack));
    socket->waitForBytesWritten(1000);
    delete socket;
    emit messageReceived(message); // ##(might take a long time to return)
}

} // namespace SharedTools
