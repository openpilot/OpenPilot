#include "cache.h"

Cache* Cache::m_pInstance=0;

Cache* Cache::Instance()
{
    if(!m_pInstance)
        m_pInstance=new Cache;
    return m_pInstance;
}

void Cache::setCacheLocation(const QString& value)
{
    cache=value;
    routeCache = cache + "RouteCache" + QDir::separator();
    geoCache = cache + "GeocoderCache"+ QDir::separator();
    placemarkCache = cache + "PlacemarkCache" + QDir::separator();
    ImageCache.setGtileCache(value);
}
QString Cache::CacheLocation()
{
    return cache;
}
Cache::Cache()
{
    if(cache.isNull()|cache.isEmpty())
    {
        cache=QDir::currentPath()+QDir::separator()+"mapscache"+QDir::separator();
        setCacheLocation(cache);
    }
}
QString Cache::GetGeocoderFromCache(const QString &urlEnd)
{
#ifdef DEBUG_GetGeocoderFromCache
    qDebug()<<"Entered GetGeocoderFromCache";
#endif
    QString ret=QString::null;
    QString filename=geoCache+QString(urlEnd)+".geo";
#ifdef DEBUG_GetGeocoderFromCache
    qDebug()<<"GetGeocoderFromCache: Does file exist?:"<<filename;
#endif
    QFileInfo File(filename);
    if (File .exists())
    {
#ifdef DEBUG_GetGeocoderFromCache
        qDebug()<<"GetGeocoderFromCache:File exists!!";
#endif
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            stream>>ret;
        }
    }
#ifdef DEBUG_GetGeocoderFromCache
    qDebug()<<"GetGeocoderFromCache:Returning:"<<ret;
#endif
    return ret;
}
void Cache::CacheGeocoder(const QString &urlEnd, const QString &content)
{
    QString ret=QString::null;
    QString filename=geoCache+QString(urlEnd)+".geo";
    qDebug()<<"CacheGeocoder: Filename:"<<filename;
    QFileInfo File(filename);;
    QDir dir=File.absoluteDir();
    QString path=dir.absolutePath();
    qDebug()<<"CacheGeocoder: Path:"<<path;
    if(!dir.exists())
    {
        qDebug()<<"CacheGeocoder: Cache path doesn't exist, try to create";
        if(!dir.mkpath(path))
        {
            qDebug()<<"GetGeocoderFromCache: Could not create path";
        }
    }
    qDebug()<<"CacheGeocoder: OpenFile:"<<filename;
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
    {
        qDebug()<<"CacheGeocoder: File Opened!!!:"<<filename;
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream<<content;
    }
}
QString Cache::GetPlacemarkFromCache(const QString &urlEnd)
{
    qDebug()<<"Entered GetPlacemarkFromCache";
    QString ret=QString::null;
    QString filename=placemarkCache+QString(urlEnd)+".plc";
    qDebug()<<"GetPlacemarkFromCache: Does file exist?:"<<filename;
    QFileInfo File(filename);
    if (File .exists())
    {
        qDebug()<<"GetPlacemarkFromCache:File exists!!";
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            stream>>ret;
        }
    }
    qDebug()<<"GetPlacemarkFromCache:Returning:"<<ret;
    return ret;
}
void Cache::CachePlacemark(const QString &urlEnd, const QString &content)
{
    QString ret=QString::null;
    QString filename=placemarkCache+QString(urlEnd)+".plc";
    qDebug()<<"CachePlacemark: Filename:"<<filename;
    QFileInfo File(filename);;
    QDir dir=File.absoluteDir();
    QString path=dir.absolutePath();
    qDebug()<<"CachePlacemark: Path:"<<path;
    if(!dir.exists())
    {
        qDebug()<<"CachePlacemark: Cache path doesn't exist, try to create";
        if(!dir.mkpath(path))
        {
            qDebug()<<"CachePlacemark: Could not create path";
        }
    }
    qDebug()<<"CachePlacemark: OpenFile:"<<filename;
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
    {
        qDebug()<<"CachePlacemark: File Opened!!!:"<<filename;
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream<<content;
    }
}
