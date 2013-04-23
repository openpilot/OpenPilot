/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      UAVObjectGenerator main.
 *
 * @see        The GNU Public License (GPL) Version 3
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
#include <QFile>
#include <QtCore/QDir>
#include <QtCore>
#include <QString>
#include <QStringList>
#include <iostream>
#include "../uavobjgenerator/generators/generator_io.h"

#define RETURN_ERR_USAGE 1
#define RETURN_ERR_XML 2
#define RETURN_OK 0

using namespace std;

/**
 * print usage info
 */
void usage() {
    cout << "Usage: loggerextractor [-adc] [-mpu6050] [-R1] [-R2] [-R3] [-v] log_path " << endl;
    cout << "Languages: "<< endl;
    cout << "\t-adc          export adc value to csv" << endl;
    cout << "\t-mpu6050        export mpu6050 value to csv" << endl;
    cout << "\t-R1          export usart1 to .opl" << endl;
    cout << "\t-R2        export usart2 to .opl" << endl;
    cout << "\t-R3        export usart2 to .opl" << endl;
    cout << "Misc: "<< endl;
    cout << "\t-h             this help" << endl;
    cout << "\t-v             verbose" << endl;
    cout << "\log_path     path to log (.LOG) files." << endl;
}

/**
 * inform user of invalid usage
 */
int usage_err() {
    cout << "Invalid usage!" << endl;
    usage();
    return RETURN_ERR_USAGE;
}

/**
 * entrance
 */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    cout << "- OpenPilot OpLogger UAVObject Extractor -" << endl;

    QString inputpath;
    QString templatepath;
    QString outputpath;
    QStringList arguments_stringlist;
    QStringList objects_stringlist;
	//QByteArray Out;
	//QByteArray Out2;
	int Len;
	int Len2;
	int Pos=0;
	int Pos2=0;

    // process arguments
    for (int argi=1;argi<argc;argi++)
        arguments_stringlist << argv[argi];

    if ((arguments_stringlist.removeAll("-h")>0)||(arguments_stringlist.removeAll("-h")>0)) {
      usage();
      return RETURN_OK; 
    }

    bool verbose=(arguments_stringlist.removeAll("-v")>0);
    bool do_adc=(arguments_stringlist.removeAll("-adc")>0);
    bool do_mpu6050=(arguments_stringlist.removeAll("-mpu6050")>0);
    bool do_R1=(arguments_stringlist.removeAll("-R1")>0);
    bool do_R2=(arguments_stringlist.removeAll("-R2")>0);
    bool do_R3=(arguments_stringlist.removeAll("-R3")>0);

    if (arguments_stringlist.length() >= 1) {
        inputpath = arguments_stringlist.at(0);
        templatepath = inputpath;
    } else {
        // wrong number of arguments
        return usage_err();
    }

    if (!inputpath.endsWith("/"))
        inputpath.append("/"); // append a slash if it is not there

    if (!templatepath.endsWith("/"))
        templatepath.append("/"); // append a slash if it is not there

    // put all output files in the current directory
    outputpath = QString("./");

    QDir logPath = QDir(inputpath);
