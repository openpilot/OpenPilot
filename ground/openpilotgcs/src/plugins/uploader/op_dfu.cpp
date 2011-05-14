/**
 ******************************************************************************
 *
 * @file       op_dfu.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Uploader Uploader Plugin
 * @{
 * @brief The uploader plugin
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

#include "op_dfu.h"
#include <cmath>
#include <qwaitcondition.h>
#include <QMetaType>
#include <QApplication>

using namespace OP_DFU;

DFUObject::DFUObject(bool _debug,bool _use_serial,QString portname):
    debug(_debug),use_serial(_use_serial),mready(true)
{
    info = NULL;

    qRegisterMetaType<OP_DFU::Status>("Status");

    if(use_serial)
    {
        PortSettings settings;
        settings.BaudRate=BAUD57600;
        settings.DataBits=DATA_8;
        settings.FlowControl=FLOW_OFF;
        settings.Parity=PAR_NONE;
        settings.StopBits=STOP_1;
        settings.Timeout_Millisec=1000;
        info = new port(settings,portname);
        info->rxBuf 		= sspRxBuf;
        info->rxBufSize 	= MAX_PACKET_DATA_LEN;
        info->txBuf 		= sspTxBuf;
        info->txBufSize 	= MAX_PACKET_DATA_LEN;
        info->max_retry	= 10;
        info->timeoutLen	= 1000;
        if(info->status()!=port::open)
        {
            cout << "Could not open serial port\n";
            mready = false;
            return;
        }
        serialhandle = new qsspt(info,debug);

        int count = 0;
        while((serialhandle->ssp_Synchronise()==false) && (count < 10))
        {
             if (debug)
                 qDebug()<<"SYNC failed, resending";
             count++;
        }
        if (count == 10) {
            mready = false;
            return;
        }
        qDebug() << "SYNC Succeded";
        serialhandle->start();
    }
    else
    {
        send_delay=10;
        use_delay=true;
//        int numDevices=0;
        QList<USBPortInfo> devices;
        int count=0;
        while((devices.length()==0) && count < 10)
        {
            if (debug)
                qDebug() << ".";
            delay::msleep(500);
            // processEvents enables XP to process the system
            // plug/unplug events, otherwise it will not process
            // those events before the end of the call!
            QApplication::processEvents();
            devices = USBMonitor::instance()->availableDevices(0x20a0,-1,-1,USBMonitor::Bootloader);
            count++;
        }
       if (devices.length()==1) {
           hidHandle.open(1,devices.first().vendorID,devices.first().productID,0,0);
        } else {
           qDebug() << "More than one device, don't know what to do!";
           mready = false;
       }

    }
}

DFUObject::~DFUObject()
{
    if (use_serial) {
        if (mready) {
            delete serialhandle;
            delete info;
        }
    } else {
        hidHandle.close(0);
    }

}

bool DFUObject::SaveByteArrayToFile(QString const & sfile, const QByteArray &array)
{
    QFile file(sfile);
    if (!file.open(QIODevice::WriteOnly))
    {
        if(debug)
            qDebug()<<"Can't open file";
        return false;
    }
    file.write(array);
    file.close();
    return true;
}

/**
  Tells the mainboard to enter DFU Mode.
  */
bool DFUObject::enterDFU(int const &devNumber)
{
    char buf[BUF_LEN];
    buf[0] =0x02;              //reportID
    buf[1] = OP_DFU::EnterDFU; //DFU Command
    buf[2] = 0;                //DFU Count
    buf[3] = 0;                //DFU Count
    buf[4] = 0;                //DFU Count
    buf[5] = 0;                //DFU Count
    buf[6] = devNumber;        //DFU Data0
    buf[7] = 1;                //DFU Data1
    buf[8] = 1;                //DFU Data2
    buf[9] = 1;                //DFU Data3

    int result = sendData(buf, BUF_LEN);
    // int result = hidHandle.send(0,buf, BUF_LEN, 500);
    if(result<1)
        return false;
    if(debug)
        qDebug() << "EnterDFU: " << result << " bytes sent";
    return true;
}

