#ifndef QYMODEMSEND_H
#define QYMODEMSEND_H
#include "qymodem_TX.h"
#include <QString>
#include <QFile>


class QymodemSend:public QymodemTx
{

public:
    QymodemSend(QextSerialPort& port);
    int SendFile(QString filename);
    int SendFileT(QString filename);
    int PercentSend();

private:
    void run();
    const char* FileName;
    void Send();
    class InFile;
    QString FileNameT;

};
#endif // QYmodemSend_H
