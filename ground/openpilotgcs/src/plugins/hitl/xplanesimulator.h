/**
 ******************************************************************************
 *
 * @file       xplanesimulator.h
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

#ifndef XPLANESIMULATOR_H
#define XPLANESIMULATOR_H

#include <QObject>
#include <simulator.h>

class XplaneSimulator: public Simulator
{
	Q_OBJECT
public:
	XplaneSimulator(const SimulatorSettings& params);
	~XplaneSimulator();
        bool setupProcess();

	void setupUdpPorts(const QString& host, int inPort, int outPort);

private slots:
	void transmitUpdate();

private:
    enum XplaneOutputData //***WARNING***: Elements in this enum are in a precise order, do
    {                     // not change. Cf. http://www.nuclearprojects.com/xplane/info.shtml
		FramRate,
		Times,
		SimStats,
		Speed,
		Gload,
		AtmosphereWeather,
		AtmosphereAircraft,
		SystemPressures,
		Joystick1,
		Joystick2,
		ArtStab,
		FlightCon,
		WingSweep,
		Trim,
		Brakes,
		AngularMoments,
		AngularAccelerations,
        AngularVelocities,
        PitchRollHeading,
		AoA,
        LatitudeLongitudeAltitude,
		LocVelDistTraveled
	};

	void processUpdate(const QByteArray& data);

};

class XplaneSimulatorCreator : public SimulatorCreator
{
public:
	XplaneSimulatorCreator(const QString& classId, const QString& description)
	:  SimulatorCreator (classId,description)
	{}

	Simulator* createSimulator(const SimulatorSettings& params)
	{
		return new XplaneSimulator(params);
	}
};

#endif // XPLANESIMULATOR_H