/**
  Tells the board to get ready for an upload. It will in particular
  erase the memory to make room for the data. You will have to query
  its status to wait until erase is done before doing the actual upload.
  */
bool DFUObject::StartUpload(qint32 const & numberOfBytes, TransferTypes const & type,quint32 crc)
{
    int lastPacketCount;
    qint32 numberOfPackets=numberOfBytes/4/14;
    int pad=(numberOfBytes-numberOfPackets*4*14)/4;
    if(pad==0)
    {
        lastPacketCount=14;
    }
    else
    {
        ++numberOfPackets;
        lastPacketCount=pad;
    }
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = setStartBit(OP_DFU::Upload);//DFU Command
    buf[2] = numberOfPackets>>24;//DFU Count
    buf[3] = numberOfPackets>>16;//DFU Count
    buf[4] = numberOfPackets>>8;//DFU Count
    buf[5] = numberOfPackets;//DFU Count
    buf[6] = (int)type;//DFU Data0
    buf[7] = lastPacketCount;//DFU Data1
    buf[8] = crc>>24;
    buf[9] = crc>>16;
    buf[10] = crc>>8;
    buf[11] = crc;
    if(debug)
        qDebug()<<"Number of packets:"<<numberOfPackets<<" Size of last packet:"<<lastPacketCount;

    int result = sendData(buf, BUF_LEN);
    delay::msleep(1000);
    // int result = hidHandle.send(0,buf, BUF_LEN, 5000);

    if(debug)
        qDebug() << result << " bytes sent";
    if(result>0)
    {
        return true;
    }
    return false;
}


/**
  Does the actual data upload to the board. Needs to be called once the
  board is ready to accept data following a StartUpload command, and it is erased.
  */
bool DFUObject::UploadData(qint32 const & numberOfBytes, QByteArray  & data)
{
    int lastPacketCount;
    qint32 numberOfPackets=numberOfBytes/4/14;
    int pad=(numberOfBytes-numberOfPackets*4*14)/4;
    if(pad==0)
    {
        lastPacketCount=14;
    }
    else
    {
        ++numberOfPackets;
        lastPacketCount=pad;
    }
    if(debug)
        qDebug()<<"Start Uploading:"<<numberOfPackets<<"4Bytes";
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = OP_DFU::Upload;//DFU Command
    int packetsize;
    float percentage;
    int laspercentage;
    for(qint32 packetcount=0;packetcount<numberOfPackets;++packetcount)
    {
        percentage=(float)(packetcount+1)/numberOfPackets*100;
        if(laspercentage!=(int)percentage)
            printProgBar((int)percentage,"UPLOADING");
        laspercentage=(int)percentage;
        if(packetcount==numberOfPackets)
            packetsize=lastPacketCount;
        else
            packetsize=14;
       // qDebug()<<packetcount;
        buf[2] = packetcount>>24;//DFU Count
        buf[3] = packetcount>>16;//DFU Count
        buf[4] = packetcount>>8;//DFU Count
        buf[5] = packetcount;//DFU Count
        char *pointer=data.data();
        pointer=pointer+4*14*packetcount;
        //  qDebug()<<"Packet Number="<<packetcount<<"Data0="<<(int)data[0]<<" Data1="<<(int)data[1]<<" Data0="<<(int)data[2]<<" Data0="<<(int)data[3]<<" buf6="<<(int)buf[6]<<" buf7="<<(int)buf[7]<<" buf8="<<(int)buf[8]<<" buf9="<<(int)buf[9];
        CopyWords(pointer,buf+6,packetsize*4);
        //        for (int y=0;y<packetsize*4;++y)
        //        {

        //                qDebug()<<y<<":"<<(int)data[packetcount*14*4+y]<<"---"<<(int)buf[6+y];


        //        }
        // qDebug()<<" Data0="<<(int)data[0]<<" Data0="<<(int)data[1]<<" Data0="<<(int)data[2]<<" Data0="<<(int)data[3]<<" buf6="<<(int)buf[6]<<" buf7="<<(int)buf[7]<<" buf8="<<(int)buf[8]<<" buf9="<<(int)buf[9];
        //delay::msleep(send_delay);

        //if(StatusRequest()!=OP_DFU::uploading) return false;
        int result = sendData(buf, BUF_LEN);
     //   qDebug()<<"sent:"<<result;
        if(result<1)
        {
            return false;
        }

        //  qDebug() << "UPLOAD:"<<"Data="<<(int)buf[6]<<(int)buf[7]<<(int)buf[8]<<(int)buf[9]<<";"<<result << " bytes sent";

    }
    cout<<"\n";
    // while(true){}
    return true;
}

