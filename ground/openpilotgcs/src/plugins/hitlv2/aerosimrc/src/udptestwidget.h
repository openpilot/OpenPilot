/**
 ******************************************************************************
 *
 * @file       udptestwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2012.
 * @addtogroup 3rdParty Third-party integration
 * @{
 * @addtogroup AeroSimRC AeroSimRC proxy plugin
 * @{
 * @brief AeroSimRC simulator to HITL proxy plugin test utility
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

#ifndef UDPTESTWIDGET_H
#define UDPTESTWIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QTime>
#include <qmath.h>
#include <QVector3D>
#include <QMatrix4x4>
#include <QDebug>
#include <QTimer>

namespace Ui {
    class Widget;
}

const float RAD2DEG = (float)(180.0/M_PI);
const float DEG2RAD = (float)(M_PI/180.0);

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_btReciveStart_clicked();
    void on_btReciveStop_clicked();
    void on_btTransmitStart_clicked();
    void on_btTransmitStop_clicked();

    void readDatagram();
    void sendDatagram();

    void on_autoSend_clicked();

    void on_autoAnswer_clicked();

private:
    Ui::Widget *ui;

    QTime screenTimeout;
    QUdpSocket *inSocket;
    QUdpSocket *outSocket;
    QHostAddress outHost;
    quint16 outPort;
    quint32 packetCounter;

    void processDatagram(const QByteArray &data);
    QTimer *autoSendTimer;

    void asMatrix2Quat(const QMatrix4x4 &m, QQuaternion &q);
    void asMatrix2RPY(const QMatrix4x4 &m, QVector3D &rpy);
    void asQuat2RPY(const QQuaternion &q, QVector3D &rpy);

/*  // not used
 *  void ccRPY2Quat(const QVector3D &rpy, QQuaternion &q);
 *  void ccQuat2Matrix(const QQuaternion &q, QMatrix4x4 &m);
 */
};

#endif // UDPTESTWIDGET_H
