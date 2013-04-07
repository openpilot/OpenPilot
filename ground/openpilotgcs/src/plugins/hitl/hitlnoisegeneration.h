/**
 ******************************************************************************
 *
 * @file       hitlnoisegeneration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlplugin
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

#ifndef HITLNOISEGENERATION_H
#define HITLNOISEGENERATION_H

//#include <QObject>
//#include <simulator.h>
#include "xplanesimulator.h"
#include "hitlnoisegeneration.h"
#include "extensionsystem/pluginmanager.h"
#include <coreplugin/icore.h>
#include <coreplugin/threadmanager.h>

struct Noise{
    Accels::DataFields accelData;
    AttitudeActual::DataFields attActualData;
    BaroAltitude::DataFields baroAltData;
    AirspeedActual::DataFields airspeedActual;
    GPSPosition::DataFields gpsPosData;
    GPSVelocity::DataFields gpsVelData;
    Gyros::DataFields gyroData;
    HomeLocation::DataFields homeData;
    PositionActual::DataFields positionActualData;
    VelocityActual::DataFields velocityActualData;
};

class HitlNoiseGeneration
{
//	Q_OBJECT
public:
    HitlNoiseGeneration();
    ~HitlNoiseGeneration();

    Noise getNoise();
    Noise generateNoise();
private slots:

private:
    Noise noise;
};
#endif // HITLNOISEGENERATION_H
