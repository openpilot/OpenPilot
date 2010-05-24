#ifndef KIBERTILECACHE_H
#define KIBERTILECACHE_H

#include <QPixmapCache>
#include "rawtile.h"
#include <QMutex>
#include <QReadWriteLock>
#include <QQueue>

class KiberTileCache
{
public:
    KiberTileCache();

    void setMemoryCacheCapacity(const int &value);
    int MemoryCacheCapacity();
    double MemoryCacheSize(){return memoryCacheSize/1048576.0;}
    void RemoveMemoryOverload();
    QReadWriteLock kiberCacheLock;
    QHash <RawTile,QByteArray> cachequeue;
    QQueue <RawTile> list;
    long memoryCacheSize;
private:
    int _MemoryCacheCapacity;

  //  QPixmapCache TilesInMemory;


};



#endif // KIBERTILECACHE_H
