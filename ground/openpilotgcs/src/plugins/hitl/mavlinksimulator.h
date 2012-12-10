/**
 ******************************************************************************
 *
 * @file       mavlinksimulator.h
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

#ifndef MAVLINKSIMULATOR_H
#define MAVLINKSIMULATOR_H

#include <QObject>
#include <simulator.h>

class MavlinkSimulator: public Simulator
{
	Q_OBJECT
public:
	MavlinkSimulator(const SimulatorSettings& params);
	~MavlinkSimulator();
        bool setupProcess();

	void setupUdpPorts(const QString& host, int inPort, int outPort);

private slots:
	void transmitUpdate();

private:
        bool once;
        float initX;
        float initY;
        float initZ;
	enum MavlinkOutputData
	{
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
		LatitudeLongitude,
		LocVelDistTraveled
	};

	void processUpdate(const QByteArray& data);

};

class MavlinkSimulatorCreator : public SimulatorCreator
{
public:
	MavlinkSimulatorCreator(const QString& classId, const QString& description)
	:  SimulatorCreator (classId,description)
	{}

	Simulator* createSimulator(const SimulatorSettings& params)
	{
		return new MavlinkSimulator(params);
	}
};

#endif // MAVLINKSIMULATOR_H
