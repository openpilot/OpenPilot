#include "op_dfu.h"
#include <cmath>
#include <qwaitcondition.h>
#include <QMutex>

OP_DFU::OP_DFU(bool _debug,bool _use_serial,QString portname,bool umodereset): debug(_debug),use_serial(_use_serial),mready(true)
{
    if(use_serial)
    {
        cout<<"Connect hw now and press any key";
        //  getch();
        // delay::msleep(2000);
        PortSettings settings;
        settings.BaudRate=BAUD57600;
        settings.DataBits=DATA_8;
        settings.FlowControl=FLOW_OFF;
        settings.Parity=PAR_NONE;
        settings.StopBits=STOP_1;
        settings.Timeout_Millisec=1000;
        info=new port(settings,portname);
        info->rxBuf 		= sspRxBuf;
        info->rxBufSize 	= MAX_PACKET_DATA_LEN;
        info->txBuf 		= sspTxBuf;
        info->txBufSize 	= MAX_PACKET_DATA_LEN;
        info->max_retry	= 10;
        info->timeoutLen	= 1000;
        if(info->status()!=port::open)
        {
            cout<<"Could not open serial port\n";
            mready=false;
            return;
        }
        if(umodereset)
            sendReset();
        delay::msleep(5000);
        serialhandle=new qsspt(info,debug);

        while(serialhandle->ssp_Synchronise()==false)
        {
             if (debug)
                 qDebug()<<"SYNC failed, resending";
        }
        qDebug()<<"SYNC Succeded";
        serialhandle->start();
        // serialhandle->start();
    }
    else
    {

        send_delay=10;
        use_delay=true;
        int numDevices=0;
        cout<<"Please connect device now\n";
        int count=0;
        while(numDevices==0)
        {
            cout<<".";
            delay::msleep(500);
            numDevices = hidHandle.open(1,0x20a0,0x415A,0,0); //0xff9c,0x0001);
            if(numDevices==0)
                numDevices = hidHandle.open(1,0x20a0,0x415B,0,0); //0xff9c,0x0001);
            if(++count==10)
            {
                cout<<"\r";
                cout<<"           ";
                cout<<"\r";
                count=0;
            }
        }
        if(debug)
            qDebug() << numDevices << " device(s) opened";
        if(umodereset)
        {
            sendReset();

            qDebug()<<"before delay";
            delay::msleep(5000);
            qDebug()<<"after delay";
            if(hidHandle.open(1,0x20a0,0x4117,0,0)==0)
                 mready=false;
        }
    }
}
void OP_DFU::sendReset(void)
{
    qDebug()<<"Requesting user mode reset";
    char aa[255]= {0x02,0x3E,0x3C,0x20,0x75,0x00,0x20,0x4F,0x67,0x34,0x62,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//63
    char ba[255]= {0x02,0x3E,0x3C,0x20,0x75,0x00,0x20,0x4F,0x67,0x34,0xB9,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    char ca[255]= {0x02,0x3E,0x3C,0x20,0x75,0x00,0x20,0x4F,0x67,0x34,0x10,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    char bb[255]={0x02,0x3D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x28,0x8D,0x19,0x00,0x00,0x13};
    char ab[255]={0x02,0x3D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x1B,0x19,0x00,0x00,0x76};
    char cb[255]={0x02,0x3D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x66,0x78,0x1C,0x00,0x00,0x25};
    //125
    if(!use_serial)
    {
    hidHandle.send(0,aa, 64, 5000);
    hidHandle.send(0,ab, 64, 5000);
    delay::msleep(600);
    hidHandle.send(0,ba, 64, 5000);
    hidHandle.send(0,bb, 64, 5000);
    delay::msleep(600);
    hidHandle.send(0,ca, 64, 5000);
    hidHandle.send(0,cb, 64, 5000);
    delay::msleep(100);
    hidHandle.close(1);
}
    else
    {
        char a[255];
        char b[255];
        char c[255];
        memcpy (a,aa+2,62);
        memcpy (a+62,ab+2,60);
        memcpy (b,ba+2,62);
        memcpy (b+62,bb+2,60);
        memcpy (c,ca+2,62);
        memcpy (c+62,cb+2,60);
        for(int x=0;x<123;++x)
            info->pfSerialWrite(a[x]);
        delay::msleep(600);
        for(int x=0;x<123;++x)
            info->pfSerialWrite(b[x]);
        delay::msleep(600);
        for(int x=0;x<123;++x)
            info->pfSerialWrite(c[x]);
    }
}

bool OP_DFU::SaveByteArrayToFile(QString const & sfile, const QByteArray &array)
{
    QFile file(sfile);
    //QFile file("in.txt");
    if (!file.open(QIODevice::WriteOnly))
    {
        if(debug)
            qDebug()<<"Cant open file";
        return false;
    }
    file.write(array);
    file.close();
    return true;
}

bool OP_DFU::enterDFU(int const &devNumber)
{
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = OP_DFU::EnterDFU;//DFU Command
    buf[2] = 0;//DFU Count
    buf[3] = 0;//DFU Count
    buf[4] = 0;//DFU Count
    buf[5] = 0;//DFU Count
    buf[6] = devNumber;//DFU Data0
    buf[7] = 1;//DFU Data1
    buf[8] = 1;//DFU Data2
    buf[9] = 1;//DFU Data3

    int result = sendData(buf, BUF_LEN);
    if(result<1)
        return false;
    if(debug)
        qDebug() << result << " bytes sent";
    return true;
}
bool OP_DFU::StartUpload(qint32 const & numberOfBytes, TransferTypes const & type,quint32 crc)
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
    if(debug)
        qDebug() << result << " bytes sent";
    if(result>0)
    {
        return true;
    }
    return false;
}
bool OP_DFU::UploadData(qint32 const & numberOfBytes, QByteArray  & data)
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
       // if(int ret=StatusRequest()!=OP_DFU::uploading) return false;
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
OP_DFU::Status OP_DFU::UploadDescription(QString  & description)
{
    cout<<"Starting uploading description\n";
    if(description.length()%4!=0)
    {      
        int pad=description.length()/4;
        pad=(pad+1)*4;
        pad=pad-description.length();
        QString padding;
        padding.fill(' ',pad);
        description.append(padding);
    }
    if(!StartUpload(description.length(),OP_DFU::Descript,0))
        return OP_DFU::abort;
    QByteArray array=description.toAscii();
    if(!UploadData(description.length(),array))
    {
        return OP_DFU::abort;
    }
    if(!EndOperation())
    {
        return OP_DFU::abort;
    }
    int ret=StatusRequest();

    if(debug)
        qDebug()<<"Upload description Status="<<ret;
    return (OP_DFU::Status)ret;
}
QString OP_DFU::DownloadDescription(int const & numberOfChars)
{
    // enterDFU(devNumber);
    QByteArray arr=StartDownload(numberOfChars,Descript);
    QString str(arr);
    return str;

}
void OP_DFU::test()
{
    char buf[BUF_LEN];
    int result;
    buf[0]=0x02;
    buf[1]=11;
    buf[2]=11;
    buf[63]=333;
    while(true)
    {
        buf[0]=0x02;
        ++buf[1];
        ++buf[2];
        ++buf[63];
        sendData(buf,BUF_LEN);
        result = receiveData(buf,BUF_LEN);

        qDebug()<<"Result="<<result;
        qDebug()<<(int)buf[0]<<":"<<(int)buf[1]<<":"<<(int)buf[2]<<":"<<(int)buf[63];
    }}

QByteArray OP_DFU::StartDownload(qint32 const & numberOfBytes, TransferTypes const & type)
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
    QByteArray ret;
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = OP_DFU::Download_Req;//DFU Command
    buf[2] = numberOfPackets>>24;//DFU Count
    buf[3] = numberOfPackets>>16;//DFU Count
    buf[4] = numberOfPackets>>8;//DFU Count
    buf[5] = numberOfPackets;//DFU Count
    buf[6] = (int)type;//DFU Data0
    buf[7] = lastPacketCount;//DFU Data1
    buf[8] = 1;//DFU Data2
    buf[9] = 1;//DFU Data3

    int result = sendData(buf, BUF_LEN);
    if(debug)
        qDebug() << "StartDownload:"<<numberOfPackets<<"packets"<<" Last Packet Size="<<lastPacketCount<<" "<<result << " bytes sent";
    float percentage;
    int laspercentage;
    for(qint32 x=0;x<numberOfPackets;++x)
    {
        percentage=(float)(x+1)/numberOfPackets*100;
        if(laspercentage!=(int)percentage)
            printProgBar((int)percentage,"DOWNLOADING");
        laspercentage=(int)percentage;


        //  qDebug()<<"Status="<<StatusToString(StatusRequest());
        result = receiveData(buf,BUF_LEN);
        if(debug)
            qDebug() << result << " bytes received"<<" Count="<<x<<"-"<<(int)buf[2]<<";"<<(int)buf[3]<<";"<<(int)buf[4]<<";"<<(int)buf[5]<<" Data="<<(int)buf[6]<<";"<<(int)buf[7]<<";"<<(int)buf[8]<<";"<<(int)buf[9];
        int size;
        if(x==numberOfPackets-1)
            size=lastPacketCount*4;
        else
            size=14*4;
        ret.append(buf+6,size);
    }
    cout<<"\n";
    StatusRequest();
    return ret;
}
void OP_DFU::ResetDevice(void)
{
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = OP_DFU::Reset;//DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;
    int result = sendData(buf, BUF_LEN);
}
void OP_DFU::AbortOperation(void)
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
    int result = sendData(buf, BUF_LEN);
}
void OP_DFU::JumpToApp()
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

    int result = sendData(buf, BUF_LEN);
}
OP_DFU::Status OP_DFU::StatusRequest()
{
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = OP_DFU::Status_Request;//DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;

    int result = sendData(buf, BUF_LEN);
    if(debug)
        qDebug() << result << " bytes sent";
    result = receiveData(buf,BUF_LEN);
    if(debug)
        qDebug() << result << " bytes received";
    if(buf[1]==OP_DFU::Status_Rep)
    {
        return (OP_DFU::Status)buf[6];
    }
    else
        return OP_DFU::abort;


}
bool OP_DFU::findDevices()
{
    devices.clear();
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = OP_DFU::Req_Capabilities;//DFU Command
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    buf[8] = 0;
    buf[9] = 0;
    int result = sendData(buf, BUF_LEN);
    if(result<1)
    {
        return false;
    }
    result = receiveData(buf,BUF_LEN);
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
            buf[0] =0x02;//reportID
            buf[1] = OP_DFU::Req_Capabilities;//DFU Command
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
          //  devices[x].ID=buf[9];
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
            qDebug()<<"Found "<<numberOfDevices;
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
bool OP_DFU::EndOperation()
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
    // hidHandle.receive(0,buf,BUF_LEN,5000);
    if(debug)
        qDebug() << result << " bytes sent";
    if(result>0)
        return true;
    return false;
}
OP_DFU::Status OP_DFU::UploadFirmware(const QString &sfile, const bool &verify,int device)
{
    OP_DFU::Status ret;
    cout<<"Starting Firmware Uploading...\n";
    QFile file(sfile);
    //QFile file("in.txt");
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
    if(devices[device].SizeOfCode<arr.length())
    {
        cout<<"ERROR file to big for device\n";
        return OP_DFU::abort;;
    }
    quint32 crc=CRCFromQBArray(arr,devices[device].SizeOfCode);
    cout<<"NEW FIRMWARE CRC="<<crc<<"\n";
    if(!StartUpload(arr.length(),FW,crc))
    {
        if(debug)
        {
            qDebug()<<"StartUpload failed";
            OP_DFU::Status ret=StatusRequest();
            qDebug()<<"StartUpload returned:"<< StatusToString(ret);
        }
        return OP_DFU::abort;
    }
    cout<<"Erasing memory\n";
    if(StatusRequest()==OP_DFU::abort) return OP_DFU::abort;
    for(int x=0;x<3;++x)
    {
        OP_DFU::Status ret=StatusRequest();
        if(debug)
            qDebug()<<"Erase returned:"<<StatusToString(ret);
        if (ret==OP_DFU::uploading)
            break;
        else
            return OP_DFU::abort;
    }
    if(!UploadData(arr.length(),arr))
    {
        if(debug)
        {
            qDebug()<<"Upload failed (upload data)";
            OP_DFU::Status ret=StatusRequest();
            qDebug()<<"StartUpload returned:"<<StatusToString(ret);
        }
        return OP_DFU::abort;
    }
    if(!EndOperation())
    {
        if(debug)
        {
            qDebug()<<"Upload failed (end operation)";
            OP_DFU::Status ret=StatusRequest();
            qDebug()<<"StartUpload returned:"<<StatusToString(ret);
        }
        return OP_DFU::abort;
    }
    ret=StatusRequest();
    if(ret==OP_DFU::Last_operation_Success)
    {

    }
    else
    {
        return ret;
    }
    if(verify)
    {
        cout<<"Starting code verification\n";
        if(arr==StartDownload(arr.length(),OP_DFU::FW))
            cout<<"Verify:PASSED\n";
        else
        {
            cout<<"Verify:FAILED\n";
            return OP_DFU::abort;
        }
    }
    else
    {
        return ret;
    }
    if(debug)
        qDebug()<<"Status="<<ret;
    cout<<"Firmware Uploading succeeded\n";
    return ret;
}
OP_DFU::Status OP_DFU::CompareFirmware(const QString &sfile, const CompareType &type,int device)
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
        if(arr==StartDownload(arr.length(),OP_DFU::FW))
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

