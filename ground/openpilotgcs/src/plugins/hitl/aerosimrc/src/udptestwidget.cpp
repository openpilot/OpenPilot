/**
 ******************************************************************************
 *
 * @file       udptestwidget.cpp
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

#include "udptestwidget.h"
#include "ui_udptestwidget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    inSocket = NULL;
    outSocket = NULL;
    screenTimeout.start();
    packetCounter = 0;

    autoSendTimer = new QTimer(this);
    connect(autoSendTimer, SIGNAL(timeout()), this, SLOT(sendDatagram()), Qt::DirectConnection);
}

Widget::~Widget()
{
    if(outSocket) {
        delete outSocket;
    }
    if(inSocket) {
        delete inSocket;
    }
    delete ui;
}

// private slots //////////////////////////////////////////////////////////////

void Widget::on_btReciveStart_clicked()
{
    on_btReciveStop_clicked();

    inSocket = new QUdpSocket();
    QString host = ui->localHost->text();
    quint16 port = ui->localPort->text().toInt();

    if (inSocket->bind(QHostAddress(host), port)) {
        connect(inSocket, SIGNAL(readyRead()),
                this, SLOT(readDatagram()), Qt::DirectConnection);

        ui->listWidget->addItem("bind ok");
        ui->btReciveStop->setEnabled(1);
        ui->localHost->setDisabled(1);
        ui->localPort->setDisabled(1);
        ui->btReciveStart->setDisabled(1);
    } else {
        ui->listWidget->addItem("bind error: " + inSocket->errorString());
    }
}

void Widget::on_btReciveStop_clicked()
{
    if(inSocket) {
        delete inSocket;
        inSocket = NULL;
        ui->listWidget->addItem("unbind ok");
    } else {
        ui->listWidget->addItem("socket not found");
    }
    ui->btReciveStart->setEnabled(1);
    ui->localHost->setEnabled(1);
    ui->localPort->setEnabled(1);
    ui->btReciveStop->setDisabled(1);
}

void Widget::on_btTransmitStart_clicked()
{
    on_btTransmitStop_clicked();

    outSocket = new QUdpSocket();
    outHost = ui->simHost->text();
    outPort = ui->simPort->text().toInt();

    ui->listWidget->addItem("transmit started");
    ui->btTransmitStop->setEnabled(1);
    ui->simHost->setDisabled(1);
    ui->simPort->setDisabled(1);
    ui->btTransmitStart->setDisabled(1);
}

void Widget::on_btTransmitStop_clicked()
{
    if(outSocket) {
        delete outSocket;
        outSocket = NULL;
        ui->listWidget->addItem("transmit stopped");
    } else {
        ui->listWidget->addItem("transmit socket not found");
    }
    ui->btTransmitStart->setEnabled(1);
    ui->simHost->setEnabled(1);
    ui->simPort->setEnabled(1);
    ui->btTransmitStop->setDisabled(1);
}

void Widget::readDatagram()
{
    while (inSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(inSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        quint64 datagramSize = inSocket->readDatagram(datagram.data(), datagram.size(),
                                                      &sender, &senderPort);
        Q_UNUSED(datagramSize);

        processDatagram(datagram);
        if (ui->autoAnswer->isChecked())
            sendDatagram();
    }
}

// private ////////////////////////////////////////////////////////////////////

void Widget::processDatagram(const QByteArray &data)
{
    QByteArray buf = data;
    QDataStream stream(&buf, QIODevice::ReadOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // check magic header
    quint32 magic;
    stream >> magic;
    if (magic == 0x4153494D) {  // "AERO"
        float   timeStep,
                homeX, homeY, homeZ,
                WpHX, WpHY, WpLat, WpLon,
                posX, posY, posZ,
                velX, velY, velZ,
                angX, angY, angZ,
                accX, accY, accZ,
                lat, lon, alt,
                head, pitch, roll,
                volt, curr,
                rx, ry, rz, fx, fy, fz, ux, uy, uz,
                chAil, chEle, chThr, chRud, chPlg1, chPlg2, chFpv1, chFpv2;

        stream >> timeStep;
        stream >> homeX >> homeY >> homeZ;
        stream >> WpHX >> WpHY >> WpLat >> WpLon;
        stream >> posX >> posY >> posZ;
        stream >> velX >> velY >> velZ;
        stream >> angX >> angY >> angZ;
        stream >> accX >> accY >> accZ;
        stream >> lat >> lon >> alt;
        stream >> head >> pitch >> roll;
        stream >> volt >> curr;
        stream >> rx >> ry >> rz >> fx >> fy >> fz >> ux >> uy >> uz;
        stream >> chAil >> chEle >> chThr >> chRud >> chPlg1 >> chPlg2 >> chFpv1 >> chFpv2;
        stream >> packetCounter;

        if(ui->tabWidget->currentIndex() != 0)
            return;

        if (screenTimeout.elapsed() < 200)
            return;

        ui->listWidget->clear();
        /*
        ui->listWidget->addItem("time step (s)");
        ui->listWidget->addItem(QString("%1")
                                .arg(timeStep);
        ui->listWidget->addItem("home location (m)");
        ui->listWidget->addItem(QString("%1, %2, %3")
                                .arg(homeX, 7, 'f', 4)
                                .arg(homeY, 7, 'f', 4)
                                .arg(homeZ, 7, 'f', 4));
        ui->listWidget->addItem("home waypoint");
        ui->listWidget->addItem(QString("%1, %2, %3, %4")
                                .arg(WpHX, 7, 'f', 4)
                                .arg(WpHY, 7, 'f', 4)
                                .arg(WpLat, 7, 'f', 4)
                                .arg(WpLon, 7, 'f', 4));
        ui->listWidget->addItem("model position (m)");
        ui->listWidget->addItem(QString("%1, %2, %3")
                                .arg(posX, 7, 'f', 4)
                                .arg(posY, 7, 'f', 4)
                                .arg(posZ, 7, 'f', 4));
        ui->listWidget->addItem("model velocity (m/s)");
        ui->listWidget->addItem(QString("%1, %2, %3")
                                .arg(velX, 7, 'f', 4)
                                .arg(velY, 7, 'f', 4)
                                .arg(velZ, 7, 'f', 4));
        ui->listWidget->addItem("model angular velocity (rad/s)");
        ui->listWidget->addItem(QString("%1, %2, %3")
                                .arg(angX, 7, 'f', 4)
                                .arg(angY, 7, 'f', 4)
                                .arg(angZ, 7, 'f', 4));
        ui->listWidget->addItem("model acceleration (m/s/s)");
        ui->listWidget->addItem(QString("%1, %2, %3")
                                .arg(accX, 7, 'f', 4)
                                .arg(accY, 7, 'f', 4)
                                .arg(accZ, 7, 'f', 4));
        ui->listWidget->addItem("model coordinates (deg, deg, m)");
        ui->listWidget->addItem(QString("%1, %2, %3")
                                .arg(lat, 7, 'f', 4)
                                .arg(lon, 7, 'f', 4)
                                .arg(alt, 7, 'f', 4));
        ui->listWidget->addItem("model electrics");
        ui->listWidget->addItem(QString("%1V, %2A")
                                .arg(volt, 7, 'f', 4)
                                .arg(curr, 7, 'f', 4));
        ui->listWidget->addItem("channels");
        ui->listWidget->addItem(QString("%1 %2 %3 %4 %5 %6 %7 %8")
                                .arg(chAil, 6, 'f', 3)
                                .arg(chEle, 6, 'f', 3)
                                .arg(chThr, 6, 'f', 3)
                                .arg(chRud, 6, 'f', 3)
                                .arg(chPlg1, 6, 'f', 3)
                                .arg(chPlg2, 6, 'f', 3)
                                .arg(chFpv1, 6, 'f', 3)
                                .arg(chFpv2, 6, 'f', 3));
        ui->listWidget->addItem("datagram size (bytes), packet counter");
        ui->listWidget->addItem(QString("%1 %2")
                                .arg(data.size())
                                .arg(packetCounter));
*/

        // matrix calculation start
        // model matrix
        QMatrix4x4 m = QMatrix4x4( fy,  fx, -fz,  0.0,
                                   ry,  rx, -rz,  0.0,
                                  -uy, -ux,  uz,  0.0,
                                  0.0, 0.0, 0.0,  1.0);
        m.optimize();

        // world matrix
        QMatrix4x4 w = m.inverted();

        // model quat
        QQuaternion q;
        asMatrix2Quat(m, q);

        // model roll, pitch, yaw
        QVector3D rpy;
        asMatrix2RPY(m, rpy);

        // sonar
        float sAlt = 5.0;
        if ((alt < (sAlt * 2.0)) && (roll < 0.35) && (pitch < 0.35)) {
            float x = alt * qTan(roll);
            float y = alt * qTan(pitch);
            float h = QVector3D(x, y, alt).length();
            sAlt = qMin(h, sAlt);
        }

        ui->listWidget->addItem("sonar altitude");
        ui->listWidget->addItem(QString("%1")
                                .arg(sAlt, 8, 'f', 5));
        ui->listWidget->addItem("vectors");
        ui->listWidget->addItem(QString("       X       Y       Z"));
        ui->listWidget->addItem(QString("R: %1 %2 %3\nF: %4 %5 %6\nU: %7 %8 %9")
                                .arg(rx, 8, 'f', 5).arg(ry, 8, 'f', 5).arg(rz, 8, 'f', 5)
                                .arg(fx, 8, 'f', 5).arg(fy, 8, 'f', 5).arg(fz, 8, 'f', 5)
                                .arg(ux, 8, 'f', 5).arg(uy, 8, 'f', 5).arg(uz, 8, 'f', 5));
        ui->listWidget->addItem("CC model matrix");
        ui->listWidget->addItem(QString("   %1 %2 %3\n   %4 %5 %6\n   %7 %8 %9")
                                .arg(m(0, 0), 8, 'f', 5).arg(m(0, 1), 8, 'f', 5).arg(m(0, 2), 8, 'f', 5)
                                .arg(m(1, 0), 8, 'f', 5).arg(m(1, 1), 8, 'f', 5).arg(m(1, 2), 8, 'f', 5)
                                .arg(m(2, 0), 8, 'f', 5).arg(m(2, 1), 8, 'f', 5).arg(m(2, 2), 8, 'f', 5));
        ui->listWidget->addItem("CC world matrix");
        ui->listWidget->addItem(QString("   %1 %2 %3\n   %4 %5 %6\n   %7 %8 %9")
                                .arg(w(0, 0), 8, 'f', 5).arg(w(0, 1), 8, 'f', 5).arg(w(0, 2), 8, 'f', 5)
                                .arg(w(1, 0), 8, 'f', 5).arg(w(1, 1), 8, 'f', 5).arg(w(1, 2), 8, 'f', 5)
                                .arg(w(2, 0), 8, 'f', 5).arg(w(2, 1), 8, 'f', 5).arg(w(2, 2), 8, 'f', 5));
        ui->listWidget->addItem("CC quaternion");
        ui->listWidget->addItem(QString("%1, %2, %3, %4")
                                .arg(q.x(), 7, 'f', 4)
                                .arg(q.y(), 7, 'f', 4)
                                .arg(q.z(), 7, 'f', 4)
                                .arg(q.scalar(), 7, 'f', 4));
        ui->listWidget->addItem("model attitude (deg)");
        ui->listWidget->addItem(QString("%1, %2, %3")
                                .arg(roll*RAD2DEG, 7, 'f', 4)
                                .arg(pitch*RAD2DEG, 7, 'f', 4)
                                .arg(head*RAD2DEG, 7, 'f', 4));
        ui->listWidget->addItem("CC attitude calculated (deg)");
        ui->listWidget->addItem(QString("%1, %2, %3")
                                .arg(rpy.x(), 7, 'f', 4)
                                .arg(rpy.y(), 7, 'f', 4)
                                .arg(rpy.z(), 7, 'f', 4));

        screenTimeout.restart();

    } else if (magic == 0x52434D44) { // "RCMD"
        qreal ch1, ch2, ch3, ch4, ch5, ch6, ch7, ch8, ch9, ch10;
        stream >> ch1 >> ch2 >> ch3 >> ch4 >> ch5 >> ch6 >> ch7 >> ch8 >> ch9 >> ch10;
        quint8 armed, mode;
        stream >> armed >> mode;

        if(ui->tabWidget->currentIndex() == 0) {
            if (screenTimeout.elapsed() < 200)
                return;
            ui->listWidget->clear();
            ui->listWidget->addItem("channels");
            ui->listWidget->addItem("CH1: " + QString::number(ch1));
            ui->listWidget->addItem("CH2: " + QString::number(ch2));
            ui->listWidget->addItem("CH3: " + QString::number(ch3));
            ui->listWidget->addItem("CH4: " + QString::number(ch4));
            ui->listWidget->addItem("CH5: " + QString::number(ch5));
            ui->listWidget->addItem("CH6: " + QString::number(ch6));
            ui->listWidget->addItem("CH7: " + QString::number(ch7));
            ui->listWidget->addItem("CH8: " + QString::number(ch8));
            ui->listWidget->addItem("CH9: " + QString::number(ch9));
            ui->listWidget->addItem("CH10:" + QString::number(ch10));
            ui->listWidget->addItem("armed:" + QString::number(armed));
            ui->listWidget->addItem("fmode:" + QString::number(mode));
        }
        screenTimeout.restart();
    } else {
        qDebug() << "unknown magic:" << magic;
    }
}

