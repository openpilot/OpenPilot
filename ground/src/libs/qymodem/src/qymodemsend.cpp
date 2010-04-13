/**
 ******************************************************************************
 *
 * @file       qymodemsend.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Implementation of Y-Modem File transmit protocol.
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

#include "qymodemsend.h"


/**
Constructor.

@param port	The port to use for File transmission.


*/
QymodemSend::QymodemSend(QextSerialPort& port)
    : QymodemTx(port)
    {

    }



/**
Send the file.

*/
void QymodemSend::Send()
{

    QymodemFileStream source;
    int error = source.Open(FileName);
    if(error)
        emit Error("Can't open file " + QString(FileName),error);


    unsigned Timeout=30000;
    error = SendY(FileName,source.Size(),source,Timeout);
    if(error)
    {
        emit Error("Error during file transfer, error "+QString::number(error),error);
    }
    else
    {

        emit Information("Sent OK",QymodemSend::InfoSent);
    }
    source.Close();
}
/**
Send file.

@param filename	The name of the file to Send.

@return Zero if successful, or a negative error value if failed.

*/
int QymodemSend::SendFile(QString filename)
{
    QFile file;
    if(!file.exists(filename))
    {
        emit Error("File not found",QymodemSend::ErrorFileNotFound);
        return QymodemSend::ErrorFileNotFound;
    }
    if(!Port.open(_DEVICE_SET_))
    {
        emit Error("Could not open port",QymodemSend::ErrorCoulNotOpenPort);
        return QymodemSend::ErrorCoulNotOpenPort;
    }
        QByteArray a=filename.toLocal8Bit();
        FileName=a.constData();
        Send();
        Port.close();
        return 0;

}
/**
Send file on a new Thread.

@param filename	The name of the file to Send.

@return Zero if successful, or a negative error value if failed.

*/
int QymodemSend::SendFileT(QString filename)
{
    if(!isRunning())
    {
        FileNameT=filename;
        start();
    }
    else
    {
        return QymodemSend::ErrorFileTransmissionInProgress;
    }
    return 0;
}

void QymodemSend::run()
{
    QFile file;
    if(!file.exists(FileNameT))
    {
        emit Error("File not found",QymodemSend::ErrorFileNotFound);
        return;
    }
    if(!Port.open(_DEVICE_SET_))
    {
        emit Error("Could not open port",QymodemSend::ErrorCoulNotOpenPort);
        return;
    }
        QByteArray a=FileNameT.toLocal8Bit();
        FileName=a.constData();
        Send();
        Port.close();
        return;

}
