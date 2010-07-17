/**
 ******************************************************************************
 *
 * @file       il2bridge.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlil2plugin
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

/*
 * Description of DeviceLink Protocol:
 * A Request is initiated with R/ followed by id's of to be requested settings
 * even id's indicate read only values, odd are write only
 * (usually id =get value id+1= set - for same setting)
 * id's are separated by /
 * requests can contain values to set, or to select a subsystem
 * values are separated by \
 * example: R/30/48/64\0/64\1/
 * request read only settings 30,48 and 64 with parameters 0 and 1
 * the answer consists of an A followed by id value pairs in the same format
 * example: A/30\0/48\0/64\0\22/64\1\102/
 *
 * A full protocol description as well as a list of ID's and their meanings
 * can be found shipped with IL2 in the file DeviceLink.txt
 *
 * id's used in this file:
 * 30: IAS in km/h (float)
 * 32: vario in m/s (float)
 * 38: angular speed °/s (float) (which direction? azimuth?)
 * 40: barometric alt in m (float)
 * 42: flight course in ° (0-360) (float)
 * 46: roll angle in ° (-180 - 180) (floatniguration)
 * 48: pitch angle in ° (-90 - 90) (float)
 * 80/81: engine power  (-1.0 (0%) - 1.0 (100%)) (float)
 * 84/85: aileron servo (-1.0 - 1.0) (float)
 * 86/87: elevator servo (-1.0 - 1.0) (float)
 * 88/89: rudder servo (-1.0 - 1.0) (float)
 *
 * IL2 currently offers no useful way of providing GPS data
 * therefore fake GPS data will be calculated using IMS
 *
 * unfortunately angular acceleration provided is very limited, too
 */
#include "il2bridge.h"
#include "extensionsystem/pluginmanager.h"
#include <math.h>

Il2Bridge::Il2Bridge(QString il2HostName, int il2Port, QString il2Latitude, QString il2Longitude)
{
    // Init fields
    il2Host = QHostAddress(il2HostName);
    outPort = il2Port;
    updatePeriod = 50;
    il2Timeout = 2000;
    autopilotConnectionStatus = false;
    il2ConnectionStatus = false;
    latitude=il2Latitude.toFloat();
    longitude=il2Longitude.toFloat();

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    actDesired = ActuatorDesired::GetInstance(objManager);
    altActual = AltitudeActual::GetInstance(objManager);
    attActual = AttitudeActual::GetInstance(objManager);
    posActual = PositionActual::GetInstance(objManager);
    telStats = GCSTelemetryStats::GetInstance(objManager);

    // Listen to autopilot connection events
    TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));
    //connect(telStats, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(telStatsUpdated(UAVObject*)));

    // If already connect setup autopilot
    GCSTelemetryStats::DataFields stats = telStats->getData();
    if ( stats.Status == GCSTelemetryStats::STATUS_CONNECTED )
    {
        onAutopilotConnect();
    }

    // Setup local ports
    outSocket = new QUdpSocket(this);
    outSocket->connectToHost((const QString )il2HostName,il2Port);
    connect(outSocket, SIGNAL(readyRead()), this, SLOT(receiveUpdate()));

    // Setup transmit timer
    txTimer = new QTimer(this);
    connect(txTimer, SIGNAL(timeout()), this, SLOT(transmitUpdate()));
    txTimer->setInterval(updatePeriod);
    txTimer->start();

    // Setup FG connection timer
    il2Timer = new QTimer(this);
    connect(il2Timer, SIGNAL(timeout()), this, SLOT(onIl2ConnectionTimeout()));
    il2Timer->setInterval(il2Timeout);
    il2Timer->start();

    // setup time
    time = new QTime();
    time->start();

    current.T=0;
}

Il2Bridge::~Il2Bridge()
{
    delete outSocket;
    delete txTimer;
    delete il2Timer;
}

bool Il2Bridge::isAutopilotConnected()
{
    return autopilotConnectionStatus;
}

bool Il2Bridge::isIl2Connected()
{
    return il2ConnectionStatus;
}

void Il2Bridge::transmitUpdate()
{
    // Read ActuatorDesired from autopilot
    ActuatorDesired::DataFields actData = actDesired->getData();
    float ailerons = actData.Roll;
    float elevator = actData.Pitch;
    float rudder = actData.Yaw;
    float throttle = actData.Throttle*2-1.0;

    // Send update to Il2
    QString cmd;
    cmd=QString("R/30/32/40/42/46/48/81\\%1/85\\%2/87\\%3/89\\%4/")
    	.arg(throttle)
    	.arg(ailerons)
    	.arg(elevator)
    	.arg(rudder);
    QByteArray data = cmd.toAscii();
    outSocket->write(data);
}