void OP_DFU::CopyWords(char *source, char *destination, int count)
{
    for (int x=0;x<count;x=x+4)
    {
        *(destination+x)=source[x+3];
        *(destination+x+1)=source[x+2];
        *(destination+x+2)=source[x+1];
        *(destination+x+3)=source[x+0];
    }
}
QString OP_DFU::StatusToString(OP_DFU::Status const & status)
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
void OP_DFU::printProgBar( int const & percent,QString const& label){
    std::string bar;

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
quint32 OP_DFU::CRC32WideFast(quint32 Crc, quint32 Size, quint32 *Buffer)
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
quint32 OP_DFU::CRCFromQBArray(QByteArray array, quint32 Size)
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
int OP_DFU::sendData(void * data,int size)
{
    if(!use_serial)
    {
        return hidHandle.send(0,data, size, 5000);
    }
    else
    {
        if(serialhandle->sendData((uint8_t*)data+1,size-1))
        {
             if (debug)
                qDebug()<<"packet sent"<<"data0"<<((uint8_t*)data+1)[0];
            return size;
        }
        if(debug)
            qDebug()<<"Serial send OVERRUN";
    }
}
int OP_DFU::receiveData(void * data,int size)
{
    if(!use_serial)
    {
        return hidHandle.receive(0,data, size, 10000);
    }
    else
    {
        int x;
        QTime time;
        time.start();
        while(true)
        {
            if(x=serialhandle->read_Packet(((char *) data)+1)!=-1||time.elapsed()>10000)
            {
                if(time.elapsed()>10000)
                    qDebug()<<"____timeout";
                return x;
            }
        }
    }
}