/**
  Sends the firmware description to the device
  */
OP_DFU::Status DFUObject::UploadDescription(QVariant desc)
{
     cout<<"Starting uploading description\n";
     QByteArray array;

    if (desc.type() == QMetaType::QString) {
        QString description = desc.toString();
        if(description.length()%4!=0)
        {
            int pad=description.length()/4;
            pad=(pad+1)*4;
            pad=pad-description.length();
            QString padding;
            padding.fill(' ',pad);
            description.append(padding);
        }
        array=description.toAscii();

    } else if (desc.type() == QMetaType::QByteArray) {
        array = desc.toByteArray();
    }

    if(!StartUpload(array.length(),OP_DFU::Descript,0))
        return OP_DFU::abort;
    if(!UploadData(array.length(),array))
    {
        return OP_DFU::abort;
    }
    if(!EndOperation())
    {
        return OP_DFU::abort;
    }
    OP_DFU::Status ret = StatusRequest();


    if(debug)
        qDebug() << "Upload description Status=" << StatusToString(ret);
    return ret;
}


/**
  Downloads the description string for the current device.
  You have to call enterDFU before calling this function.
*/
QString DFUObject::DownloadDescription(int const & numberOfChars)
{

    QByteArray arr;
    StartDownloadT(&arr, numberOfChars,OP_DFU::Descript);
    QString str(arr);
    return str;

}

QByteArray DFUObject::DownloadDescriptionAsBA(int const & numberOfChars)
{

    QByteArray arr;
    StartDownloadT(&arr, numberOfChars,OP_DFU::Descript);
    return arr;

}

/**
  Starts a firmware download
  @param firmwareArray: pointer to the location where we should store the firmware
  @package device: the device to use for the download
  */
bool DFUObject::DownloadFirmware(QByteArray *firmwareArray, int device)
{

    if (isRunning())
        return false;
    requestedOperation = OP_DFU::Download;
    requestSize = devices[device].SizeOfCode;
    requestTransferType = OP_DFU::FW;
    requestStorage = firmwareArray;
    start();
    return true;
}


/**
   Runs the upload or download operations.
  */
void DFUObject::run()
{

    switch (requestedOperation) {
        case OP_DFU::Download:
            StartDownloadT(requestStorage, requestSize, requestTransferType);
            emit(downloadFinished());
            break;
        case OP_DFU::Upload: {
            OP_DFU::Status ret = UploadFirmwareT(requestFilename, requestVerify, requestDevice);
            emit(uploadFinished(ret));
            break;
        }
        default:
        break;
    }

    return;
}

/**
  Downloads a certain number of bytes from a certain location, and stores in an array whose
  pointer is passed as an argument
  */