//    UAVObjectParser* parser = new UAVObjectParser();

    QStringList filters=QStringList("*.LOG");

    logPath.setNameFilters(filters);
    QFileInfoList logList = logPath.entryInfoList();

    // Read in each XML file and parse object(s) in them
    
    for (int n = 0; n < logList.length(); ++n) {
        QFileInfo fileinfo = logList[n];
        /*if (!do_allObjects) {
            if (!objects_stringlist.removeAll(fileinfo.fileName().toLower())) {
                if (verbose)
                  cout << "Skipping XML file: " << fileinfo.fileName().toStdString() << endl;
               continue;
            }
        }*/
        if (verbose)
          cout << "Parsing LOG file: " << fileinfo.fileName().toStdString() << endl;
        QString filename = fileinfo.fileName();
		QFile file(filename);
		QFile file2("R1.opl");
		QFile file3("R2.opl");
		QFile file4("Adc.csv");
		QFile file5("mpu.csv");
		
		file.open(QIODevice::ReadOnly);
		file2.open(QIODevice::WriteOnly);
		file3.open(QIODevice::WriteOnly);
		file4.open(QIODevice::WriteOnly| QIODevice::Text);
		file5.open(QIODevice::WriteOnly| QIODevice::Text);
		
		int lastSysTime;
		float Adc;
		char FloatIEEE[4];
		QString qStr; 
		int AdError=0;
		int MpError=0;
		int R1Error=0;
		int R2Error=0;
		
		while (!file.atEnd()) {
         QByteArray line = file.readLine();
		 if(line[0]=='A')
		 {
			if(line[1]=='d')
			{	
				int len1=line.length();
				while(line.length()<20)
				{
					QByteArray line2 = file.readLine();
					//qRealloc(len1, len1*sizeof(QByteArray));
					line.resize(len1+line2.length());
					for(int i=0;i<line2.length();i++)
					{
						line[len1+i]= line2[i];
					}
					len1+=line2.length();
				}
				while((line[len1-2]!='\r')&&(len1<20))
				{
					QByteArray line2 = file.readLine();
					//qRealloc(len1, len1*sizeof(QByteArray));
					line.resize(len1+line2.length());
					for(int i=0;i<line2.length();i++)
					{
						line[len1+i]= line2[i];
					}
					len1+=line2.length();
				}
				if(len1==20)
				{
					FloatIEEE[0]=line[2];
					FloatIEEE[1]=line[3];
					FloatIEEE[2]=line[4];
					FloatIEEE[3]=line[5];
					memcpy(&lastSysTime,&FloatIEEE, 4);
					qStr = QString::number(lastSysTime);
					file4.write( qStr.toAscii()+QString(";").toAscii());
					FloatIEEE[0]=line[6];
					FloatIEEE[1]=line[7];
					FloatIEEE[2]=line[8];
					FloatIEEE[3]=line[9];
					memcpy(&Adc,&FloatIEEE, 4);
					qStr = QString::number(Adc);
					file4.write( qStr.toAscii()+QString(";").toAscii());
					FloatIEEE[0]=line[10];
					FloatIEEE[1]=line[11];
					FloatIEEE[2]=line[12];
					FloatIEEE[3]=line[13];
					memcpy(&Adc,&FloatIEEE, 4);
					qStr = QString::number(Adc);
					file4.write( qStr.toAscii()+QString(";").toAscii());
					FloatIEEE[0]=line[14];
					FloatIEEE[1]=line[15];
					FloatIEEE[2]=line[16];
					FloatIEEE[3]=line[17];
					memcpy(&Adc,&FloatIEEE, 4);
					qStr = QString::number(Adc);
					file4.write( qStr.toAscii()+QString("\r").toAscii());
					 cout << "Ad : " <<line.length()<<"\r\n" ;
				}
				else
				{
					cout << "OverlenghtAd " <<line.length() <<"\r\n";
					AdError++;
				}
			}
		 }
		 else if(line[0]=='M')
		 {
			if(line[1]=='p')
			{
				int len1=line.length();
				while(line.length()<36)
				{
					QByteArray line2 = file.readLine();
					//qRealloc(len1, len1*sizeof(QByteArray));
					line.resize(len1+line2.length());
					for(int i=0;i<line2.length();i++)
					{
						line[len1+i]= line2[i];
					}
					len1+=line2.length();
				}
				while((line[len1-2]!='\r')&&(len1<36))
				{
					QByteArray line2 = file.readLine();
					//qRealloc(len1, len1*sizeof(QByteArray));
					line.resize(len1+line2.length());
					for(int i=0;i<line2.length();i++)
					{
						line[len1+i]= line2[i];
					}
					len1+=line2.length();
				}
				if(len1==36)
				{
					FloatIEEE[0]=line[2];
					FloatIEEE[1]=line[3];
					FloatIEEE[2]=line[4];
					FloatIEEE[3]=line[5];
					memcpy(&lastSysTime,&FloatIEEE, 4);
					qStr = QString::number(lastSysTime);
					file5.write( qStr.toAscii()+QString(";").toAscii());
					FloatIEEE[0]=line[6];
					FloatIEEE[1]=line[7];
					FloatIEEE[2]=line[8];
					FloatIEEE[3]=line[9];
					memcpy(&Adc,&FloatIEEE, 4);
					qStr = QString::number(Adc);
					file5.write( qStr.toAscii()+QString(";").toAscii());
					FloatIEEE[0]=line[10];
					FloatIEEE[1]=line[11];
					FloatIEEE[2]=line[12];
					FloatIEEE[3]=line[13];
					memcpy(&Adc,&FloatIEEE, 4);
					qStr = QString::number(Adc);
					file5.write( qStr.toAscii()+QString(";").toAscii());
					FloatIEEE[0]=line[14];
					FloatIEEE[1]=line[15];
					FloatIEEE[2]=line[16];
					FloatIEEE[3]=line[17];
					memcpy(&Adc,&FloatIEEE, 4);
					qStr = QString::number(Adc);
					file5.write( qStr.toAscii()+QString(";").toAscii());
					FloatIEEE[0]=line[18];
					FloatIEEE[1]=line[19];
					FloatIEEE[2]=line[20];
					FloatIEEE[3]=line[21];
					memcpy(&Adc,&FloatIEEE, 4);
					qStr = QString::number(Adc);
					file5.write( qStr.toAscii()+QString(";").toAscii());
					FloatIEEE[0]=line[22];
					FloatIEEE[1]=line[23];
					FloatIEEE[2]=line[24];
					FloatIEEE[3]=line[25];
					memcpy(&Adc,&FloatIEEE, 4);
					qStr = QString::number(Adc);
					file5.write( qStr.toAscii()+QString(";").toAscii());
					FloatIEEE[0]=line[26];
					FloatIEEE[1]=line[27];
					FloatIEEE[2]=line[28];
					FloatIEEE[3]=line[29];
					memcpy(&Adc,&FloatIEEE, 4);
					qStr = QString::number(Adc);
					file5.write( qStr.toAscii()+QString(";").toAscii());
					FloatIEEE[0]=line[30];
					FloatIEEE[1]=line[31];
					FloatIEEE[2]=line[32];
					FloatIEEE[3]=line[33];
					memcpy(&Adc,&FloatIEEE, 4);
					qStr = QString::number(Adc);
					file5.write( qStr.toAscii()+QString("\r").toAscii());
					 cout << "Mp : " <<line.length()<< "\r\n";
				}
				else
				{
					//file5.write("Error "+line+"\r\n");
					cout << "OverlenghtMp " <<line.length()<< "\r\n";
					//cout << line.Data();
					MpError++;
				}
			}
		 }
		 else if(line[0]=='R')
		 {
			if(line[1]=='1')
			{
				QByteArray Out;
				Pos=0;
				int len1=line.length();
				while(len1<10)
				{
					QByteArray line2 = file.readLine();
					//qRealloc(len1, len1*sizeof(QByteArray));
					line.resize(len1+line2.length());
					for(int i=0;i<line2.length();i++)
					{
						line[len1+i]= line2[i];
					}
					len1+=line2.length();
					//line += file.readLine();
				}
				Len2=(line[len1-3]+9);
				cout << "R1 : ("<< Len2 <<")" <<"["<<len1<<"] \r\n";
				while((line[len1-2]!='\r')&&(Len2!=len1))
				{
					QByteArray line4 = file.readLine();
					line.resize(len1+line4.length());
					//len1=line.length();
					for(int i=0;i<line4.length();i++)
					{
						line[len1+i]= line4[i];
					}
					//line += file.readLine();
					len1+=line4.length();
					Len2=(line[len1-3]+9);
					//cout << Len2;
				}
				//if(line.length()<20)
				{
					for(int i=0;i<line[line.length()-3];i++)
					{
						//Out[Pos]=line[6+i];
						//Pos++;
						Out[Pos]=line[6+i];
						Pos++;
					}
					//Pos+=line[line.length()-3];
					//Pos2+=line[line.length()-3];
					//cout << "R1 : " <<line.length() ;
					file2.write(Out);
				}
				/*else
				{
					cout << "R1 Overlenght "<<line.length()<< "\r\n";
					//QByteArray line2 = file.readLine();
				}*/
			}
			else if(line[1]=='2')
			{
				QByteArray Out2;
				Pos2=0;
				int len1=line.length();
				while(len1<10)
				{
					QByteArray line2 = file.readLine();
					//qRealloc(len1, len1*sizeof(QByteArray));
					line.resize(len1+line2.length());
					for(int i=0;i<line2.length();i++)
					{
						line[len1+i]= line2[i];
					}
					len1+=line2.length();
					//line += file.readLine();
				}
				Len2=(line[len1-3]+9);
				cout << "R2 : ("<< Len2 <<")" <<"["<<len1<<"] \r\n";
				while((line[len1-2]!='\r')&&(Len2!=len1))
				{
					QByteArray line4 = file.readLine();
					line.resize(len1+line4.length());
					//len1=line.length();
					for(int i=0;i<line4.length();i++)
					{
						line[len1+i]= line4[i];
					}
					//line += file.readLine();
					len1+=line4.length();
					Len2=(line[len1-3]+9);
					//cout << Len2;
				}
				//if(line.length()<30)
				{
					for(int i=0;i<line[line.length()-3];i++)
					{
						//Out[Pos]=line[6+i];
						//Pos++;
						Out2[Pos2]=line[6+i];
						Pos2++;
					}
					//Pos+=line[line.length()-3];
					//Pos2+=line[line.length()-3];
					//cout << "R2 : " <<line.length() ;
					file3.write(Out2);
				}
				/*else
				{
					cout << "R2 Overlenght "<<line.length()<< "\r\n";
					file3.close();
					exit(0);
					//QByteArray line2 = file.readLine();
				}*/
			}
			else if(line[1]=='3')
			{
				cout << "R3 : " <<line.length() ;
			}
		 }
		 else
		 {
			cout << "Error" <<line.length() << "\r\n";
			R1Error++;
		 }
     }
	 
	 cout << "Error R:" <<R1Error << " Ad:"<<AdError<<" Mp:"<<MpError<<"\r\n";
	 
	 file2.close();
	 file3.close();
	 file4.close();
	 file5.close();
/*
		QTextStream fileStr(&file);
		QString str = fileStr.readAll();
		file.close();
		cout << "File is: " <<str.length() << " bytes long";*/
		/*
        QString logstr = readFile(fileinfo.absoluteFilePath());
			cout << "File is: " <<logstr.length() << " bytes long";
			*/

		/*

        QString res = parser->parseXML(xmlstr, filename);

        if (!res.isNull()) {
	    if (!verbose) {
               cout << "Error in XML file: " << fileinfo.fileName().toStdString() << endl;
            }
            cout << "Error parsing " << res.toStdString() << endl;
            return RETURN_ERR_XML;
        }*/
    }
