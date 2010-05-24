#ifndef MEMORYCACHE_H
#define MEMORYCACHE_H

#include <QPixmapCache>
#include "rawtile.h"
#include <QMutex>
#include <QReadWriteLock>
#include <QQueue>
#include "kibertilecache.h"
class MemoryCache
{
public:
    MemoryCache();

    KiberTileCache TilesInMemory;
    QByteArray GetTileFromMemoryCache(const RawTile &tile);
    void AddTileToMemoryCache(const RawTile &tile, const QByteArray &pic);
    QReadWriteLock kiberCacheLock;

};


#endif // MEMORYCACHE_H
