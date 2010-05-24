#include "kibertilecache.h"

//TODO add readwrite lock
KiberTileCache::KiberTileCache()
{
    memoryCacheSize = 0;
    _MemoryCacheCapacity = 22;
}

void KiberTileCache::setMemoryCacheCapacity(const int &value)
{
    kiberCacheLock.lockForWrite();
    _MemoryCacheCapacity=value;
    kiberCacheLock.unlock();
}
int KiberTileCache::MemoryCacheCapacity()
{
    kiberCacheLock.lockForRead();
    return _MemoryCacheCapacity;
    kiberCacheLock.unlock();
}

void KiberTileCache::RemoveMemoryOverload()
{
    while(memoryCacheSize>MemoryCacheCapacity())
    {
        if(cachequeue.count()>0 && list.count()>0)
        {
            RawTile first=list.dequeue();
            memoryCacheSize-=cachequeue.value(first).length();
            cachequeue.remove(first);
        }
    }
}
