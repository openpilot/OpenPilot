/**
 ******************************************************************************
 *
 * @file       levellingutil.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup LevellingUtil
 * @{
 * @brief
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

#ifndef LEVELLINGUTIL_H
#define LEVELLINGUTIL_H

#include <QObject>
#include <QTimer>
#include <QMutex>

#include "uavobject.h"
#include "vehicleconfigurationsource.h"

class LevellingUtil : public QObject
{
    Q_OBJECT
public:
    explicit LevellingUtil(long measurementCount, long measurementPeriod);
        
signals:
    void progress(long current, long total);
    void done(accelGyroBias measuredBias);
    void timeout(QString message);

public slots:
    void start();
    void abort();

private slots:
    void measurementsUpdated(UAVObject * obj);
    void timeout();

private:
    static const float G = 9.81f;
    static const float ACCELERATION_SCALE = 0.004f * 9.81f;

    QMutex m_measurementMutex;
    QTimer m_timeoutTimer;

    bool m_isMeasuring;
    long m_receivedUpdates;

    long m_measurementCount;
    long m_measurementPeriod;

    UAVObject::Metadata m_previousMetaData;

    QList<double> m_accelerometerX;
    QList<double> m_accelerometerY;
    QList<double> m_accelerometerZ;
    QList<double> m_gyroX;
    QList<double> m_gyroY;
    QList<double> m_gyroZ;

    void stop();
    void startMeasurement();
    void stopMeasurement();
    accelGyroBias calculateLevellingData();
    double listMean(QList<double> list);
};

#endif // LEVELLINGUTIL_H