void Il2Bridge::receiveUpdate()
{
    // Update connection timer and status
    il2Timer->setInterval(il2Timeout);
    il2Timer->stop();
    il2Timer->start();
    if ( !il2ConnectionStatus )
    {
        il2ConnectionStatus = true;
        emit il2Connected();
    }
    // Process data
    while ( outSocket->bytesAvailable() > 0 )
    {
        // Receive datagram
        QByteArray datagram;
        datagram.resize(outSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        outSocket->readDatagram(datagram.data(), datagram.size(),
                               &sender, &senderPort);
        QString datastr(datagram);
        // Process incomming data
        processUpdate(datastr);
     }
}

void Il2Bridge::setupObjects()
{
    setupInputObject(actDesired, 75);
    setupOutputObject(altActual, 250);
    setupOutputObject(attActual, 75);
    setupOutputObject(posActual, 250);
}

void Il2Bridge::setupInputObject(UAVObject* obj, int updatePeriod)
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

void Il2Bridge::setupOutputObject(UAVObject* obj, int updatePeriod)
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

void Il2Bridge::onAutopilotConnect()
{
    autopilotConnectionStatus = true;
    setupObjects();
    emit autopilotConnected();
}

void Il2Bridge::onAutopilotDisconnect()
{
    autopilotConnectionStatus = false;
    emit autopilotDisconnected();
}

void Il2Bridge::onIl2ConnectionTimeout()
{
    if ( il2ConnectionStatus )
    {
        il2ConnectionStatus = false;
        emit il2Disconnected();
    }
}

/**
 * calculate air density from altitude
 */
float Il2Bridge::DENSITY(float alt) {
	return (GROUNDDENSITY * pow(
				((TEMP_GROUND+(TEMP_LAPSE_RATE*alt))/TEMP_GROUND),
				((AIR_CONST_FACTOR/TEMP_LAPSE_RATE)-1) )
				);
}

/**
 * calculate air pressure from altitude
 */
float Il2Bridge::PRESSURE(float alt) {
	return DENSITY(alt)*(TEMP_GROUND+(alt*TEMP_LAPSE_RATE))*AIR_CONST;
	
}

/**
 * calculate TAS from IAS and altitude
 */
float Il2Bridge::TAS(float IAS, float alt) {
	return (IAS*sqrt(GROUNDDENSITY/DENSITY(alt)));
}

/**
 * process data string from flight simulator
 */
void Il2Bridge::processUpdate(QString& data)
{
    // save old flight data to calculate delta's later
    old=current;

    // Split
    QStringList fields = data.split("/");

    // split up response string
    int t;
    for (t=0; t<fields.length(); t++) {
    	QStringList values = fields[t].split("\\");
	// parse values
	if (values.length()>=2) {
	    int id = values[0].toInt();
	    float value = values[1].toFloat();
	    switch (id) {
	        case 30:
		    current.ias=value * KMH2MPS;
		    break;
	        case 32:
		    current.dZ=value;
		    break;
	        case 40:
		    current.Z=value;
		    break;
	        case 42:
		    current.azimuth=value;
		    break;
	        case 46:
		    current.roll=-value;
		    break;
	        case 48:
		    current.pitch=value;
		    break;
	    }
	}
    }

    // measure time
    current.dT = ((float)time->restart()) / 1000.0;
    current.T = old.T+current.dT;

    // calculate TAS from alt and IAS
    current.tas = TAS(current.ias,current.Z);

    // assume the plane actually flies straight and no wind
    // groundspeed is horizontal vector of TAS
    current.groundspeed = current.tas*cos(current.pitch*DEG2RAD);
    // x and y vector components
    current.dX = current.groundspeed*sin(current.azimuth*DEG2RAD);
    current.dY = current.groundspeed*cos(current.azimuth*DEG2RAD);

    // simple IMS - integration over time the easy way...
    current.X = old.X + (current.dX*current.dT);
    current.Y = old.Y + (current.dY*current.dT);

    // Update AltitudeActual object
    AltitudeActual::DataFields altActualData;
    altActualData.Altitude = current.Z;
    altActualData.Temperature = TEMP_GROUND + (current.Z * TEMP_LAPSE_RATE) - 273.0;
    altActualData.Pressure = PRESSURE(current.Z)/1000.0; // kpa
    altActual->setData(altActualData);

    // Update attActual object
    AttitudeActual::DataFields attActualData;
    attActualData.Roll = current.roll;
    attActualData.Pitch = current.pitch;
    attActualData.Yaw = current.azimuth;
    attActualData.q1 = 0;
    attActualData.q2 = 0;
    attActualData.q3 = 0;
    attActualData.q4 = 0;
    attActual->setData(attActualData);

    // Update gps objects
    PositionActual::DataFields gpsData;
    gpsData.Altitude = current.Z;
    gpsData.Heading = current.azimuth;
    gpsData.Groundspeed = current.groundspeed;
    gpsData.Latitude = latitude + current.Y * DEG2M;
    gpsData.Longitude = longitude + current.X * cos(gpsData.Latitude*DEG2RAD) * DEG2M;
    gpsData.Satellites = 7;
    gpsData.Status = PositionActual::STATUS_FIX3D;
    posActual->setData(gpsData);
}

void Il2Bridge::telStatsUpdated(UAVObject* obj)
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
