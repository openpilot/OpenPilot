#ifndef GMAPS_H
#define GMAPS_H

#include "memorycache.h"
#include "rawtile.h"
#include "cache.h"
#include "accessmode.h"
#include "languagetype.h"
#include "cacheitemqueue.h"
#include "tilecachequeue.h"
#include "pureimagecache.h"
#include "alllayersoftype.h"
#include "urlfactory.h"
//#include "point.h"

class GMaps: public MemoryCache,public AllLayersOfType,public UrlFactory
{


public:

    ~GMaps();

    static GMaps* Instance();
    bool ImportFromGMDB(const QString &file);
    bool ExportToGMDB(const QString &file);
    /// <summary>
    /// timeout for map connections
    /// </summary>


    QByteArray GetImageFrom(const MapType::Types &type,const Point &pos,const int &zoom);
    bool UseMemoryCache();//TODO
    void setUseMemoryCache(const bool& value);//TODO
    void setLanguage(const LanguageType::Types& language);//TODO
    LanguageType::Types GetLanguage();//TODO
    AccessMode GetAccessMode();
    void setAccessMode(const AccessMode& mode);
    int MaxZoom;
    int RetryLoadTile;
private:




    bool useMemoryCache;
    LanguageType::Types Language;
    AccessMode::Types accessmode;

    //  PureImageCache ImageCacheLocal;//TODO Criar acesso Get Set
    TileCacheQueue TileDBcacheQueue;
    GMaps();

    GMaps(GMaps const&){};
    GMaps& operator=(GMaps const&){};
    static GMaps* m_pInstance;


protected:
   // MemoryCache TilesInMemory;



};

#endif // GMAPS_H
