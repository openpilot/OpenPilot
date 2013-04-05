#include <QtCore/QCoreApplication>
#include "../../../libs/qextserialport/src/qextserialport.h"
#include <QTime>
#include <QDebug>
#include "qssp.h"
#include "port.h"
#include "qsspt.h"
int main(int argc, char *argv[])
{
#define MAX_PACKET_DATA_LEN	255
#define MAX_PACKET_BUF_SIZE	(1+1+255+2)

    uint8_t	sspTxBuf[MAX_PACKET_BUF_SIZE];
    uint8_t	sspRxBuf[MAX_PACKET_BUF_SIZE];

    port * info;
    PortSettings settings;
    settings.BaudRate=BAUD57600;
    settings.DataBits=DATA_8;
    settings.FlowControl=FLOW_OFF;
    settings.Parity=PAR_NONE;
    settings.StopBits=STOP_1;
    settings.Timeout_Millisec=5000;

    info=new port(settings,"COM3");
    info->rxBuf 		= sspRxBuf;
    info->rxBufSize 	= MAX_PACKET_DATA_LEN;
    info->txBuf 		= sspTxBuf;
    info->txBufSize 	= 255;
    info->max_retry	= 3;
    info->timeoutLen	= 5000;
    //qssp b(info);
    qsspt bb(info);
    uint8_t buf[1000];
    QCoreApplication a(argc, argv);
    while(!bb.ssp_Synchronise())
    {
        qDebug()<<"trying sync";
    }
     bb.start();
     qDebug()<<"sync complete";
     buf[0]=0;
     buf[1]=1;
     buf[2]=2;
     while(true)
     {
     if(bb.sendData(buf,63))
         qDebug()<<"send OK";
//     else
//         qDebug()<<"send NOK";
//     //bb.ssp_SendData(buf,63);
 }
     while(true)
    {



        if(bb.packets_Available()>0)
        {

            bb.read_Packet(buf);
            qDebug()<<"receive="<<(int)buf[0]<<(int)buf[1]<<(int)buf[2];
            ++buf[0];
            ++buf[1];
            ++buf[2];
            //bb.ssp_SendData(buf,63);
            bb.sendData(buf,63);
        }
        //bb.ssp_ReceiveProcess();
        //bb.ssp_SendProcess();


    }
    return a.exec();
}
