#include <QtCore/QCoreApplication>
#include <QThread>
#include <../../plugins/rawhid/pjrc_rawhid.h>
#include "op_dfu.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString filename;
    if(argc < 1)
        filename = QString("C:\\OpenPilot.bin");
    else
        filename = QString(argv[0]);

    OP_DFU dfu;
//    dfu.enterDFU(1);
//    dfu.StartUpload(4,OP_DFU::Descript);
//    QByteArray array;
//    array[0]=11;
//    array[1]=2;
//    array[2]=3;
//    array[3]=4;
//    array[4]=5;
//    array[5]=6;
//    array[6]=7;
//    array[7]=8;
//    dfu.UploadData(8,array);
    //dfu.UploadDescription(1,"jose manuel");
    //QString str=dfu.DownloadDescription(1,12);
   // dfu.JumpToApp();
  dfu.UploadFirmware(filename.toAscii());
    //qDebug()<<"Description="<<str;
    return a.exec();
}
