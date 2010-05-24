#include "memorycache.h"
//TODO add readwrite lock
MemoryCache::MemoryCache()
{

}


QByteArray MemoryCache::GetTileFromMemoryCache(const RawTile &tile)
{
    kiberCacheLock.lockForRead();
    QByteArray pic;
    pic=TilesInMemory.cachequeue.value(tile);
   // TilesInMemory.find(key,&pic);
    kiberCacheLock.unlock();
    return pic;
}
void MemoryCache::AddTileToMemoryCache(const RawTile &tile, const QByteArray &pic)
{
    kiberCacheLock.lockForWrite();
   // QPixmapCache::Key key=TilesInMemory.insert(pic);
    TilesInMemory.memoryCacheSize +=pic.count();
    TilesInMemory.cachequeue.insert(tile,pic);
    TilesInMemory.list.enqueue(tile);

    kiberCacheLock.unlock();
}

