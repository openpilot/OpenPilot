/**
 ******************************************************************************
 *
 * @file       il2simulator.h
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

#ifndef IL2SIMULATOR_H
#define IL2SIMULATOR_H

#include <QObject>
#include <simulator.h>

class IL2Simulator: public Simulator
{
	Q_OBJECT
public:
	IL2Simulator(const SimulatorSettings& params);
	~IL2Simulator();

	void setupUdpPorts(const QString& host, int inPort, int outPort);

private slots:
	void transmitUpdate();

private:
    static const float FT2M;
    static const float KT2MPS;
    static const float MPS2KMH;
    static const float KMH2MPS;
    static const float INHG2KPA;
    static const float RAD2DEG;
    static const float DEG2RAD;
    static const float NM2DEG;
    static const float DEG2NM;

	void processUpdate(const QByteArray& data);
	float angleDifference(float a,float b);

    AirParameters airParameters;
};

class IL2SimulatorCreator : public SimulatorCreator
{
public:
	IL2SimulatorCreator(const QString& classId, const QString& description)
	:  SimulatorCreator (classId,description)
	{}

	Simulator* createSimulator(const SimulatorSettings& params)
	{
		return new IL2Simulator(params);
	}
};

#endif // IL2SIMULATOR_H
