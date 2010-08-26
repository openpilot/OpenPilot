#include "op_dfu.h"
#include <cmath>
OP_DFU::OP_DFU()
{
    qDebug() << "Hello";

    int numDevices = hidHandle.open(1,0x20a0,0x4117,0,0); //0xff9c,0x0001);
    if( numDevices == 0 )
        numDevices = hidHandle.open(1,0x0483,0,0,0);

    qDebug() << numDevices << " device(s) opened";
}
void OP_DFU::enterDFU(int devNumber)
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

    int result = hidHandle.send(0,buf, BUF_LEN, 500);

    qDebug() << result << " bytes sent";
}
void OP_DFU::StartUpload(qint32 numberOfBytes, TransferTypes type)
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
    buf[8] = 1;//DFU Data2
    buf[9] = 1;//DFU Data3

    int result = hidHandle.send(0,buf, BUF_LEN, 5000);

    qDebug() << result << " bytes sent";
}
void OP_DFU::UploadData(qint32 numberOfBytes, QByteArray data)
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
    qDebug()<<"Start Uploading:"<<numberOfPackets<<"4Bytes";
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = OP_DFU::Upload;//DFU Command
    int packetsize;
    for(qint32 packetcount=0;packetcount<numberOfPackets;++packetcount)
    {
        if(packetcount==numberOfPackets)
            packetsize=lastPacketCount;
        else
            packetsize=14;
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
        int result = hidHandle.send(0,buf, BUF_LEN, 5000);

      //  qDebug() << "UPLOAD:"<<"Data="<<(int)buf[6]<<(int)buf[7]<<(int)buf[8]<<(int)buf[9]<<";"<<result << " bytes sent";
    }
}
void OP_DFU::UploadDescription(int devNumber, QString description)
{
    enterDFU(devNumber);
    if(description.length()%2!=0)
    {

        int pad=description.length()/4;
        pad=(pad+1)*4;
        pad=pad-description.length();
        QString padding;
        padding.fill(' ',pad);
        description.append(padding);
    }
    StartUpload(description.length()/4,OP_DFU::Descript);
    QByteArray array=description.toAscii();
    UploadData(description.length()/4,array);
    EndOperation();
    int ret=StatusRequest();
    qDebug()<<"Status="<<ret;
}
QString OP_DFU::DownloadDescription(int devNumber, int numberOfChars)
{
   // enterDFU(devNumber);
    QByteArray arr=StartDownload(numberOfChars/4,Descript);
    QString str(arr);
    return str;

}
QByteArray OP_DFU::StartDownload(qint32 numberOfPackets, TransferTypes type)
{
    QByteArray ret;
    char buf[BUF_LEN];
    buf[0] =0x02;//reportID
    buf[1] = OP_DFU::Download_Req;//DFU Command
    buf[2] = (char)numberOfPackets>>24;//DFU Count
    buf[3] = (char)numberOfPackets>>16;//DFU Count
    buf[4] = (char)numberOfPackets>>8;//DFU Count
    buf[5] = (char)numberOfPackets;//DFU Count
    buf[6] = (int)type;//DFU Data0
    buf[7] = 1;//DFU Data1
    buf[8] = 1;//DFU Data2
    buf[9] = 1;//DFU Data3

    int result = hidHandle.send(0,buf, BUF_LEN, 500);

    qDebug() << "StartDownload:"<<result << " bytes sent";
    for(qint32 x=0;x<numberOfPackets;++x)
    {
        result = hidHandle.receive(0,buf,BUF_LEN,5000);
        qDebug() << result << " bytes received"<<" Count="<<(int)buf[2]<<";"<<(int)buf[3]<<";"<<(int)buf[4]<<";"<<(int)buf[5]<<" Data="<<(int)buf[6]<<";"<<(int)buf[7]<<";"<<(int)buf[8]<<";"<<(int)buf[9];
        ret.append(buf+6,4);
    }
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

    int result = hidHandle.send(0,buf, BUF_LEN, 500);
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

    int result = hidHandle.send(0,buf, BUF_LEN, 500);
}
int OP_DFU::StatusRequest()
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

    int result = hidHandle.send(0,buf, BUF_LEN, 500);
    hidHandle.receive(0,buf,BUF_LEN,500);
    qDebug() << result << " bytes sent";
    result = hidHandle.receive(0,buf,BUF_LEN,500);
    qDebug() << result << " bytes received";
    if(buf[1]==OP_DFU::Status_Rep)
    {
        return buf[6];
    }
    else
        return -1;


}
void OP_DFU::EndOperation()
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

    int result = hidHandle.send(0,buf, BUF_LEN, 5000);
    hidHandle.receive(0,buf,BUF_LEN,5000);
    qDebug() << result << " bytes sent";
}
void OP_DFU::UploadFirmware(const QString &sfile)
{
    enterDFU(1);
    QFile file(sfile);
    //QFile file("in.txt");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"Cant open file";
             return;
                 }
    QByteArray arr=file.readAll();
    qDebug()<<"Bytes Loaded="<<arr.length();
    if(arr.length()%4!=0)
    {
    int pad=arr.length()/4;
    ++pad;
    pad=pad*4;
    pad=pad-arr.length();
    arr.append(QByteArray(pad,255));
}
    StartUpload(arr.length(),FW);
    UploadData(arr.length(),arr);
    EndOperation();
    int ret=StatusRequest();
    qDebug()<<"Status="<<ret;

}
void OP_DFU::CopyWords(char *source, char *destination, int count)
{
    for (int x=0;x<count;x=x+4)
    {
        //qDebug()<<(int)source[x*4+3]<<"xxx="<<4*x;
        *(destination+x)=source[x+3];
        *(destination+x+1)=source[x+2];
        *(destination+x+2)=source[x+1];
        *(destination+x+3)=source[x+0];
    }
}