bool DFUObject::StartDownloadT(QByteArray *fw, qint32 const & numberOfBytes, TransferTypes const & type)
{
    int lastPacketCount;

    // First of all, work out the number of DFU packets we should ask for:
    qint32 numberOfPackets = numberOfBytes/4/14;
    int pad = (numberOfBytes-numberOfPackets*4*14)/4;
    if(pad == 0) {
        lastPacketCount=14;
    }
    else {
        ++numberOfPackets;
        lastPacketCount=pad;
    }

    char buf[BUF_LEN];

    buf[0] = 0x02;                  //reportID
    buf[1] = OP_DFU::Download_Req;  //DFU Command
    buf[2] = numberOfPackets>>24;   //DFU Count
    buf[3] = numberOfPackets>>16;   //DFU Count
    buf[4] = numberOfPackets>>8;    //DFU Count
    buf[5] = numberOfPackets;       //DFU Count
    buf[6] = (int)type;             //DFU Data0
    buf[7] = lastPacketCount;       //DFU Data1
    buf[8] = 1;                     //DFU Data2
    buf[9] = 1;                     //DFU Data3

    int result = sendData(buf, BUF_LEN);
    //int result = hidHandle.send(0,buf, BUF_LEN, 500);
    if(debug)
        qDebug() << "StartDownload:"<<numberOfPackets<<"packets"<<" Last Packet Size="<<lastPacketCount<<" "<<result << " bytes sent";
    float percentage;
    int laspercentage;

    // Now get those packets:
    for(qint32 x=0;x<numberOfPackets;++x)
    {
        int size;
        percentage=(float)(x+1)/numberOfPackets*100;
        if(laspercentage!=(int)percentage)
            printProgBar((int)percentage,"DOWNLOADING");
        laspercentage=(int)percentage;

        result = receiveData(buf,BUF_LEN);
        //result = hidHandle.receive(0,buf,BUF_LEN,5000);
        if(debug)
            qDebug() << result << " bytes received"<<" Count="<<x<<"-"<<(int)buf[2]<<";"<<(int)buf[3]<<";"<<(int)buf[4]<<";"<<(int)buf[5]<<" Data="<<(int)buf[6]<<";"<<(int)buf[7]<<";"<<(int)buf[8]<<";"<<(int)buf[9];
        if(x==numberOfPackets-1)
            size=lastPacketCount*4;
        else
            size=14*4;
        fw->append(buf+6,size);
    }

    StatusRequest();
    return true;
}


/**
  Resets the device
  */
int DFUObject::ResetDevice(void)
{
    char buf[BUF_LEN];
    buf[0] =0x02;           //reportID
    buf[1] = OP_DFU::Reset; //DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    return sendData(buf, BUF_LEN);
    //return hidHandle.send(0,buf, BUF_LEN, 500);
}

int DFUObject::AbortOperation(void)
{
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = OP_DFU::Abort_Operation;//DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    return sendData(buf, BUF_LEN);
    //return hidHandle.send(0,buf, BUF_LEN, 500);
}

/**
  Starts the firmware (leaves bootloader and boots the main software)
  */
int DFUObject::JumpToApp()
{
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = OP_DFU::JumpFW;//DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    return sendData(buf, BUF_LEN);
    //return hidHandle.send(0,buf, BUF_LEN, 500);
}

OP_DFU::Status DFUObject::StatusRequest()
{
    char buf[BUF_LEN];
    buf[0] =0x02;                    //reportID
    buf[1] = OP_DFU::Status_Request; //DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    int result = sendData(buf, BUF_LEN);
    //int result = hidHandle.send(0,buf, BUF_LEN, 10000);
    if(debug)
        qDebug() << "StatusRequest: " << result << " bytes sent";
    result = receiveData(buf,BUF_LEN);
    // result = hidHandle.receive(0,buf,BUF_LEN,10000);
    if(debug)
        qDebug() << "StatusRequest: " << result << " bytes received";
    if(buf[1]==OP_DFU::Status_Rep)
    {
        return (OP_DFU::Status)buf[6];
    }
    else
        return OP_DFU::abort;
}

/**
  Ask the bootloader for the list of devices available
  */
