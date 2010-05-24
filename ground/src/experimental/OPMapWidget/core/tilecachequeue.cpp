#include "tilecachequeue.h"



TileCacheQueue::TileCacheQueue()
{

}

void TileCacheQueue::EnqueueCacheTask(CacheItemQueue &task)
{
    qDebug()<<"DB Do I EnqueueCacheTask"<<task.GetPosition().X()<<","<<task.GetPosition().Y();
    if(!tileCacheQueue.contains(task))
    {
        qDebug()<<"EnqueueCacheTask"<<task.GetPosition().X()<<","<<task.GetPosition().Y();
        mutex.lock();
        tileCacheQueue.enqueue(task);
        mutex.unlock();
        if(this->isRunning())
        {
            qDebug()<<"Wake Thread";
            wait.wakeOne();
        }
        else
        {
            qDebug()<<"Start Thread";
            this->start(QThread::LowestPriority);
        }
    }

}
void TileCacheQueue::run()
{
    qDebug()<<"Cache Engine Start";
    while(true)
    {
        CacheItemQueue task;
        qDebug()<<"Cache";
        if(tileCacheQueue.count()>0)
        {
            mutex.lock();
            task=tileCacheQueue.dequeue();
            mutex.unlock();
            qDebug()<<"Cache engine Put:"<<task.GetPosition().X()<<","<<task.GetPosition().Y();
            Cache::Instance()->ImageCache.PutImageToCache(task.GetImg(),task.GetMapType(),task.GetPosition(),task.GetZoom());
            QThread::usleep(44);            
        }

        else
        {
            waitmutex.lock();
            if(!wait.wait(&waitmutex,4444))

            {
                qDebug()<<"Cache Engine TimeOut";
                if(tileCacheQueue.count()==0) break;
            }
            waitmutex.unlock();
        }
    }
    qDebug()<<"Cache Engine Stopped";
}


