/**
 ******************************************************************************
 *
 * @file       udpconnect.h
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

#ifndef UDPCONNECT_H
#define UDPCONNECT_H

#include <QObject>
#include <QUdpSocket>
#include <QList>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>
#include "aerosimrcdatastruct.h"

class UdpSender : public QObject
{
//    Q_OBJECT
public:
    explicit UdpSender(const QList<quint8> map, bool isTX, QObject *parent = 0);
    ~UdpSender();
    void init(const QString &remoteHost, quint16 remotePort);
    void sendDatagram(const simToPlugin *stp);
    quint32 pcks() { return packetsSended; }

private:
    QUdpSocket *outSocket;
    QHostAddress outHost;
    quint16 outPort;
    QList<float> channels;
    QList<quint8> channelsMap;
    bool takeFromTX;
    quint32 packetsSended;
};


class UdpReceiver : public QThread
{
//    Q_OBJECT
public:
    explicit UdpReceiver(const QList<quint8> map, bool isRX, QObject *parent = 0);
    ~UdpReceiver();
    void init(const QString &localHost, quint16 localPort);
    void run();
    void stop();
    // function getChannels for other threads
    void setChannels(pluginToSim *pts);
    void getFlighStatus(quint8 &arm, quint8 &mod);
    quint8 getArmed() { return armed; }
    quint8 getMode() { return mode; }
    quint32 pcks() { return packetsRecived; }

private:
    volatile bool stopped;
    QMutex mutex;
    QUdpSocket *inSocket;
    QList<float> channels;
    QList<quint8> channelsMap;
    bool sendToRX;
    quint8 armed;
    quint8 mode;
    quint32 packetsRecived;
    void onReadyRead();
    void processDatagram(QByteArray &datagram);
};

#endif // UDPCONNECT_H
