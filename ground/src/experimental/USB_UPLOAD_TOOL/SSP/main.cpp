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
    settings.BaudRate=BAUD19200;
    settings.DataBits=DATA_8;
    settings.FlowControl=FLOW_OFF;
    settings.Parity=PAR_NONE;
    settings.StopBits=STOP_2;
    settings.Timeout_Millisec=500;

    info=new port(settings,"COM2");
    info->rxBuf 		= sspRxBuf;
    info->rxBufSize 	= MAX_PACKET_DATA_LEN;
    info->txBuf 		= sspTxBuf;
    info->txBufSize 	= 255;
    info->max_retry	= 3;
    info->timeoutLen	= 100;
    //qssp b(info);
    qsspt bb(info);
    QCoreApplication a(argc, argv);
    QByteArray arr;
    while(true)
    {
        if(bb.packets_Available()>0)
        {
            arr=bb.read_Packet();
            qDebug()<<"receive="<<(int)arr[0]<<(int)arr[1]<<(int)arr[2];
        }

      //  b.ssp_ReceiveProcess();
      //  b.ssp_SendProcess();
//        if(b.isDataAvailable())
//        {
//            QByteArray bb=b.readData();
//            qDebug()<<bb[0]<<bb[1]<<bb[2];
//        }
    }
    return a.exec();
}
