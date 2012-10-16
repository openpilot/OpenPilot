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

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

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
    udpCounterFGrecv = 0;
    udpCounterGCSsend = 0;
}

FGSimulator::~FGSimulator()
{
    disconnect(simProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyRead()));
}

void FGSimulator::setupUdpPorts(const QString& host, int inPort, int outPort)
{
    Q_UNUSED(outPort);

    if(inSocket->bind(QHostAddress(host), inPort))
        emit processOutput("Successfully bound to address " + host + " on port " + QString::number(inPort) + "\n");
    else
        emit processOutput("Cannot bind to address " + host + " on port " + QString::number(inPort) + "\n");
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
                 "--timeofday=noon " +
                 "--httpd=5400 " +
                 "--enable-hud " +
                 "--in-air " +
                 "--altitude=3000 " +
                 "--vc=100 " +
                 "--log-level=alert " +
                 "--generic=socket,out,20," + settings.hostAddress + "," + QString::number(settings.inPort) + ",udp,opfgprotocol");
    if(settings.manualControlEnabled) // <--[BCH] What does this do? Why does it depend on ManualControl?
    {
        args.append(" --generic=socket,in,400," + settings.remoteAddress + "," + QString::number(settings.outPort) + ",udp,opfgprotocol");
    }

    // Start FlightGear - only if checkbox is selected in HITL options page
    if (settings.startSim)
    {
        QString cmd("\"" + settings.binPath + "\" " + args + "\n");
        simProcess->write(cmd.toAscii());
    }
    else
    {
        emit processOutput("Start Flightgear from the command line with the following arguments: \n\n" + args + "\n\n" +
                           "You can optionally run Flightgear from a networked computer.\n" +
                           "Make sure the computer running Flightgear can can ping your local interface adapter. ie." + settings.hostAddress + "\n"
                           "Remote computer must have the correct OpenPilot protocol installed.");
    }

    udpCounterGCSsend = 0;

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
    ActuatorDesired::DataFields actData;
    FlightStatus::DataFields flightStatusData = flightStatus->getData();
    ManualControlCommand::DataFields manCtrlData = manCtrlCommand->getData();
    float ailerons = -1;
    float elevator = -1;
    float rudder = -1;
    float throttle = -1;

    if(flightStatusData.FlightMode == FlightStatus::FLIGHTMODE_MANUAL)
    {
        // Read joystick input
        if(flightStatusData.Armed == FlightStatus::ARMED_ARMED)
        {
            // Note: Pitch sign is reversed in FG ?
            ailerons = manCtrlData.Roll;
            elevator = -manCtrlData.Pitch;
            rudder = manCtrlData.Yaw;
            throttle = manCtrlData.Throttle;
        }
    }
    else
    {
        // Read ActuatorDesired from autopilot
        actData = actDesired->getData();

        ailerons = actData.Roll;
        elevator = -actData.Pitch;
        rudder = actData.Yaw;
        throttle = actData.Throttle;
    }

    int allowableDifference = 10;

    //qDebug() << "UDP sent:" << udpCounterGCSsend << " - UDP Received:" << udpCounterFGrecv;

    if(udpCounterFGrecv == udpCounterGCSsend)
        udpCounterGCSsend = 0;
    
    if((udpCounterGCSsend < allowableDifference) || (udpCounterFGrecv==0) ) //FG udp queue is not delayed
    {
        udpCounterGCSsend++;

        // Send update to FlightGear
        QString cmd;
        cmd = QString("%1,%2,%3,%4,%5\n")
                .arg(ailerons) //ailerons
                .arg(elevator) //elevator
                .arg(rudder) //rudder
                .arg(throttle) //throttle
                .arg(udpCounterGCSsend); //UDP packet counter delay

        QByteArray data = cmd.toAscii();

        if(outSocket->writeDatagram(data, QHostAddress(settings.remoteAddress), settings.outPort) == -1)
        {
            emit processOutput("Error sending UDP packet to FG: " + outSocket->errorString() + "\n");
        }
    }
    else
    {
        // don't send new packet. Flightgear cannot process UDP fast enough.
        // V1.9.1 reads udp packets at set frequency and will get delayed if packets are sent too fast
        // V2.0 does not currently work with --generic-protocol
    }
    
    if(settings.manualControlEnabled)
    {
        actData.Roll = ailerons;
        actData.Pitch = -elevator;
        actData.Yaw = rudder;
        actData.Throttle = throttle;
        //actData.NumLongUpdates = (float)udpCounterFGrecv;
        //actData.UpdateTime = (float)udpCounterGCSsend;
        actDesired->setData(actData);
    }
}