/*
    if (objects_stringlist.length() > 0) {
        cout << "required UAVObject definitions not found! " << objects_stringlist.join(",").toStdString() << endl;
        return RETURN_ERR_XML;
    }

    // check for duplicate object ID's
    QList<quint32> objIDList;
    int numBytesTotal=0;
    for (int objidx = 0; objidx < parser->getNumObjects(); ++objidx) {
        quint32 id = parser->getObjectID(objidx);
        numBytesTotal+=parser->getNumBytes(objidx);
        if (verbose)
          cout << "Checking object " << parser->getObjectName(objidx).toStdString() << " (" << parser->getNumBytes(objidx) << " bytes)" << endl;
        if ( objIDList.contains(id) || id == 0 ) {
            cout << "Error: Object ID collision found in object " << parser->getObjectName(objidx).toStdString() << ", modify object name" << endl;
            return RETURN_ERR_XML;
        }

        objIDList.append(id);
    }

    // done parsing and checking
    cout << "Done: processed " << xmlList.length() << " XML files and generated "
         << objIDList.length() << " objects with no ID collisions. Total size of the data fields is " << numBytesTotal << " bytes." << endl;
    

    if (verbose) 
        cout << "used units: " << parser->all_units.join(",").toStdString() << endl;

    if (do_none)
      return RETURN_OK;     
*/
    return RETURN_OK;
}

