/**
 ******************************************************************************
 *
 * @file       mavlinksimulator.cpp
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

#include "mavlinksimulator.h"
#include "extensionsystem/pluginmanager.h"
#include "mavlink/ardupilotmega/mavlink.h"
#include "mavlink/mavlink_helpers.h"
#include <coreplugin/icore.h>
#include <coreplugin/threadmanager.h>
#include <math.h>
#include <qxtlogger.h>

double toDegrees(double radians)
{
    return radians / M_PI * 180;
}

MavlinkSimulator::MavlinkSimulator(const SimulatorSettings& params) :
		Simulator(params)
{
    once = false;
}


MavlinkSimulator::~MavlinkSimulator()
{
}

void MavlinkSimulator::setupUdpPorts(const QString& host, int inPort, int outPort)
{
	inSocket->bind(QHostAddress(host), inPort);
        //outSocket->bind(QHostAddress(host), outPort);
        once = false;

}

bool MavlinkSimulator::setupProcess()
{
    emit processOutput(QString("Please start simulator manually, and make sure it is setup to output its ") +
                       "data to host " + settings.hostAddress + " UDP port " + QString::number(settings.inPort));
    return true;

}

/**
 * update data in simulator
 */
void MavlinkSimulator::transmitUpdate()
{
}

/**
 * process data string from simulator
 */
void MavlinkSimulator::processUpdate(const QByteArray& dataBuf)
{
    mavlink_message_t message;
    mavlink_status_t status;
    bool msgReceived = false;
    for(int i = 0; i < dataBuf.size(); i++)
    {
        msgReceived = mavlink_parse_char(MAVLINK_COMM_1, dataBuf.at(i), &message, &status);
        if(msgReceived) 
        {
            break;
        }
    }

    if(msgReceived) 
    {
        switch(message.msgid)
        {
            case MAVLINK_MSG_ID_ATTITUDE:
                mavlink_attitude_t attitude;
                mavlink_msg_attitude_decode(&message, &attitude);

                AttitudeActual::DataFields attActualData;
                memset(&attActualData, 0, sizeof(AttitudeActual::DataFields));
                attActualData.Roll = toDegrees(attitude.roll);   //roll;
                attActualData.Pitch = toDegrees(attitude.pitch);  // pitch
                attActualData.Yaw = toDegrees(attitude.yaw); // Yaw
                float rpy[3];
                float quat[4];
                rpy[0] = attActualData.Roll;
                rpy[1] = attActualData.Pitch;
                rpy[2] = attActualData.Yaw;
                Utils::CoordinateConversions().RPY2Quaternion(rpy,quat);
                attActualData.q1 = quat[0];
                attActualData.q2 = quat[1];
                attActualData.q3 = quat[2];
                attActualData.q4 = quat[3];
                attActual->setData(attActualData);
                break;
            case MAVLINK_MSG_ID_GPS_RAW_INT:
                mavlink_gps_raw_int_t gps;
                mavlink_msg_gps_raw_int_decode(&message, &gps);
                // Update gps objects
                GPSPosition::DataFields gpsData;
                memset(&gpsData, 0, sizeof(GPSPosition::DataFields));
                gpsData.Altitude = gps.alt / 1000.0f; // Convert mm to m
                gpsData.Latitude = gps.lat; // Note this is in degrees * 10^-7
                gpsData.Longitude = gps.lon; // Note this is in degrees * 10^-7
                // 65535 indicates 'unknown'
                if (gps.cog != 65535) {
                    // XXX: This might be wrong, since heading != course over ground
                    gpsData.Heading = gps.cog / 100.0f;
                }
                if (gps.vel != 65535) {
                    gpsData.Groundspeed = gps.vel / 100.0f;
                }
                if (gps.eph != 65535) {
                    gpsData.HDOP = gps.eph / 100.0f;
                }
                if (gps.epv != 65535) {
                    gpsData.VDOP = gps.epv / 100.0f;
                }
                // 255 indicates 'unknown'
                if (gps.satellites_visible != 255) {
                    gpsData.Satellites = gps.satellites_visible;
                }
                if (gps.fix_type == 3) {
                    gpsData.Status = GPSPosition::STATUS_FIX3D;
                } else if (gps.fix_type == 2) {
                    gpsData.Status = GPSPosition::STATUS_FIX2D;
                } else {
                    gpsData.Status = GPSPosition::STATUS_NOFIX;
                }

                gpsPos->setData(gpsData);
                break;
        }
    }
}
