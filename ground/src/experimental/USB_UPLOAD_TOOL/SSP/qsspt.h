#ifndef QSSPT_H
#define QSSPT_H

#include "qssp.h"
#include <QThread>
#include <QQueue>

class qsspt:private qssp, public QThread
{
public:
    qsspt(port * info);
    void run();
    int packets_Available();
    QByteArray read_Packet();
    ~qsspt();
private:
    virtual void pfCallBack( uint8_t *, uint16_t);
    QQueue<QByteArray> queue;
    QMutex mutex;
    bool endthread;
};

#endif // QSSPT_H
