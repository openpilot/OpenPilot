#ifndef QSSPT_H
#define QSSPT_H

#include "qssp.h"
#include <QThread>
#include <QQueue>
#include <QWaitCondition>
class qsspt:public qssp, public QThread
{
public:
    qsspt(port * info,bool debug);
    void run();
    int packets_Available();
    int read_Packet(void *);
    ~qsspt();
    bool sendData(uint8_t * buf,uint16_t size);
private:
    virtual void pfCallBack( uint8_t *, uint16_t);
    uint8_t * mbuf;
    uint16_t msize;
    QQueue<QByteArray> queue;
    QMutex mutex;
    QMutex sendbufmutex;
    bool datapending;
    bool endthread;
    uint16_t sendstatus;
    uint16_t receivestatus;
    QWaitCondition sendwait;
    QMutex msendwait;
    bool debug;
};

#endif // QSSPT_H
