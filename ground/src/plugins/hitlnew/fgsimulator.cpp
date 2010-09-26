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

#include "fgsimulator.h"
#include "extensionsystem/pluginmanager.h"
#include "coreplugin/icore.h"
#include "coreplugin/threadmanager.h"

const float FGSimulator::FT2M = 0.3048;
const float FGSimulator::KT2MPS = 0.514444444;
const float FGSimulator::INHG2KPA = 3.386;

//FGSimulator::FGSimulator(QString hostAddr, int outPort, int inPort, bool manual, QString binPath, QString dataPath) :
//		Simulator(hostAddr, outPort, inPort,  manual, binPath, dataPath),
//		fgProcess(NULL)
//{
//	// Note: Only tested on windows 7
//#if defined(Q_WS_WIN)
//	cmdShell = QString("c:/windows/system32/cmd.exe");
//#else
//	cmdShell = QString("bash");
//#endif
//}

FGSimulator::FGSimulator(const SimulatorSettings& params) :
		Simulator(params)
{
}

FGSimulator::~FGSimulator()
{
	disconnect(simProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyRead()));
}

void FGSimulator::setupUdpPorts(const QString& host, int inPort, int outPort)
{
	inSocket->bind(QHostAddress(host), inPort);
}

bool FGSimulator::setupProcess()
{
	QMutexLocker locker(&lock);

	// Copy FlightGear generic protocol configuration file to the FG protocol directory
	// NOTE: Not working on Windows 7, if FG is installed in the "Program Files",
	// likelly due to permissions. The file should be manually copied to data/Protocol/opfgprotocol.xml
//	QFile xmlFile(":/flightgear/genericprotocol/opfgprotocol.xml");
//	xmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
//	QString xml = xmlFile.readAll();
//	xmlFile.close();
//	QFile xmlFileOut(pathData + "/Protocol/opfgprotocol.xml");
//	xmlFileOut.open(QIODevice::WriteOnly | QIODevice::Text);
//	xmlFileOut.write(xml.toAscii());
//	xmlFileOut.close();

	Qt::HANDLE mainThread = QThread::currentThreadId();
	qDebug() << "setupProcess Thread: "<< mainThread;

	simProcess = new QProcess();
	simProcess->setReadChannelMode(QProcess::MergedChannels);
	connect(simProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyRead()));
	// Note: Only tested on windows 7
#if defined(Q_WS_WIN)
	QString cmdShell("c:/windows/system32/cmd.exe");
#else
	QString cmdShell("bash");
#endif

	// Start shell (Note: Could not start FG directly on Windows, only through terminal!)
	simProcess->start(cmdShell);
	if (simProcess->waitForStarted() == false)
	{
		emit processOutput("Error:" + simProcess->errorString());
		return false;
	}

	// Setup arguments
	// Note: The input generic protocol is set to update at a much higher rate than the actual updates are sent by the GCS.
	// If this is not done then a lag will be introduced by FlightGear, likelly because the receive socket buffer builds up during startup.
        QString args("--fg-root=\"" + settings.dataPath + "\" " +
                     "--timeofday=dusk " +
                     "--httpd=5400 " +
                     "--enable-hud " +
                     "--in-air " +
                     "--altitude=2000 " +
                     "--vc=100 " +
                     "--generic=socket,out,50,localhost," + QString::number(settings.inPort) + ",udp,opfgprotocol");
	if(!settings.manual)
	{
            args.append(" --generic=socket,in,400,localhost," + QString::number(settings.outPort) + ",udp,opfgprotocol");
	}

	// Start FlightGear
	QString cmd("\"" + settings.binPath + "\" " + args + "\n");
	simProcess->write(cmd.toAscii());

	return true;
}

void FGSimulator::processReadyRead()
{
	QByteArray bytes = simProcess->readAllStandardOutput();
	QString str(bytes);
	if ( !str.contains("Error reading data") ) // ignore error
	{
		emit processOutput(str);
	}
}