void FGSimulator::processUpdate(const QByteArray& inp)
{
    //TODO: this does not use the FLIGHT_PARAM structure, it should!
    // Split
    QString data(inp);
    QStringList fields = data.split(",");
    // Get xRate (deg/s)
    //        float xRate = fields[0].toFloat() * 180.0/M_PI;
    // Get yRate (deg/s)
    //        float yRate = fields[1].toFloat() * 180.0/M_PI;
    // Get zRate (deg/s)
    //        float zRate = fields[2].toFloat() * 180.0/M_PI;
    // Get xAccel (m/s^2)
    float xAccel = fields[3].toFloat() * FT2M;
    // Get yAccel (m/s^2)
    float yAccel = fields[4].toFloat() * FT2M;
    // Get xAccel (m/s^2)
    float zAccel = fields[5].toFloat() * FT2M;
    // Get pitch (deg)
    float pitch = fields[6].toFloat();
    // Get pitchRate (deg/s)
    float pitchRate = fields[7].toFloat();
    // Get roll (deg)
    float roll = fields[8].toFloat();
    // Get rollRate (deg/s)
    float rollRate = fields[9].toFloat();
    // Get yaw (deg)
    float yaw = fields[10].toFloat();
    // Get yawRate (deg/s)
    float yawRate = fields[11].toFloat();
    // Get latitude (deg)
    float latitude = fields[12].toFloat();
    // Get longitude (deg)
    float longitude = fields[13].toFloat();
    // Get heading (deg)
    float heading = fields[14].toFloat();
    // Get altitude (m)
    float altitude_msl = fields[15].toFloat() * FT2M;
    // Get altitudeAGL (m)
    float altitude_agl = fields[16].toFloat() * FT2M;
    // Get groundspeed (m/s)
    float groundspeed = fields[17].toFloat() * KT2MPS;
    // Get airspeed (m/s)
    float airspeed = fields[18].toFloat() * KT2MPS;
    // Get temperature (degC)
    float temperature = fields[19].toFloat();
    // Get pressure (kpa)
    float pressure = fields[20].toFloat() * INHG2KPA;
    // Get VelocityActual Down (cm/s)
    float velocityActualDown = - fields[21].toFloat() * FPS2CMPS;
    // Get VelocityActual East (cm/s)
    float velocityActualEast = fields[22].toFloat() * FPS2CMPS;
    // Get VelocityActual Down (cm/s)
    float velocityActualNorth = fields[23].toFloat() * FPS2CMPS;

    // Get UDP packets received by FG
    int n = fields[24].toInt();
    udpCounterFGrecv = n;

    ///////
    // Output formatting
    ///////
    Output2Hardware out;
    memset(&out, 0, sizeof(Output2Hardware));

    float NED[3];
    // convert from cm back to meters

    double LLA[3] = {latitude, longitude, altitude_msl};
    double ECEF[3];
    double RNE[9];
    Utils::CoordinateConversions().RneFromLLA(LLA,(double (*)[3])RNE);
    Utils::CoordinateConversions().LLA2ECEF(LLA,ECEF);
    Utils::CoordinateConversions().LLA2Base(LLA, ECEF, (float (*)[3]) RNE, NED);


    // Update GPS Position objects
    out.latitude = latitude * 1e7;
    out.longitude = longitude * 1e7;
    out.altitude = altitude_msl;
    out.agl = altitude_agl;
    out.groundspeed = groundspeed;

    out.calibratedAirspeed = airspeed;


    // Update BaroAltitude object
    out.temperature = temperature;
    out.pressure = pressure;

    // Update attActual object
    out.roll = roll;       //roll;
    out.pitch = pitch;     // pitch
    out.heading = yaw; // yaw

    out.dstN= NED[0];
    out.dstE= NED[1];
    out.dstD= NED[2];

    // Update VelocityActual.{North,East,Down}
    out.velNorth = velocityActualNorth;
    out.velEast = velocityActualEast;
    out.velDown = velocityActualDown;

    //Update gyroscope sensor data
    out.rollRate = rollRate;
    out.pitchRate = pitchRate;
    out.yawRate = yawRate;

    //Update accelerometer sensor data
    out.accX = xAccel;
    out.accY = yAccel;
    out.accZ = -zAccel;

    updateUAVOs(out);
}

