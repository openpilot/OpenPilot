/**
 ******************************************************************************
 *
 * @file       il2simulator.cpp
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
 * 30: CAS in km/h (float)
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


#include "il2simulator.h"
#include "extensionsystem/pluginmanager.h"
#include <coreplugin/icore.h>
#include <coreplugin/threadmanager.h>
#include <math.h>
#include <qxtlogger.h>

const float IL2Simulator::FT2M = 12*.254;
const float IL2Simulator::KT2MPS = 0.514444444;
const float IL2Simulator::MPS2KMH = 3.6;
const float IL2Simulator::KMH2MPS = (1.0/3.6);
const float IL2Simulator::INHG2KPA = 3.386;
const float IL2Simulator::RAD2DEG = (180.0/M_PI);
const float IL2Simulator::DEG2RAD = (M_PI/180.0);
const float IL2Simulator::NM2DEG =  60.*1852.; // 60 miles per degree times 1852 meters per mile
const float IL2Simulator::DEG2NM = (1.0/(60.*1852.));

IL2Simulator::IL2Simulator(const SimulatorSettings& params) :
    Simulator(params)
{
    airParameters=getAirParameters();
}


IL2Simulator::~IL2Simulator()
{
}

void IL2Simulator::setupUdpPorts(const QString& host, int inPort, int outPort)
{
    Q_UNUSED(outPort);

    inSocket->connectToHost(host,inPort); // IL2
    if(!inSocket->waitForConnected())
        qxtLog->error(Name() + " cann't connect to UDP Port: " + QString::number(inPort));
}

void IL2Simulator::transmitUpdate()
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
    //outSocket->write(data);
    inSocket->write(data);  // for IL2 must send to the same port as input!!!!!!!!!!!!!
}


/**
 * process data string from flight simulator
 */