void Widget::sendDatagram()
{
    if(!outSocket)
        return;

    float ch[10];   // = {0,0,0,0,0,0,0,0,0,0};
    quint8 armed;
    quint8 fmode;
    const float coeff = 1.0 / 512.0;

    ch[0] = ui->ch1->value() * coeff;
    ch[1] = ui->ch2->value() * coeff;
    ch[2] = ui->ch3->value() * coeff;
    ch[3] = ui->ch4->value() * coeff;
    ch[4] = ui->ch5->value() * coeff;
    ch[5] = ui->ch6->value() * coeff;
    ch[6] = ui->ch7->value() * coeff;
    ch[7] = ui->ch8->value() * coeff;
    ch[8] = ui->ch9->value() * coeff;
    ch[9] = ui->ch10->value() * coeff;

    armed = (ui->disarmed->isChecked()) ? 0 : (ui->arming->isChecked()) ? 1 : 2;
    fmode = ui->flightMode->value();

    QByteArray data;
    // 50 - current size of values, 4(quint32) + 10*4(float) + 2*1(quint8) + 4(quint32)
    data.resize(50);
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // magic header, "RCMD"
    stream << quint32(0x52434D44);
    // send channels
    for (int i = 0; i < 10; ++i) {
        stream << ch[i];
    }
    // send armed and mode
    stream << armed << fmode;
    // send readed counter
    stream << packetCounter;

    if (outSocket->writeDatagram(data, outHost, outPort) == -1) {
        qDebug() << "write failed: outHost" << outHost << " "
                 << "outPort " << outPort << " "
                 << outSocket->errorString();
    }
}