void FGSimulator::transmitUpdate()
{
	// Read ActuatorDesired from autopilot
	ActuatorDesired::DataFields actData = actDesired->getData();
	float ailerons = actData.Roll;
	float elevator = -actData.Pitch;
	float rudder = actData.Yaw;
	float throttle = actData.Throttle;
	// Send update to FlightGear
	QString cmd;
	cmd = QString("%1,%2,%3,%4\n")
		  .arg(ailerons)
		  .arg(elevator)
		  .arg(rudder)
		  .arg(throttle);
	QByteArray data = cmd.toAscii();
	outSocket->writeDatagram(data, QHostAddress(settings.hostAddress), settings.outPort);
}


void FGSimulator::processUpdate(const QByteArray& inp)
{
	// Split
	QString data(inp);
	QStringList fields = data.split(",");
	// Get xRate (deg/s)
//	float xRate = fields[0].toFloat() * 180.0/M_PI;
	// Get yRate (deg/s)
//	float yRate = fields[1].toFloat() * 180.0/M_PI;
	// Get zRate (deg/s)
//	float zRate = fields[2].toFloat() * 180.0/M_PI;
	// Get xAccel (m/s^2)
//	float xAccel = fields[3].toFloat() * FT2M;
	// Get yAccel (m/s^2)
//	float yAccel = fields[4].toFloat() * FT2M;
	// Get xAccel (m/s^2)
//	float zAccel = fields[5].toFloat() * FT2M;
	// Get pitch (deg)
	float pitch = fields[6].toFloat();
	// Get pitchRate (deg/s)
//	float pitchRate = fields[7].toFloat();
	// Get roll (deg)
	float roll = fields[8].toFloat();
	// Get rollRate (deg/s)
//	float rollRate = fields[9].toFloat();
	// Get yaw (deg)
	float yaw = fields[10].toFloat();
	// Get yawRate (deg/s)
//	float yawRate = fields[11].toFloat();
	// Get latitude (deg)
	float latitude = fields[12].toFloat();
	// Get longitude (deg)
	float longitude = fields[13].toFloat();
	// Get heading (deg)
	float heading = fields[14].toFloat();
	// Get altitude (m)
	float altitude = fields[15].toFloat() * FT2M;
	// Get altitudeAGL (m)
	float altitudeAGL = fields[16].toFloat() * FT2M;
	// Get groundspeed (m/s)
	float groundspeed = fields[17].toFloat() * KT2MPS;
	// Get airspeed (m/s)
//	float airspeed = fields[18].toFloat() * KT2MPS;
	// Get temperature (degC)
	float temperature = fields[19].toFloat();
	// Get pressure (kpa)
	float pressure = fields[20].toFloat() * INHG2KPA;

	// Update AltitudeActual object
        BaroAltitude::DataFields altActualData;
        memset(&altActualData, 0, sizeof(BaroAltitude::DataFields));
        altActualData.Altitude = altitudeAGL;
	altActualData.Temperature = temperature;
	altActualData.Pressure = pressure;
	altActual->setData(altActualData);

	// Update attActual object
	AttitudeActual::DataFields attActualData;
        memset(&attActualData, 0, sizeof(AttitudeActual::DataFields));
	attActualData.Roll = roll;
	attActualData.Pitch = pitch;
	attActualData.Yaw = yaw;
	attActualData.q1 = 0;
	attActualData.q2 = 0;
	attActualData.q3 = 0;
	attActualData.q4 = 0;
	attActual->setData(attActualData);

	// Update gps objects
        GPSPosition::DataFields gpsData;
        memset(&gpsData, 0, sizeof(GPSPosition::DataFields));
        gpsData.Altitude = altitude;
	gpsData.Heading = heading;
	gpsData.Groundspeed = groundspeed;
	gpsData.Latitude = latitude;
	gpsData.Longitude = longitude;
	gpsData.Satellites = 10;
        gpsData.Status = GPSPosition::STATUS_FIX3D;
        gpsPos->setData(gpsData);
}