bool DFUObject::findDevices()
{
    devices.clear();
    char buf[BUF_LEN];
    buf[0] =0x02;                      //reportID
    buf[1] = OP_DFU::Req_Capabilities; //DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    int result = sendData(buf, BUF_LEN);
    //int result = hidHandle.send(0,buf, BUF_LEN, 5000);
    if(result<1)
    {
        return false;
    }
    result = receiveData(buf,BUF_LEN);
    //result = hidHandle.receive(0,buf,BUF_LEN,5000);
    if(result<1)
    {
        return false;
    }
    numberOfDevices=buf[7];
    RWFlags=buf[8];
    RWFlags=RWFlags<<8 | buf[9];


    if(buf[1]==OP_DFU::Rep_Capabilities)
    {
        for(int x=0;x<numberOfDevices;++x)
        {
            device dev;
            dev.Readable=(bool)(RWFlags>>(x*2) & 1);
            dev.Writable=(bool)(RWFlags>>(x*2+1) & 1);
            devices.append(dev);
            buf[0] =0x02;                      //reportID
            buf[1] = OP_DFU::Req_Capabilities; //DFU Command
            buf[2] = 0;
            buf[3] = 0;
            buf[4] = 0;
            buf[5] = 0;
            buf[6] = x+1;
            buf[7] = 0;
            buf[8] = 0;
            buf[9] = 0;
            int result = sendData(buf, BUF_LEN);
            result = receiveData(buf,BUF_LEN);
            // int result = hidHandle.send(0,buf, BUF_LEN, 5000);
            // result = hidHandle.receive(0,buf,BUF_LEN,5000);
            //devices[x].ID=buf[9];
            devices[x].ID=buf[14];
            devices[x].ID=devices[x].ID<<8 | (quint8)buf[15];
            devices[x].BL_Version=buf[7];
            devices[x].SizeOfDesc=buf[8];

            quint32 aux;
            aux=(quint8)buf[10];
            aux=aux<<8 |(quint8)buf[11];
            aux=aux<<8 |(quint8)buf[12];
            aux=aux<<8 |(quint8)buf[13];

            devices[x].FW_CRC=aux;


            aux=(quint8)buf[2];
            aux=aux<<8 |(quint8)buf[3];
            aux=aux<<8 |(quint8)buf[4];
            aux=aux<<8 |(quint8)buf[5];
            devices[x].SizeOfCode=aux;
        }
        if(debug)
        {
            qDebug() << "Found " << numberOfDevices << " devices";
            for(int x=0;x<numberOfDevices;++x)
            {
                qDebug()<<"Device #"<<x+1;
                qDebug()<<"Device ID="<<devices[x].ID;
                qDebug()<<"Device Readable="<<devices[x].Readable;
                qDebug()<<"Device Writable="<<devices[x].Writable;
                qDebug()<<"Device SizeOfCode="<<devices[x].SizeOfCode;
                qDebug()<<"Device SizeOfDesc="<<devices[x].SizeOfDesc;
                qDebug()<<"BL Version="<<devices[x].BL_Version;
                qDebug()<<"FW CRC="<<devices[x].FW_CRC;
            }
        }
    }
    return true;
}


bool DFUObject::EndOperation()
{
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = OP_DFU::Op_END;//DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    int result = sendData(buf, BUF_LEN);
    // int result = hidHandle.send(0,buf, BUF_LEN, 5000);
    // hidHandle.receive(0,buf,BUF_LEN,5000);
    if(debug)
        qDebug() << result << " bytes sent";
    if(result>0)
        return true;
    return false;
}


//
/**
  Starts a firmware upload (asynchronous)
  */
bool DFUObject::UploadFirmware(const QString &sfile, const bool &verify,int device)
{

    if (isRunning())
        return false;
    requestedOperation = OP_DFU::Upload;
    requestFilename = sfile;
    requestDevice = device;
    requestVerify = verify;
    start();
    return true;
}

