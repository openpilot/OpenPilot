#include "op_dfu.h"
#include <cmath>
#include <qwaitcondition.h>
#include <QMutex>

OP_DFU::OP_DFU(bool _debug): debug(_debug)
{
    QWaitCondition sleep;
    QMutex mutex;
    int numDevices=0;
    cout<<"Please connect device now\n";
    int count=0;
    while(numDevices==0)
    {
        cout<<".";
        mutex.lock();
        sleep.wait(&mutex,500);
        mutex.unlock();
        numDevices = hidHandle.open(1,0x20a0,0x4117,0,0); //0xff9c,0x0001);
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
}
bool OP_DFU::SaveByteArrayToFile(QString sfile, const QByteArray &array)
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

bool OP_DFU::enterDFU(int devNumber)
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
    if(result<1)
        return false;
    if(debug)
        qDebug() << result << " bytes sent";
    return true;
}
bool OP_DFU::StartUpload(qint32 numberOfBytes, TransferTypes type)
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
    qDebug()<<"Number of packets:"<<numberOfPackets<<" Size of last packet:"<<lastPacketCount;
    int result = hidHandle.send(0,buf, BUF_LEN, 5000);
    if(debug)
        qDebug() << result << " bytes sent";
    if(result>0)
    {
        return true;
    }
    return false;
}
bool OP_DFU::UploadData(qint32 numberOfBytes, QByteArray data)
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
        if(result<1)
        {
            return false;
        }

        //  qDebug() << "UPLOAD:"<<"Data="<<(int)buf[6]<<(int)buf[7]<<(int)buf[8]<<(int)buf[9]<<";"<<result << " bytes sent";

    }
    return true;
}
OP_DFU::Status OP_DFU::UploadDescription(QString description)
{
    if(description.length()%4!=0)
    {      
        int pad=description.length()/4;
        pad=(pad+1)*4;
        pad=pad-description.length();
        QString padding;
        padding.fill(' ',pad);
        description.append(padding);
    }
    if(!StartUpload(description.length(),OP_DFU::Descript))
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
    OP_DFU::Status ret=StatusRequest();
    if(debug)
        qDebug()<<"Upload description Status="<<ret;
    return ret;
}
QString OP_DFU::DownloadDescription(int numberOfChars)
{
    // enterDFU(devNumber);
    QByteArray arr=StartDownload(numberOfChars,Descript);
    QString str(arr);
    return str;

}
QByteArray OP_DFU::StartDownload(qint32 numberOfBytes, TransferTypes type)
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

    int result = hidHandle.send(0,buf, BUF_LEN, 500);
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
        result = hidHandle.receive(0,buf,BUF_LEN,5000);
        if(debug)
            qDebug() << result << " bytes received"<<" Count="<<x<<"-"<<(int)buf[2]<<";"<<(int)buf[3]<<";"<<(int)buf[4]<<";"<<(int)buf[5]<<" Data="<<(int)buf[6]<<";"<<(int)buf[7]<<";"<<(int)buf[8]<<";"<<(int)buf[9];
        int size;
        if(x==numberOfPackets-1)
            size=lastPacketCount*4;
        else
            size=14*4;
        ret.append(buf+6,size);
    }
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

    int result = hidHandle.send(0,buf, BUF_LEN, 5000);
    if(debug)
        qDebug() << result << " bytes sent";
    result = hidHandle.receive(0,buf,BUF_LEN,5000);
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
    int result = hidHandle.send(0,buf, BUF_LEN, 5000);
    if(result<1)
    {
        return false;
    }
    result = hidHandle.receive(0,buf,BUF_LEN,5000);
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
            int result = hidHandle.send(0,buf, BUF_LEN, 5000);
            result = hidHandle.receive(0,buf,BUF_LEN,5000);
            devices[x].ID=buf[9];
            devices[x].SizeOfHash=buf[7];
            devices[x].SizeOfDesc=buf[8];
            quint32 aux;
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
                qDebug()<<"Device SizeOfHash="<<devices[x].SizeOfHash;
                qDebug()<<"Device SizeOfDesc="<<devices[x].SizeOfDesc;
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

    int result = hidHandle.send(0,buf, BUF_LEN, 5000);
    // hidHandle.receive(0,buf,BUF_LEN,5000);
    if(debug)
        qDebug() << result << " bytes sent";
    if(result>0)
        return true;
    return false;
}
OP_DFU::Status OP_DFU::UploadFirmware(const QString &sfile, const bool &verify)
{
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
    QByteArray hash=QCryptographicHash::hash(arr,QCryptographicHash::Sha1);
    if(debug)
        qDebug()<<"hash size="<<hash.length()<<" -"<<hash;
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
    if(!StartUpload(arr.length(),FW))
    {
        if(debug)
            qDebug()<<"StartUpload failed";
        return OP_DFU::abort;
    }
    if(!UploadData(arr.length(),arr))
    {
        if(debug)
            qDebug()<<"Upload failed";
        return OP_DFU::abort;
    }
    if(!EndOperation())
    {
        if(debug)
            qDebug()<<"Upload failed";
        return OP_DFU::abort;
    }
    OP_DFU::Status ret=StatusRequest();
    if(ret==OP_DFU::Last_operation_Success)
    {
        cout<<"Firmware Uploading succeeded...going to upload hash\n";
    }
    if(verify)
    {
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
    if(!StartUpload(hash.length(),Hash))
    {
        if(debug)
            qDebug()<<"StartUpload failed";
        return OP_DFU::abort;
    }
    if(!UploadData(hash.length(),hash))
    {
        if(debug)
            qDebug()<<"Upload failed";
        return OP_DFU::abort;
    }
    if(!EndOperation())
    {
        {
            if(debug)
                qDebug()<<"Upload failed";
            return OP_DFU::abort;
        }
    }
    ret=StatusRequest();
    if(debug)
        qDebug()<<"Status="<<ret;
    if(ret==OP_DFU::Last_operation_Success)
    {
        cout<<"Hash Uploading succeeded...\n";
    }
    return ret;
}
OP_DFU::Status OP_DFU::CompareFirmware(const QString &sfile, const CompareType &type)
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
    QByteArray hash=QCryptographicHash::hash(arr,QCryptographicHash::Sha1);
    if(debug)
        qDebug()<<"hash size="<<hash.length()<<" -"<<hash;
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
    if(type==OP_DFU::hashcompare)
    {
        if(hash==StartDownload(hash.length(),OP_DFU::Hash))
        {
            cout<<"Compare Successfull Hashes MATCH!\n";
        }
        else
        {
            cout<<"Compare failed Hashes DONT MATCH!\n";
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
QString OP_DFU::StatusToString(OP_DFU::Status status)
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
    case abort:
        return "abort";

    }
}
void OP_DFU::printProgBar( int percent,QString const& label){
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
