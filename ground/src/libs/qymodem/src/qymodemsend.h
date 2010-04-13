/**
 ******************************************************************************
 *
 * @file       qymodemsend.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   ymodem_lib
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


#ifndef QYMODEMSEND_H
#define QYMODEMSEND_H
#include "qymodem_tx.h"
#include <QString>
#include <QFile>
#include "qymodemfilestream.cpp"

#ifdef Q_OS_WIN
#define _DEVICE_SET_ QIODevice::ReadWrite|QIODevice::Unbuffered
#else
#define _DEVICE_SET_ QIODevice::ReadWrite
#endif

/**
Class for sending a file via Y-Modem transmit protocol.
*/
class QymodemSend:public QymodemTx
{

public:
    QymodemSend(QextSerialPort& port);
    int SendFile(QString filename);
    int SendFileT(QString filename);

private:
    void run();
    const char* FileName;
    void Send();
    QymodemFileStream InFile;
    QString FileNameT;

};
#endif // QYmodemSend_H
