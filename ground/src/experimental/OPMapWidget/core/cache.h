#ifndef CACHE_H
#define CACHE_H

#include "pureimagecache.h"

class Cache
{
public:
    static Cache* Instance();


    PureImageCache ImageCache;
    QString CacheLocation();
    void setCacheLocation(const QString& value);
    void CacheGeocoder(const QString &urlEnd,const QString &content);
    QString GetGeocoderFromCache(const QString &urlEnd);
    void CachePlacemark(const QString &urlEnd,const QString &content);
    QString GetPlacemarkFromCache(const QString &urlEnd);
    void CacheRoute(const QString &urlEnd,const QString &content);
    QString GetRouteFromCache(const QString &urlEnd);

private:
    Cache();
    Cache(Cache const&){};
    Cache& operator=(Cache const&){};
    static Cache* m_pInstance;
    QString cache;
    QString routeCache;
    QString geoCache;
    QString placemarkCache;
};

#endif // CACHE_H
