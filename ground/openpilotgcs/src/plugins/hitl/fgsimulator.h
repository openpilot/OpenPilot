/**
 ******************************************************************************
 *
 * @file       flightgearbridge.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup HITLPlugin HITL Plugin
 * @{
 * @brief The Hardware In The Loop plugin 
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

#ifndef FGSIMULATOR_H
#define FGSIMULATOR_H_H

#include <QObject>
#include "simulator.h"

class FGSimulator: public Simulator
{
    Q_OBJECT

public:
	FGSimulator(const SimulatorSettings& params);
	~FGSimulator();

	bool setupProcess();
	void setupUdpPorts(const QString& host, int inPort, int outPort);

private slots:
    void transmitUpdate();
	void processReadyRead();

private:

    int udpCounterGCSsend; //keeps track of udp packets sent to FG
    int udpCounterFGrecv; //keeps track of udp packets received by FG

	void processUpdate(const QByteArray& data);
};

class FGSimulatorCreator : public SimulatorCreator
{
public:
	FGSimulatorCreator(const QString& classId, const QString& description)
	:  SimulatorCreator (classId,description)
	{}

	Simulator* createSimulator(const SimulatorSettings& params)
	{
		return new FGSimulator(params);
	}

};
#endif // FGSIMULATOR_H
