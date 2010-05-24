#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "QThreadPool"
#include <QDebug>
#include "gmaps.h"
#include <QList>

class threadpool: public QRunnable
{

public:
    threadpool();
    QList <QByteArray> pix;
    QByteArray GetPic(int i);
     int count;
     int count2;
 QMutex m;
private:
    void run();

QMutex mm;

    QPixmap temp;

};

#endif // THREADPOOL_H
