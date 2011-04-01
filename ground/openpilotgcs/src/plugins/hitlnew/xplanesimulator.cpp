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
    once = false;
}


XplaneSimulator::~XplaneSimulator()
{
}

void XplaneSimulator::setupUdpPorts(const QString& host, int inPort, int outPort)
{
	inSocket->bind(QHostAddress(host), inPort);
        //outSocket->bind(QHostAddress(host), outPort);
        once = false;

}

bool XplaneSimulator::setupProcess()
{
    emit processOutput(QString("Please start X-Plane manually, and make sure it is setup to output its ") +
                       "data to host " + settings.hostAddress + " UDP port " + QString::number(settings.inPort));
    return true;

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
        float none = -999;
        //quint32 none = *((quint32*)&tmp); // get float as 4 bytes

        quint32 code;
	QByteArray buf;
	QDataStream stream(&buf,QIODevice::ReadWrite);

        // !!! LAN byte order - Big Endian
        #if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
            stream.setByteOrder(QDataStream::LittleEndian);
        #endif

	// 11th data settings (flight con: ail/elv/rud)
	buf.clear();
        code = 11;
        //quint8 header[] = "DATA";
        /*
        stream << *((quint32*)header) <<
                  (quint8)0x30 <<
                  code <<
                  *((quint32*)&elevator) <<
                  *((quint32*)&ailerons) <<
                  *((quint32*)&rudder)   <<
                  none <<
                  *((quint32*)&ailerons) <<
                  none <<
                  none  <<
                  none;
                  */
        buf.append("DATA0");
        buf.append(reinterpret_cast<const char*>(&code), sizeof(code));
        buf.append(reinterpret_cast<const char*>(&elevator), sizeof(elevator));
        buf.append(reinterpret_cast<const char*>(&ailerons), sizeof(ailerons));
        buf.append(reinterpret_cast<const char*>(&rudder), sizeof(rudder));
        buf.append(reinterpret_cast<const char*>(&none), sizeof(none));
        buf.append(reinterpret_cast<const char*>(&rudder), sizeof(rudder));
        buf.append(reinterpret_cast<const char*>(&none), sizeof(none));
        buf.append(reinterpret_cast<const char*>(&none), sizeof(none));
        buf.append(reinterpret_cast<const char*>(&none), sizeof(none));
        TraceBuf(buf.data(),41);

        if(outSocket->writeDatagram(buf, QHostAddress(settings.remoteHostAddress), settings.outPort) == -1)
        {
            emit processOutput("Error sending UDP packet to XPlane: " + outSocket->errorString() + "\n");
        }
        //outSocket->write(buf);

	// 25th data settings (throttle command)
	buf.clear();
	code = 25;
        //stream << *((quint32*)header) << (quint8)0x30 << code << *((quint32*)&throttle) << none  << none
        //		<< none  << none  << none << none  << none;
        buf.append("DATA0");
        buf.append(reinterpret_cast<const char*>(&code), sizeof(code));
        buf.append(reinterpret_cast<const char*>(&throttle), sizeof(throttle));
        buf.append(reinterpret_cast<const char*>(&throttle), sizeof(throttle));
        buf.append(reinterpret_cast<const char*>(&none), sizeof(none));
        buf.append(reinterpret_cast<const char*>(&none), sizeof(none));
        buf.append(reinterpret_cast<const char*>(&none), sizeof(none));
        buf.append(reinterpret_cast<const char*>(&none), sizeof(none));
        buf.append(reinterpret_cast<const char*>(&none), sizeof(none));
        buf.append(reinterpret_cast<const char*>(&none), sizeof(none));

        if(outSocket->writeDatagram(buf, QHostAddress(settings.remoteHostAddress), settings.outPort) == -1)
        {
            emit processOutput("Error sending UDP packet to XPlane: " + outSocket->errorString() + "\n");
        }

        //outSocket->write(buf);



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
        float temperature = 0;
        float velX = 0;
        float velY = 0;
        float velZ = 0;
        float dstX = 0;
        float dstY = 0;
        float dstZ = 0;
        float accX = 0;
        float accY = 0;
        float accZ = 0;
        float rollRate=0;
        float pitchRate=0;
        float yawRate=0;

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
                                altitude = *((float*)(buf.data()+4*3))* FT2M;
				break;

			case XplaneSimulator::Speed:
                                airspeed = *((float*)(buf.data()+4*7));
                                speed = *((float*)(buf.data()+4*8));
				break;

			case XplaneSimulator::PitchRollHeading:
				pitch = *((float*)(buf.data()+4*1));
				roll = *((float*)(buf.data()+4*2));
				heading = *((float*)(buf.data()+4*3));
				break;

                        /*
                       case XplaneSimulator::SystemPressures:
				pressure = *((float*)(buf.data()+4*1));
				break;
                                */

			case XplaneSimulator::AtmosphereWeather:
                                pressure = *((float*)(buf.data()+4*1)) * INHG2KPA;
                                temperature = *((float*)(buf.data()+4*2));
				break;

                        case XplaneSimulator::LocVelDistTraveled:
                            dstX = *((float*)(buf.data()+4*1));
                            dstY = - *((float*)(buf.data()+4*3));
                            dstZ = *((float*)(buf.data()+4*2));
                            velX = *((float*)(buf.data()+4*4));
                            velY = - *((float*)(buf.data()+4*6));
                            velZ = *((float*)(buf.data()+4*5));
                            break;

                        case XplaneSimulator::AngularVelocities:
                            pitchRate = *((float*)(buf.data()+4*1));
                            rollRate = *((float*)(buf.data()+4*2));
                            yawRate = *((float*)(buf.data()+4*3));
                            break;

                        case XplaneSimulator::Gload:
			    accX = *((float*)(buf.data()+4*6)) * GEE;
			    accY = *((float*)(buf.data()+4*7)) * GEE;
			    accZ = *((float*)(buf.data()+4*5)) * GEE;
			    break;

			default:
				break;
			}
			channelCounter--;
			buf.remove(0,36);
		} while (channelCounter);


                HomeLocation::DataFields homeData = posHome->getData();
                if(!once)
                {
                    // Upon startup, we reset the HomeLocation object to
                    // the plane's location:
                    memset(&homeData, 0, sizeof(HomeLocation::DataFields));
                    // Update homelocation
                    homeData.Latitude = latitude * 1e7;
                    homeData.Longitude = longitude * 1e7;
                    homeData.Altitude = altitude;
                    double LLA[3];
                    LLA[0]=latitude;
                    LLA[1]=longitude;
                    LLA[2]=altitude;
                    double ECEF[3];
                    double RNE[9];
                    Utils::CoordinateConversions().RneFromLLA(LLA,(double (*)[3])RNE);
                    for (int t=0;t<9;t++) {
                            homeData.RNE[t]=RNE[t];
                    }
                    Utils::CoordinateConversions().LLA2ECEF(LLA,ECEF);
                    homeData.ECEF[0]=ECEF[0]*100;
                    homeData.ECEF[1]=ECEF[1]*100;
                    homeData.ECEF[2]=ECEF[2]*100;
                    homeData.Be[0]=0;
                    homeData.Be[1]=0;
                    homeData.Be[2]=0;
                    posHome->setData(homeData);
                    posHome->updated();

                    // Initialize the initial distance
                    initX = dstX;
                    initY = dstY;
                    initZ = dstZ;
                    once=1;
                }


		// Update AltitudeActual object
		BaroAltitude::DataFields altActualData;
		memset(&altActualData, 0, sizeof(BaroAltitude::DataFields));
		altActualData.Altitude = altitude;
                altActualData.Temperature = temperature;
		altActualData.Pressure = pressure;
		altActual->setData(altActualData);

		// Update attActual object
		AttitudeActual::DataFields attActualData;
		memset(&attActualData, 0, sizeof(AttitudeActual::DataFields));
		attActualData.Roll = roll;   //roll;
		attActualData.Pitch = pitch;  // pitch
                attActualData.Yaw = heading; // Yaw
		float rpy[3];
		float quat[4];
		rpy[0] = roll;
		rpy[1] = pitch;
		rpy[2] = heading;
		Utils::CoordinateConversions().RPY2Quaternion(rpy,quat);
		attActualData.q1 = quat[0];
		attActualData.q2 = quat[1];
		attActualData.q3 = quat[2];
		attActualData.q4 = quat[3];
		attActual->setData(attActualData);

		// Update gps objects
                GPSPosition::DataFields gpsData;
                memset(&gpsData, 0, sizeof(GPSPosition::DataFields));
                gpsData.Altitude = altitude;
                gpsData.Heading = heading;
                gpsData.Groundspeed = speed;
                gpsData.Latitude = latitude*1e7;
                gpsData.Longitude = longitude*1e7;
                gpsData.Satellites = 10;
                gpsData.Status = GPSPosition::STATUS_FIX3D;
                gpsPos->setData(gpsData);

                // Update VelocityActual.{Nort,East,Down}
                VelocityActual::DataFields velocityActualData;
                memset(&velocityActualData, 0, sizeof(VelocityActual::DataFields));
                velocityActualData.North = velY*100;
                velocityActualData.East = velX*100;
                velocityActualData.Down = -velZ*100;
                velActual->setData(velocityActualData);

                // Update PositionActual.{Nort,East,Down}
                PositionActual::DataFields positionActualData;
                memset(&positionActualData, 0, sizeof(PositionActual::DataFields));
                positionActualData.North = (dstY-initY)*100;
                positionActualData.East = (dstX-initX)*100;
                positionActualData.Down = -(dstZ-initZ)*100;
                posActual->setData(positionActualData);

                // Update AttitudeRaw object (filtered gyros only for now)
                AttitudeRaw::DataFields rawData;
                memset(&rawData, 0, sizeof(AttitudeRaw::DataFields));
                rawData = attRaw->getData();
                rawData.gyros[0] = rollRate;
                //rawData.gyros_filtered[1] = cos(DEG2RAD * roll) * pitchRate + sin(DEG2RAD * roll) * yawRate;
                //rawData.gyros_filtered[2] = cos(DEG2RAD * roll) * yawRate - sin(DEG2RAD * roll) * pitchRate;
                rawData.gyros[1] = pitchRate;
                rawData.gyros[2] = yawRate;
                rawData.accels[0] = accX;
                rawData.accels[1] = accY;
                rawData.accels[2] = -accZ;
                attRaw->setData(rawData);


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