void Widget::on_autoSend_clicked()
{
    autoSendTimer->start(100);
    qDebug() << "timer start";
}

void Widget::on_autoAnswer_clicked()
{
    autoSendTimer->stop();
    qDebug() << "timer stop";
}

// transfomations

void Widget::asMatrix2Quat(const QMatrix4x4 &m, QQuaternion &q)
{
    qreal w, x, y, z;

    // w always >= 0
    w = qSqrt(qMax(0.0, 1.0 + m(0, 0) + m(1, 1) + m(2, 2))) / 2.0;
    x = qSqrt(qMax(0.0, 1.0 + m(0, 0) - m(1, 1) - m(2, 2))) / 2.0;
    y = qSqrt(qMax(0.0, 1.0 - m(0, 0) + m(1, 1) - m(2, 2))) / 2.0;
    z = qSqrt(qMax(0.0, 1.0 - m(0, 0) - m(1, 1) + m(2, 2))) / 2.0;

    x = copysign(x, (m(1, 2) - m(2, 1)));
    y = copysign(y, (m(2, 0) - m(0, 2)));
    z = copysign(z, (m(0, 1) - m(1, 0)));

    q.setScalar(w);
    q.setX(x);
    q.setY(y);
    q.setZ(z);
}

void Widget::asQuat2RPY(const QQuaternion &q, QVector3D &rpy)
{
    qreal roll;
    qreal pitch;
    qreal yaw;

    const qreal d2 = 2.0;
    const qreal qss = q.scalar() * q.scalar();
    const qreal qxx = q.x() * q.x();
    const qreal qyy = q.y() * q.y();
    const qreal qzz = q.z() * q.z();

    qreal test = -d2 * (q.x() * q.z() - q.scalar() * q.y());
    if (qFabs(test) > 0.998) {
        // ~86.3°, gimbal lock
        qreal R10 = d2 * (q.x() * q.y() - q.scalar() * q.z());
        qreal R11 = qss - qxx + qyy - qzz;

        roll = 0.0;
        pitch = copysign(M_PI_2, test);
        yaw = qAtan2(-R10, R11);
    } else {
        qreal R12 = d2 * (q.y() * q.z() + q.scalar() * q.x());
        qreal R22 = qss - qxx - qyy + qzz;
        qreal R01 = d2 * (q.x() * q.y() + q.scalar() * q.z());
        qreal R00 = qss + qxx - qyy - qzz;

        roll    = qAtan2(R12, R22);
        pitch   = qAsin(test);
        yaw     = qAtan2(R01, R00);
    }
    rpy.setX(RAD2DEG * roll);
    rpy.setY(RAD2DEG * pitch);
    rpy.setZ(RAD2DEG * yaw);
}