OP_DFU::Status DFUObject::UploadFirmwareT(const QString &sfile, const bool &verify,int device)
{
    OP_DFU::Status ret;

    if (debug)
        qDebug() <<"Starting Firmware Uploading...";

    QFile file(sfile);

    if (!file.open(QIODevice::ReadOnly))
    {
        if(debug)
            qDebug()<<"Cant open file";
        return OP_DFU::abort;
    }

    QByteArray arr = file.readAll();

    if(debug)
        qDebug()<<"Bytes Loaded="<<arr.length();
    if(arr.length()%4!=0)
    {
        int pad=arr.length()/4;
        ++pad;
        pad=pad*4;
        pad=pad-arr.length();
        arr.append(QByteArray(pad,255));
    }
    if( devices[device].SizeOfCode < arr.length())
    {
        if (debug)
            qDebug() << "ERROR file to big for device";
        return OP_DFU::abort;;
    }

    quint32 crc=CRCFromQBArray(arr,devices[device].SizeOfCode);
    if (debug)
        qDebug() << "NEW FIRMWARE CRC=" << crc;

    if( !StartUpload(arr.length(), OP_DFU::FW, crc))
    {
        ret = StatusRequest();
        if(debug)
        {
            qDebug() << "StartUpload failed";
            qDebug() << "StartUpload returned:" << StatusToString(ret);
        }
        return ret;
    }

    emit operationProgress(QString("Erasing, please wait..."));

    if (debug) qDebug() << "Erasing memory";
    if( StatusRequest() == OP_DFU::abort) return OP_DFU::abort;

    // TODO: why is there a loop there? The "if" statement
    // will cause a break or return anyway!!
    for (int x = 0; x < 3; ++x) {
        ret = StatusRequest();
        if (debug) qDebug() << "Erase returned: " << StatusToString(ret);
        if (ret == OP_DFU::uploading)
            break;
        else
            return ret;
    }

    emit operationProgress(QString("Uploading firmware"));
    if( !UploadData(arr.length(),arr))
    {
        ret = StatusRequest();
        if(debug)
        {
            qDebug()<<"Upload failed (upload data)";
            qDebug()<<"UploadData returned:"<<StatusToString(ret);
        }
        return ret;
    }
    if(!EndOperation())
    {
        ret = StatusRequest();
        if(debug)
        {
            qDebug()<<"Upload failed (end operation)";
            qDebug()<<"EndOperation returned:"<<StatusToString(ret);
        }
        return ret;
    }
    ret = StatusRequest();
    if(ret != OP_DFU::Last_operation_Success)
        return ret;

    if(verify) {
        emit operationProgress(QString("Verifying firmware"));
        cout<<"Starting code verification\n";
        QByteArray arr2;
        StartDownloadT(&arr2, arr.length(),OP_DFU::FW);
        if (arr != arr2 ) {
            cout<<"Verify:FAILED\n";
            return OP_DFU::abort;
        }
    }

    if(debug)
        qDebug()<<"Status="<<ret;
    cout<<"Firmware Uploading succeeded\n";
    return ret;
}



OP_DFU::Status DFUObject::CompareFirmware(const QString &sfile, const CompareType &type,int device)
{
    cout<<"Starting Firmware Compare...\n";
    QFile file(sfile);
    if (!file.open(QIODevice::ReadOnly))
    {
        if(debug)
            qDebug()<<"Cant open file";
        return OP_DFU::abort;
    }
    QByteArray arr=file.readAll();

    if(debug)
        qDebug()<<"Bytes Loaded="<<arr.length();
    if(arr.length()%4!=0)
    {
        int pad=arr.length()/4;
        ++pad;
        pad=pad*4;
        pad=pad-arr.length();
        arr.append(QByteArray(pad,255));
    }
    if(type==OP_DFU::crccompare)
    {
         quint32 crc=CRCFromQBArray(arr,devices[device].SizeOfCode);
         if(crc==devices[device].FW_CRC)
         {
             cout<<"Compare Successfull CRC MATCH!\n";
         }
         else
         {
             cout<<"Compare failed CRC DONT MATCH!\n";
         }
         return StatusRequest();
     }
    else
    {
        QByteArray arr2;
        StartDownloadT(&arr2, arr.length(), OP_DFU::FW);
        if(arr == arr2)
        {
            cout<<"Compare Successfull ALL Bytes MATCH!\n";
        }
        else
        {
            cout<<"Compare failed Bytes DONT MATCH!\n";
        }
        return StatusRequest();
    }
}

