/**
 ******************************************************************************
 *
 * @file       xplanesimulator.cpp
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

 /**
 * Description of X-Plane Protocol:
 *
 * To see what data can be sended/recieved to/from X-Plane, launch X-Plane -> goto main menu
 * (cursor at top of main X-Plane window) -> Settings -> Data Input and Output -> Data Set.
 * Data Set shown all X-Plane params,
 * each row has four checkbox: 1st check - out to UDP; 4 check - show on screen
 * All the UDP messages for X-Plane have the same format, which is:
 * 5-character MESSAGE PROLOUGE (to indicate the type of message)
 * and then a DATA INPUT STRUCTURE (containing the message data that you want to send or receive)
 *
 * DATA INPUT/OUTPUT STRUCTURE is the following stuct:
 *
 *  struct data_struct
 *  {
 *      int index;     // data index, the index into the list of variables
					   // you can output from the Data Output screen in X-Plane.
 *      float data[8]; // the up to 8 numbers you see in the data output screen associated with that selection..
					   // many outputs do not use all 8, though.
 * };
 *
 * For Example, update of aileron/elevon/rudder in X-Plane (11 row in Data Set)
 * bytes     value     description
 * [0-3]     DATA      message type
 * [4]       none      no matter
 * [5-8]     11        code of setting param(row in Data Set)
 * [9-41]    data      message data (8 float values)
 * total size: 41 byte
 *
 */

#include "xplanesimulator.h"
#include "extensionsystem/pluginmanager.h"
#include <coreplugin/icore.h>
#include <coreplugin/threadmanager.h>
#include <math.h>
#include <qxtlogger.h>

void TraceBuf(const char* buf,int len);

XplaneSimulator::XplaneSimulator(const SimulatorSettings& params) :
		Simulator(params)
{
}


XplaneSimulator::~XplaneSimulator()
{
}

void XplaneSimulator::setupUdpPorts(const QString& host, int inPort, int outPort)
{
	inSocket->bind(QHostAddress(host), inPort);
	outSocket->bind(QHostAddress(host), outPort);
}

/**
 * update data in X-Plane simulator
 */
void XplaneSimulator::transmitUpdate()
{
	//Read ActuatorDesired from autopilot
	ActuatorDesired::DataFields actData = actDesired->getData();
	float ailerons = actData.Roll;
	float elevator = actData.Pitch;
	float rudder = actData.Yaw;
	float throttle = actData.Throttle*2-1.0;
	float tmp = -999;
	quint32 none = *((quint32*)&tmp); // get float as 4 bytes

	quint32 code;
	QByteArray buf;
	QDataStream stream(&buf,QIODevice::ReadWrite);

// !!! LAN byte order - Big Endian
//#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
//	stream.setByteOrder(QDataStream::LittleEndian);
//#endif

	// 11th data settings (flight con: ail/elv/rud)
	buf.clear();
	code = 11;
	quint8 header[] = "DATA";
	stream << *((quint32*)header) << (quint8)0x30 << code << *((quint32*)&elevator) << *((quint32*)&ailerons) << *((quint32*)&rudder)
			<< none << *((quint32*)&ailerons) << none << none  << none;
	TraceBuf(buf.data(),41);
	outSocket->write(buf);

	// 25th data settings (throttle command)
	buf.clear();
	code = 25;
	stream << *((quint32*)header) << (quint8)0x30 << code << *((quint32*)&throttle) << none  << none
			<< none  << none  << none << none  << none;
	outSocket->write(buf);



	/** !!! this settings was given from ardupilot X-Plane.pl, I comment them
	   but if it needed comment should be removed !!!

	// 8th data settings (joystick 1 ail/elv/rud)
	stream << "DATA0" << quint32(11) << elevator << ailerons << rudder
			<< float(-999) << float(-999) << float(-999) << float(-999) << float(-999);
	outSocket->write(buf);
	*/

}

/**
 * process data string from X-Plane simulator
 */
void XplaneSimulator::processUpdate(const QByteArray& dataBuf)
{
	float altitude = 0;
	float latitude = 0;
	float longitude = 0;
	float airspeed = 0;
	float speed = 0;
	float pitch = 0;
	float roll = 0;
	float heading = 0;
	float pressure = 0;
	float weather = 0;

	QString str;
	QByteArray& buf = const_cast<QByteArray&>(dataBuf);
	QString data(buf);

	if(data.left(4) == "DATA") // check type of packet
	{
		buf.remove(0,5);
		if(dataBuf.size() % 36)
		{
			qxtLog->info("incorrect length of UDP packet: ",buf);
			return; // incorrect length of struct
		}
		// check correctness of data length, length must be multiple of (id_size+8*float_size)=4+8*4=36
		int channelCounter = dataBuf.size() / 36;
		do
		{
			switch(buf[0]) // switch by id
			{
			case XplaneSimulator::LatitudeLongitude:
				latitude = *((float*)(buf.data()+4*1));
				longitude = *((float*)(buf.data()+4*2));
				altitude = *((float*)(buf.data()+4*3)) /* * 3.048 */;
				break;

			case XplaneSimulator::Speed:
				airspeed = *((float*)(buf.data()+4*6));
				speed = *((float*)(buf.data()+4*7));
				break;

			case XplaneSimulator::PitchRollHeading:
				pitch = *((float*)(buf.data()+4*1));
				roll = *((float*)(buf.data()+4*2));
				heading = *((float*)(buf.data()+4*3));
				break;

			case XplaneSimulator::SystemPressures:
				pressure = *((float*)(buf.data()+4*1));
				break;

			case XplaneSimulator::AtmosphereWeather:
				weather = *((float*)(buf.data()+4*1));
				break;

			default:
				break;
			}
			channelCounter--;
			buf.remove(0,36);
		} while (channelCounter);

		// Update AltitudeActual object
		BaroAltitude::DataFields altActualData;
		memset(&altActualData, 0, sizeof(BaroAltitude::DataFields));
		altActualData.Altitude = altitude;
		altActualData.Temperature = weather;
		altActualData.Pressure = pressure;
		altActual->setData(altActualData);

		// Update attActual object
		AttitudeActual::DataFields attActualData;
		memset(&attActualData, 0, sizeof(AttitudeActual::DataFields));
		attActualData.Roll = roll;   //roll;
		attActualData.Pitch = pitch;  // pitch
		//		attActualData.Yaw = yaw;
		//		attActualData.q1 = 0;
		//		attActualData.q2 = 0;
		//		attActualData.q3 = 0;
		//		attActualData.q4 = 0;
		attActual->setData(attActualData);

		// Update gps objects
                GPSPosition::DataFields gpsData;
                memset(&gpsData, 0, sizeof(GPSPosition::DataFields));
		gpsData.Altitude = altitude;
//		gpsData.Heading = pitch[2];
//		gpsData.Groundspeed = speed[0];
		gpsData.Latitude = latitude;
		gpsData.Longitude = longitude;
                gpsData->setData(gpsData);
	}
	// issue manual update
	//attActual->updated();
	//altActual->updated();
	//posActual->updated();
}


void TraceBuf(const char* buf,int len)
{
	QString str;
	bool reminder=true;
	for(int i=0; i < len; i++)
	{
		if(!(i%16))
		{
			if(i>0)
			{
				qDebug() << str;
				str.clear();
				reminder=false;
			}
			reminder=true;
		}
		str+=QString(" 0x%1").arg((quint8)buf[i],2,16,QLatin1Char('0'));
	}

	if(reminder)
		qDebug() << str;
}