void Widget::asMatrix2RPY(const QMatrix4x4 &m, QVector3D &rpy)
{
    qreal roll;
    qreal pitch;
    qreal yaw;

    if (qFabs(m(0, 2)) > 0.998) {
        // ~86.3°, gimbal lock
        roll  = 0.0;
        pitch = copysign(M_PI_2, -m(0, 2));
        yaw   = qAtan2(-m(1, 0), m(1, 1));
    } else {
        roll  = qAtan2(m(1, 2), m(2, 2));
        pitch = qAsin(-m(0, 2));
        yaw   = qAtan2(m(0, 1), m(0, 0));
    }

    rpy.setX(roll  * RAD2DEG);
    rpy.setY(pitch * RAD2DEG);
    rpy.setZ(yaw   * RAD2DEG);
}

/* // not used

void Widget::ccRPY2Quat(const QVector3D &rpy, QQuaternion &q)
{
    float phi, theta, psi;
    float cphi, sphi, ctheta, stheta, cpsi, spsi;

    phi    = rpy.x() / 2;
    theta  = rpy.y() / 2;
    psi    = rpy.z() / 2;
    cphi   = cosf(phi);
    sphi   = sinf(phi);
    ctheta = cosf(theta);
    stheta = sinf(theta);
    cpsi   = cosf(psi);
    spsi   = sinf(psi);

    q.setScalar(cphi * ctheta * cpsi + sphi * stheta * spsi);
    q.setX(sphi * ctheta * cpsi - cphi * stheta * spsi);
    q.setY(cphi * stheta * cpsi + sphi * ctheta * spsi);
    q.setZ(cphi * ctheta * spsi - sphi * stheta * cpsi);

    if (q.scalar() < 0) {   // q0 always positive for uniqueness
        q.setScalar(-q.scalar());
        q.setX(-q.x());
        q.setY(-q.y());
        q.setZ(-q.z());
    }
}

void Widget::ccQuat2Matrix(const QQuaternion &q, QMatrix4x4 &m)
{
    float q0s = q.scalar() * q.scalar();
    float q1s = q.x() * q.x();
    float q2s = q.y() * q.y();
    float q3s = q.z() * q.z();

    float m00 = q0s + q1s - q2s - q3s;
    float m01 = 2 * (q.x() * q.y() + q.scalar() * q.z());
    float m02 = 2 * (q.x() * q.z() - q.scalar() * q.y());
    float m10 = 2 * (q.x() * q.y() - q.scalar() * q.z());
    float m11 = q0s - q1s + q2s - q3s;
    float m12 = 2 * (q.y() * q.z() + q.scalar() * q.x());
    float m20 = 2 * (q.x() * q.z() + q.scalar() * q.y());
    float m21 = 2 * (q.y() * q.z() - q.scalar() * q.x());
    float m22 = q0s - q1s - q2s + q3s;

    m = QMatrix4x4(m00, m01, m02, 0,
                   m10, m11, m12, 0,
                   m20, m21, m22, 0,
                   0,   0,   0,   1);
}
*/
