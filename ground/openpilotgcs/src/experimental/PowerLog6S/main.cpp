/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @brief      Junsi Powerlog utility CLI
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
#include <QtCore/QCoreApplication>
#include <iostream>
#include <stdio.h>
#include <../../plugins/rawhid/pjrc_rawhid.h>

#define BUF_LEN 64

using namespace std;

typedef unsigned long ULONG;    // 4 Bytes
typedef short SHORT;
typedef unsigned short USHORT;  // 2 Bytes
typedef unsigned char BYTE;     // 1 Byte
typedef unsigned short WORD;    // 2 Bytes
typedef unsigned long DWORD;    // 4 Bytes


class MyParser : public QObject {

struct POWERLOG_HID_PACK
{
	BYTE Len;				
	BYTE Type;				
	DWORD Interval;			
	BYTE LogState;			
	SHORT Current;			
	USHORT Volt;			
	DWORD Cap;			
	SHORT Cell[6];			
	USHORT RPM;			
	SHORT Temp[4];			
	USHORT Period;			
	USHORT Pulse;
};

enum
{
	TYPE_DATA_ONLINE = 0x10,
	TYPE_DATA_OFFLINE = 0x11,
	TYPE_ORDER = 0x20,
};


    public:

        void start()
        {
            qDebug() << "Connect a Junsi PowerLog 6S and watch the logging output";
            pjrc_rawhid hidHandle;
            int numDevices = hidHandle.open(1, 0x0483,0x5750,0,0); //0xff9c,0x0001);
            if( numDevices == 0 )
                numDevices = hidHandle.open(1,0x0483,0,0,0);

            qDebug() << numDevices << " device(s) opened";

            //hidHandle.mytest(0);

            char buf[BUF_LEN];
            buf[0] = 2;
            buf[1] = 0;

            cout << "Interval,Current,Volt,Cap,Cell1,Cell2,Cell3,Cell4,Cell5,Cell6,RPM,Temp1,Temp2,Temp3,Temp4,Period,Pulse" << endl;

            while (int received = hidHandle.receive(0, buf, BUF_LEN, 3500) ) {
                ShowInf(buf);
            }
        }


        void ShowInf(char *pBuf)
	{
            // qDebug() << "--------------------";
            POWERLOG_HID_PACK Inf;
            int i;
            int Count;
	
		Count=0;
		Inf.Len = pBuf[Count];
		Count += sizeof(Inf.Len); 
	
		Inf.Type = pBuf[Count];
		Count += sizeof(Inf.Type);

		Inf.Interval = *((DWORD *)&pBuf[Count]);
                printf("%ld,",Inf.Interval);

		Count += sizeof(Inf.Interval);

		Inf.LogState = pBuf[Count];
		Count += sizeof(Inf.LogState);

		if(((Inf.Type == TYPE_DATA_ONLINE)||(Inf.Type == TYPE_DATA_OFFLINE)) && (Inf.Len == 0x29))//0x27
		{
			Inf.Current = *((SHORT *)&pBuf[Count]);
			Count += sizeof(Inf.Current);
                        GetShowValue(QString("Current:"),Inf.Current,5,2);

		Inf.Volt = *((USHORT *)&pBuf[Count]);
		Count += sizeof(Inf.Volt);
                GetShowValue(QString("Voltage:"),Inf.Volt,5,2);

		Inf.Cap = *((DWORD *)&pBuf[Count]);
		Count += sizeof(Inf.Cap);
                GetShowValue(QString("Cap:"),Inf.Cap,6,0);

		for(i=0;i<6;i++)
		{
			Inf.Cell[i] = *((SHORT *)&pBuf[Count]);
			Count += sizeof(Inf.Cell[i]);
		}
                GetShowValue(QString("Cell 1:"),Inf.Cell[0],5,3);
                GetShowValue(QString("Cell 2:"),Inf.Cell[1],5,3);
                GetShowValue(QString("Cell 3:"),Inf.Cell[2],5,3);
                GetShowValue(QString("Cell 4:"),Inf.Cell[3],5,3);
                GetShowValue(QString("Cell 5:"),Inf.Cell[4],5,3);
                GetShowValue(QString("Cell 6:"),Inf.Cell[5],5,3);

		Inf.RPM = *((USHORT *)&pBuf[Count]);
		Count += sizeof(Inf.RPM);
                GetShowValue(QString("RPM:"),Inf.RPM,6,0);


		for(i=0;i<4;i++)
		{
			Inf.Temp[i] = *((SHORT *)&pBuf[Count]);
			Count += sizeof(Inf.Temp[i]);
			
		}
                GetShowValue(QString("Int Temp1:"),Inf.Temp[0],4,1);
		if(Inf.Temp[1]==0x7fff)
                        QString txtExtTemp1 = QString("NULL");
		else
                        GetShowValue(QString("Ext temp1:"),Inf.Temp[1],4,1);
		if(Inf.Temp[2]==0x7fff)
                        QString txtExtTemp2 = QString("NULL");
		else
                        GetShowValue(QString("Ext temp2:"),Inf.Temp[2],4,1);
		if(Inf.Temp[3]==0x7fff)
                        QString txtExtTemp3 = QString("NULL");
		else
                        GetShowValue(QString("Ext temp3:"),Inf.Temp[3],4,1);

		Inf.Period = *((USHORT *)&pBuf[Count]);
		Count += sizeof(Inf.Period);
                GetShowValue(QString("Period:"),Inf.Period,6,0);

		Inf.Pulse = *((USHORT *)&pBuf[Count]);
		Count += sizeof(Inf.Pulse);
                GetShowValue(QString("Pulse:"),Inf.Pulse,6,0);

                printf("\n");
	}

}

void GetShowValue(QString label,DWORD Value,WORD Len,WORD Dot)
{
    //cout << label .toStdString() << " ";

    if(Value<0)
	{
		Value = -Value;
		if(Dot==1)
                        printf("-%ld.%01lu",Value/10,Value%10);
		else if(Dot==2)
                        printf("-%ld.%02lu",Value/100,Value%100);
		else if(Dot==3)
                        printf("-%ld.%03lu",Value/1000,Value%1000);
		else if(Dot==4)
                        printf("-%ld.%04lu",Value/10000,Value%10000);
		else
                        printf("-%ld",Value);
	}
	else
	{
		if(Dot==1)
                        printf("%ld.%01lu",Value/10,Value%10);
		else if(Dot==2)
                        printf("%ld.%02lu",Value/100,Value%100);
		else if(Dot==3)
                        printf("%ld.%03lu",Value/1000,Value%1000);
		else if(Dot==4)
                        printf("%ld.%04lu",Value/10000,Value%10000);
		else
                        printf("%ld",Value);
	}
    printf(",");

}



    };


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyParser b;
    b.start();

//    return a.exec();
}