void IL2Simulator::processUpdate(const QByteArray& inp)
{
    // save old flight data to calculate delta's later
    old=current;
    QString data(inp);
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
                current.cas=value * KMH2MPS;
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
    if (current.dT<0.001)  current.dT=0.001;
    current.T = old.T+current.dT;
    current.i = old.i+1;
    if (current.i==1) {
        old.dRoll=0;
        old.dPitch=0;
        old.dAzimuth=0;
        old.ddX=0;
        old.ddX=0;
        old.ddX=0;
    }

    // calculate TAS from alt and CAS
    float gravity =9.805;
    current.tas = cas2tas(current.cas, current.Z, airParameters, gravity);

    // assume the plane actually flies straight and no wind
    // groundspeed is horizontal vector of TAS
    current.groundspeed = current.tas*cos(current.pitch*DEG2RAD);
    // x and y vector components
    current.dX = current.groundspeed*sin(current.azimuth*DEG2RAD);
    current.dY = current.groundspeed*cos(current.azimuth*DEG2RAD);

    // simple IMS - integration over time the easy way...
    current.X = old.X + (current.dX*current.dT);
    current.Y = old.Y + (current.dY*current.dT);

    // accelerations (filtered)
    if (isnan(old.ddX) || isinf(old.ddX)) old.ddX=0;
    if (isnan(old.ddY) || isinf(old.ddY)) old.ddY=0;
    if (isnan(old.ddZ) || isinf(old.ddZ)) old.ddZ=0;
#define SPEED_FILTER 10
    current.ddX = ((current.dX-old.dX)/current.dT + SPEED_FILTER * (old.ddX)) / (SPEED_FILTER+1);
    current.ddY = ((current.dY-old.dY)/current.dT + SPEED_FILTER * (old.ddY)) / (SPEED_FILTER+1);
    current.ddZ = ((current.dZ-old.dZ)/current.dT + SPEED_FILTER * (old.ddZ)) / (SPEED_FILTER+1);

#define TURN_FILTER 10
    // turn speeds (filtered)
    if (isnan(old.dAzimuth) || isinf(old.dAzimuth)) old.dAzimuth=0;
    if (isnan(old.dPitch) || isinf(old.dPitch)) old.dPitch=0;
    if (isnan(old.dRoll) || isinf(old.dRoll)) old.dRoll=0;
    current.dAzimuth = (angleDifference(current.azimuth,old.azimuth)/current.dT + TURN_FILTER * (old.dAzimuth)) / (TURN_FILTER+1);
    current.dPitch   = (angleDifference(current.pitch,old.pitch)/current.dT     + TURN_FILTER * (old.dPitch))   / (TURN_FILTER+1);
    current.dRoll    = (angleDifference(current.roll,old.roll)/current.dT       + TURN_FILTER * (old.dRoll))    / (TURN_FILTER+1);

    ///////
    // Output formatting
    ///////
    Output2Hardware out;
    memset(&out, 0, sizeof(Output2Hardware));

    // Compute rotation matrix, for later calculations
    float Rbe[3][3];
    float rpy[3];
    float quat[4];
    rpy[0]=current.roll;
    rpy[1]=current.pitch;
    rpy[2]=current.azimuth;
    Utils::CoordinateConversions().RPY2Quaternion(rpy,quat);
    Utils::CoordinateConversions().Quaternion2R(quat,Rbe);

    // Update GPS Position objects
    double HomeLLA[3];
    double LLA[3];
    double NED[3];
    HomeLLA[0]=settings.latitude.toFloat();
    HomeLLA[1]=settings.longitude.toFloat();
    HomeLLA[2]=0;
    NED[0] = current.Y;
    NED[1] = current.X;
    NED[2] = -current.Z;
    Utils::CoordinateConversions().NED2LLA_HomeLLA(HomeLLA,NED,LLA);
    out.latitude = LLA[0] * 1e7;
    out.longitude = LLA[1] * 1e7;
    out.groundspeed = current.groundspeed;

    out.calibratedAirspeed = current.cas;
    out.trueAirspeed=cas2tas(current.cas, current.Z, airParameters, gravity);

    out.dstN=current.Y;
    out.dstE=current.X;
    out.dstD=-current.Z;

    // Update BaroAltitude object
    out.altitude = current.Z;
    out.agl = current.Z;
    out.temperature = airParameters.groundTemp + (current.Z * airParameters.tempLapseRate) - 273.0;
    out.pressure = airPressureFromAltitude(current.Z, airParameters, gravity) ; // kpa


    // Update attActual object
    out.roll = current.roll;   //roll;
    out.pitch = current.pitch;  // pitch
    out.heading = current.azimuth; // yaw


    // Update VelocityActual.{North,East,Down}
    out.velNorth = current.dY;
    out.velEast = current.dX;
    out.velDown = -current.dZ;

    // rotate turn rates and accelerations into body frame
    // (note: rotation deltas are NOT in NED frame but in RPY - manual conversion!)
    out.rollRate = current.dRoll;
    out.pitchRate = cos(DEG2RAD * current.roll) * current.dPitch + sin(DEG2RAD * current.roll) * current.dAzimuth;
    out.yawRate = cos(DEG2RAD * current.roll) * current.dAzimuth - sin(DEG2RAD * current.roll) * current.dPitch;

    //Update accelerometer sensor data
    out.accX = current.ddX*Rbe[0][0]
            +current.ddY*Rbe[0][1]
            +(current.ddZ+GEE)*Rbe[0][2];
    out.accY = current.ddX*Rbe[1][0]
            +current.ddY*Rbe[1][1]
            +(current.ddZ+GEE)*Rbe[1][2];
    out.accZ = - (current.ddX*Rbe[2][0]
                     +current.ddY*Rbe[2][1]
                     +(current.ddZ+GEE)*Rbe[2][2]);

    updateUAVOs(out);
}

/**
 * process data string from flight simulator
 */
float IL2Simulator::angleDifference(float a, float b)
{
    float d=a-b;
    if (d>180) d-=360;
    if (d<-180)d+=360;
    return d;
}