void DFUObject::CopyWords(char *source, char *destination, int count)
{
    for (int x=0;x<count;x=x+4)
    {
        *(destination+x)=source[x+3];
        *(destination+x+1)=source[x+2];
        *(destination+x+2)=source[x+1];
        *(destination+x+3)=source[x+0];
    }
}
QString DFUObject::StatusToString(OP_DFU::Status const & status)
{
    switch(status)
    {
    case DFUidle:
        return "DFUidle";
    case uploading:
        return "";
    case wrong_packet_received:
        return "wrong_packet_received";
    case too_many_packets:
        return "too_many_packets";
    case too_few_packets:
        return "too_few_packets";
    case Last_operation_Success:
        return "Last_operation_Success";
    case downloading:
        return "downloading";
    case idle:
        return "idle";
    case Last_operation_failed:
        return "Last_operation_failed";
    case outsideDevCapabilities:
        return "outsideDevCapabilities";
    case CRC_Fail:
        return "CRC check FAILED";
    case failed_jump:
        return "Jmp to user FW failed";
    case abort:
        return "abort";
    case uploadingStarting:
        return "Uploading Starting";
    default:
        return "unknown";

    }
}

/**
  Prints a progress bar with percentage & label during an operation.

  Also outputs to stdout if we are in debug mode.
  */
void DFUObject::printProgBar( int const & percent,QString const& label){
    std::string bar;

    emit(progressUpdated(percent));

    if (debug) {
        for(int i = 0; i < 50; i++){
            if( i < (percent/2)){
                bar.replace(i,1,"=");
            }else if( i == (percent/2)){
                bar.replace(i,1,">");
            }else{
                bar.replace(i,1," ");
            }
        }

        std::cout<< "\r"<<label.toLatin1().data()<< "[" << bar << "] ";
        std::cout.width( 3 );
        std::cout<< percent << "%     " << std::flush;
    }
}

/**
  Utility function
  */
quint32 DFUObject::CRC32WideFast(quint32 Crc, quint32 Size, quint32 *Buffer)
{
    //Size = Size >> 2; // /4  Size passed in as a byte count, assumed to be a multiple of 4

    while(Size--)
    {
        static const quint32 CrcTable[16] = { // Nibble lookup table for 0x04C11DB7 polynomial
            0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
            0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD };

        Crc = Crc ^ *((quint32 *)Buffer); // Apply all 32-bits

        Buffer += 1;

        // Process 32-bits, 4 at a time, or 8 rounds

        Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; //  0x04C11DB7 Polynomial used in STM32
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
    }

    return(Crc);
}

/**
  Utility function
  */
quint32 DFUObject::CRCFromQBArray(QByteArray array, quint32 Size)
{
    int pad=Size-array.length();
    array.append(QByteArray(pad,255));
    quint32 t[Size/4];
    for(int x=0;x<array.length()/4;x++)
    {
        quint32 aux=0;
        aux=(char)array[x*4+3]&0xFF;
        aux=aux<<8;
        aux+=(char)array[x*4+2]&0xFF;
        aux=aux<<8;
        aux+=(char)array[x*4+1]&0xFF;
        aux=aux<<8;
        aux+=(char)array[x*4+0]&0xFF;
        t[x]=aux;
    }
    return CRC32WideFast(0xFFFFFFFF,Size/4,(quint32*)t);
}


/**
  Send data to the bootloader, either through the serial port
  of through the HID handle, depending on the mode we're using
  */
int DFUObject::sendData(void * data,int size)
{
    if(!use_serial)
        return hidHandle.send(0,data, size, 5000);

    // Serial Mode:
    if(serialhandle->sendData((uint8_t*)data+1,size-1))
    {
         if (debug)
            qDebug() << "packet sent" << "data0" << ((uint8_t*)data+1)[0];
        return size;
    }
    if(debug)
        qDebug() << "Serial send OVERRUN";
    return -1;
}


/**
  Receive data from the bootloader, either through the serial port
  of through the HID handle, depending on the mode we're using
  */
int DFUObject::receiveData(void * data,int size)
{
    if(!use_serial)
        return hidHandle.receive(0,data, size, 10000);

    // Serial Mode:
    int x;
    QTime time;
    time.start();
    while(true)
    {
        if((x=serialhandle->read_Packet(((char *) data)+1)!=-1) || time.elapsed()>10000)
        {
            if(time.elapsed()>10000)
                qDebug()<<"____timeout";
            return x;
        }
    }
}
