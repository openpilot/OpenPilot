/**
 ******************************************************************************
 *
 * @file       simulator.cpp
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


#include "simulator.h"
#include "qxtlogger.h"
#include "extensionsystem/pluginmanager.h"
#include "coreplugin/icore.h"
#include "coreplugin/threadmanager.h"

volatile bool Simulator::isStarted = false;

const float Simulator::GEE = 9.81;
const float Simulator::FT2M = 0.3048;
const float Simulator::KT2MPS = 0.514444444;
const float Simulator::INHG2KPA = 3.386;
const float Simulator::FPS2CMPS = 30.48;
const float Simulator::DEG2RAD = (M_PI/180.0);


Simulator::Simulator(const SimulatorSettings& params) :
	simProcess(NULL),
	time(NULL),
	inSocket(NULL),
	outSocket(NULL),
	settings(params),
        updatePeriod(50),
        simTimeout(2000),
	autopilotConnectionStatus(false),
	simConnectionStatus(false),
	txTimer(NULL),
	simTimer(NULL),
	name("")
{
	// move to thread
	moveToThread(Core::ICore::instance()->threadManager()->getRealTimeThread());
        connect(this, SIGNAL(myStart()), this, SLOT(onStart()),Qt::QueuedConnection);
	emit myStart();
}

Simulator::~Simulator()
{
	if(inSocket)
	{
		delete inSocket;
		inSocket = NULL;
	}

	if(outSocket)
	{
		delete outSocket;
		outSocket = NULL;
	}

	if(txTimer)
	{
		delete txTimer;
		txTimer = NULL;
	}

	if(simTimer)
	{
		delete simTimer;
		simTimer = NULL;
	}
	// NOTE: Does not currently work, may need to send control+c to through the terminal
	if (simProcess != NULL)
	{
		//connect(simProcess,SIGNAL(finished(int, QProcess::ExitStatus)),this,SLOT(onFinished(int, QProcess::ExitStatus)));

		simProcess->disconnect();
		if(simProcess->state() == QProcess::Running)
			simProcess->kill();
		//if(simProcess->waitForFinished())
			//emit deleteSimProcess();
		delete simProcess;
		simProcess = NULL;
	}
}

void Simulator::onDeleteSimulator(void)
{
	// [1]
	Simulator::setStarted(false);
	// [2]
	Simulator::Instances().removeOne(simulatorId);

	disconnect(this);
	delete this;
}

void Simulator::onStart()
{
	QMutexLocker locker(&lock);

        QThread* mainThread = QThread::currentThread();
	qDebug() << "Simulator Thread: "<< mainThread;

	// Get required UAVObjects
	ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
	UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
	actDesired = ActuatorDesired::GetInstance(objManager);
        manCtrlCommand = ManualControlCommand::GetInstance(objManager);
        flightStatus = FlightStatus::GetInstance(objManager);
	posHome = HomeLocation::GetInstance(objManager);
	velActual = VelocityActual::GetInstance(objManager);
	posActual = PositionActual::GetInstance(objManager);
        altActual = BaroAltitude::GetInstance(objManager);
	attActual = AttitudeActual::GetInstance(objManager);
	attRaw = AttitudeRaw::GetInstance(objManager);
        gpsPos = GPSPosition::GetInstance(objManager);
	telStats = GCSTelemetryStats::GetInstance(objManager);

	// Listen to autopilot connection events
	TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
	connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
	connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));
	//connect(telStats, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(telStatsUpdated(UAVObject*)));

	// If already connect setup autopilot
	GCSTelemetryStats::DataFields stats = telStats->getData();
	if ( stats.Status == GCSTelemetryStats::STATUS_CONNECTED )
		onAutopilotConnect();

	inSocket = new QUdpSocket();
	outSocket = new QUdpSocket();
	setupUdpPorts(settings.hostAddress,settings.inPort,settings.outPort);

        emit processOutput("\nLocal interface: " + settings.hostAddress + "\n" + \
                           "Remote interface: " + settings.remoteHostAddress + "\n" + \
                           "inputPort: " + QString::number(settings.inPort) + "\n" + \
                           "outputPort: " + QString::number(settings.outPort) + "\n");

        qxtLog->info("\nLocal interface: " + settings.hostAddress + "\n" + \
                     "Remote interface: " + settings.remoteHostAddress + "\n" + \
                     "inputPort: " + QString::number(settings.inPort) + "\n" + \
                     "outputPort: " + QString::number(settings.outPort) + "\n");

//        if(!inSocket->waitForConnected(5000))
//                emit processOutput(QString("Can't connect to %1 on %2 port!").arg(settings.hostAddress).arg(settings.inPort));
//        outSocket->connectToHost(settings.hostAddress,settings.outPort); // FG
//        if(!outSocket->waitForConnected(5000))
//                emit processOutput(QString("Can't connect to %1 on %2 port!").arg(settings.hostAddress).arg(settings.outPort));


	connect(inSocket, SIGNAL(readyRead()), this, SLOT(receiveUpdate()),Qt::DirectConnection);

	// Setup transmit timer
	txTimer = new QTimer();
	connect(txTimer, SIGNAL(timeout()), this, SLOT(transmitUpdate()),Qt::DirectConnection);
	txTimer->setInterval(updatePeriod);
	txTimer->start();
	// Setup simulator connection timer
	simTimer = new QTimer();
	connect(simTimer, SIGNAL(timeout()), this, SLOT(onSimulatorConnectionTimeout()),Qt::DirectConnection);
	simTimer->setInterval(simTimeout);
	simTimer->start();

	// setup time
	time = new QTime();
	time->start();
	current.T=0;
	current.i=0;

}

void Simulator::receiveUpdate()
{
	// Update connection timer and status
	simTimer->setInterval(simTimeout);
	simTimer->stop();
	simTimer->start();
	if ( !simConnectionStatus )
	{
		simConnectionStatus = true;
		emit simulatorConnected();
	}

	// Process data
        while(inSocket->hasPendingDatagrams()) {
		// Receive datagram
		QByteArray datagram;
		datagram.resize(inSocket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;
		inSocket->readDatagram(datagram.data(), datagram.size(),
							   &sender, &senderPort);
		//QString datastr(datagram);
		// Process incomming data
		processUpdate(datagram);
	 }
}

void Simulator::setupObjects()
{
	setupInputObject(actDesired, 100);
	setupOutputObject(altActual, 250);
        setupOutputObject(attActual, 10);
        //setupOutputObject(attActual, 100);
        setupOutputObject(gpsPos, 250);
        setupOutputObject(posActual, 250);
        setupOutputObject(velActual, 250);
        setupOutputObject(posHome, 1000);
        setupOutputObject(attRaw, 10);
        //setupOutputObject(attRaw, 100);



}

void Simulator::setupInputObject(UAVObject* obj, int updatePeriod)
{
	UAVObject::Metadata mdata;
	mdata = obj->getDefaultMetadata();
	mdata.flightAccess = UAVObject::ACCESS_READWRITE;
	mdata.gcsAccess = UAVObject::ACCESS_READWRITE;
	mdata.flightTelemetryAcked = false;
	mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
	mdata.flightTelemetryUpdatePeriod = updatePeriod;
	mdata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
	obj->setMetadata(mdata);
}

void Simulator::setupOutputObject(UAVObject* obj, int updatePeriod)
{
	UAVObject::Metadata mdata;
	mdata = obj->getDefaultMetadata();
	mdata.flightAccess = UAVObject::ACCESS_READONLY;
	mdata.gcsAccess = UAVObject::ACCESS_READWRITE;
	mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_NEVER;
	mdata.gcsTelemetryAcked = false;
	mdata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
	mdata.gcsTelemetryUpdatePeriod = updatePeriod;
	obj->setMetadata(mdata);
}

void Simulator::onAutopilotConnect()
{
	autopilotConnectionStatus = true;
	setupObjects();
	emit autopilotConnected();
}

void Simulator::onAutopilotDisconnect()
{
	autopilotConnectionStatus = false;
	emit autopilotDisconnected();
}

void Simulator::onSimulatorConnectionTimeout()
{
	if ( simConnectionStatus )
	{
		simConnectionStatus = false;
		emit simulatorDisconnected();
	}
}


void Simulator::telStatsUpdated(UAVObject* obj)
{
	GCSTelemetryStats::DataFields stats = telStats->getData();
	if ( !autopilotConnectionStatus && stats.Status == GCSTelemetryStats::STATUS_CONNECTED )
	{
		onAutopilotConnect();
	}
	else if ( autopilotConnectionStatus && stats.Status != GCSTelemetryStats::STATUS_CONNECTED )
	{
		onAutopilotDisconnect();
	}
}

