#ifndef TILECACHEQUEUE_H
#define TILECACHEQUEUE_H

#include <QQueue>
#include "cacheitemqueue.h"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QObject>
#include <QMutexLocker>
#include "pureimagecache.h"
#include "cache.h"

class TileCacheQueue:public QThread
{
    Q_OBJECT
public:
    TileCacheQueue();

    void EnqueueCacheTask(CacheItemQueue &task);

protected:
    QQueue<CacheItemQueue> tileCacheQueue;
private:
    void run();
    QMutex mutex;
    QMutex waitmutex;
    QWaitCondition wait;
};






#endif // TILECACHEQUEUE_H
