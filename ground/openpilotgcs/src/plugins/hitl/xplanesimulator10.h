/**
 ******************************************************************************
 *
 * @file       xplanesimulator10.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#ifndef XPLANESIMULATOR10_H
#define XPLANESIMULATOR10_H

#include <QObject>
#include <simulator.h>

class XplaneSimulator10 : public Simulator {
    Q_OBJECT
public:
    XplaneSimulator10(const SimulatorSettings & params);
    ~XplaneSimulator10();
    bool setupProcess();

    void setupUdpPorts(const QString & host, int inPort, int outPort);

private slots:
    void transmitUpdate();

private:
    enum XplaneOutputData // ***WARNING***: Elements in this enum are in a precise order, do
    { // not change. Cf. http://www.nuclearprojects.com/xplane/info.shtml (outdated version 9 info)
      // These fields have been updated for X-Plane version 10.x
      /* 0 */ FramRate,
        /* 1 */ Times,
        /* 2 */ SimStats,
        /* 3 */ Speed,
        /* 4 */ Gload,
        /* 5 */ AtmosphereWeather,
        /* 6 */ AtmosphereAircraft,
        /* 7 */ SystemPressures,
        /* 8 */ Joystick1,
        /* 9 */ Joystick2,
        /* 10 */ ArtStab,
        /* 11 */ FlightCon,
        /* 12 */ WingSweep,
        /* 13 */ Trim,
        /* 14 */ Brakes,
        /* 15 */ AngularMoments,
        /* 16 */ AngularVelocities,
        /* 17 */ PitchRollHeading,
        /* 18 */ AoA,
        /* 19 */ MagCompass,
        /* 20 */ LatitudeLongitude,
        /* 21 */ LocVelDistTraveled,
        /* 22 */ AllPlanesLat,
        /* 23 */ AllPlanesLon,
        /* 24 */ AllPlanesAlt,
        /* 25 */ ThrottleCommand
        /* .. */
    };

    void processUpdate(const QByteArray & data);
};

class XplaneSimulatorCreator10 : public SimulatorCreator {
public:
    XplaneSimulatorCreator10(const QString & classId, const QString & description)
        :  SimulatorCreator(classId, description)
    {}

    Simulator *createSimulator(const SimulatorSettings & params)
    {
        return new XplaneSimulator10(params);
    }
};

#endif // XPLANESIMULATOR10_H
