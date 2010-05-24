#include "gmaps.h"

GMaps* GMaps::m_pInstance=0;

GMaps* GMaps::Instance()
{
    if(!m_pInstance)
        m_pInstance=new GMaps;
    return m_pInstance;
}
GMaps::GMaps():useMemoryCache(true),MaxZoom(19),RetryLoadTile(2)
{
    accessmode=AccessMode::ServerOnly;
    Language=LanguageType::PortuguesePortugal;
    LanguageStr=LanguageType().toString(Language);
    Cache::Instance()->ImageCache=PureImageCache();

}


GMaps::~GMaps()
{
    //delete Proxy;
}



QByteArray GMaps::GetImageFrom(const MapType::Types &type,const Point &pos,const int &zoom)
{
    qDebug()<<"Entered GetImageFrom";
    QByteArray ret;

    if(useMemoryCache)
    {
        qDebug()<<"Try Tile from memory:Size="<<TilesInMemory.MemoryCacheSize();
        ret=GetTileFromMemoryCache(RawTile(type,pos,zoom));

    }
    if(ret.isEmpty())
    {
        qDebug()<<"Tile not in memory";
        if(accessmode != (AccessMode::ServerOnly))
        {
            qDebug()<<"Try tile from DataBase";
            ret=Cache::Instance()->ImageCache.GetImageFromCache(type,pos,zoom);
            if(!ret.isEmpty())
            {
                qDebug()<<"Tile found in Database";
                if(useMemoryCache)
                {
                    qDebug()<<"Add Tile to memory";
                    AddTileToMemoryCache(RawTile(type,pos,zoom),ret);
                }
                return ret;
            }
        }
        if(accessmode!=AccessMode::CacheOnly)
        {
            QNetworkReply *reply;
            QNetworkRequest qheader;
            QNetworkAccessManager network;
            network.setProxy(Proxy);
            qDebug()<<"Try Tile from the Internet";
            QString url=MakeImageUrl(type,pos,zoom,LanguageStr);
            qheader.setUrl(QUrl(url));
            qheader.setRawHeader("User-Agent",UserAgent);
            qheader.setRawHeader("Accept","*/*");
            switch(type)
            {
            case MapType::GoogleMap:
            case MapType::GoogleSatellite:
            case MapType::GoogleLabels:
            case MapType::GoogleTerrain:
            case MapType::GoogleHybrid:
                {
                    qheader.setRawHeader("Referrer", "http://maps.google.com/");
                }
                break;

            case MapType::GoogleMapChina:
            case MapType::GoogleSatelliteChina:
            case MapType::GoogleLabelsChina:
            case MapType::GoogleTerrainChina:
            case MapType::GoogleHybridChina:
                {
                    qheader.setRawHeader("Referrer", "http://ditu.google.cn/");
                }
                break;

            case MapType::BingHybrid:
            case MapType::BingMap:
            case MapType::BingSatellite:
                {
                    qheader.setRawHeader("Referrer", "http://www.bing.com/maps/");
                }
                break;

            case MapType::YahooHybrid:
            case MapType::YahooLabels:
            case MapType::YahooMap:
            case MapType::YahooSatellite:
                {
                    qheader.setRawHeader("Referrer", "http://maps.yahoo.com/");
                }
                break;

            case MapType::ArcGIS_MapsLT_Map_Labels:
            case MapType::ArcGIS_MapsLT_Map:
            case MapType::ArcGIS_MapsLT_OrtoFoto:
            case MapType::ArcGIS_MapsLT_Map_Hybrid:
                {
                    qheader.setRawHeader("Referrer", "http://www.maps.lt/map_beta/");
                }
                break;

            case MapType::OpenStreetMapSurfer:
            case MapType::OpenStreetMapSurferTerrain:
                {
                    qheader.setRawHeader("Referrer", "http://www.mapsurfer.net/");
                }
                break;

            case MapType::OpenStreetMap:
            case MapType::OpenStreetOsm:
                {
                    qheader.setRawHeader("Referrer", "http://www.openstreetmap.org/");
                }
                break;

            case MapType::YandexMapRu:
                {
                    qheader.setRawHeader("Referrer", "http://maps.yandex.ru/");
                }
                break;
            default:
                break;
            }
            reply=network.get(qheader);
            qDebug()<<"Starting get response ";//<<pos.X()+","+pos.Y();
            QTime time;
            time.start();
            while( !(reply->isFinished() | time.elapsed()>(6*Timeout)) ){QCoreApplication::processEvents(QEventLoop::AllEvents);}
            qDebug()<<"Finished?"<<reply->error()<<" abort?"<<(time.elapsed()>Timeout*6);
            if( (reply->error()!=QNetworkReply::NoError) | (time.elapsed()>Timeout*6))
            {
                qDebug()<<"Request timed out ";//<<pos.x+","+pos.y;
                return ret;
            }
            qDebug()<<"Response OK ";//<<pos.x+","+pos.y;
            ret=reply->readAll();
            reply->deleteLater();//TODO can't this be global??
            if(ret.isEmpty())
            {
                qDebug()<<"Invalid Tile";
                return ret;
            }
            qDebug()<<"Received Tile from the Internet";
            if (useMemoryCache)
            {
                qDebug()<<"Add Tile to memory cache";
                AddTileToMemoryCache(RawTile(type,pos,zoom),ret);
            }
            if(accessmode!=AccessMode::ServerOnly)
            {
                qDebug()<<"Add tile to DataBase";
                CacheItemQueue item(type,pos,ret,zoom);
                TileDBcacheQueue.EnqueueCacheTask(item);
            }


        }
    }
    qDebug()<<"Entered GetImageFrom";
    return ret;
}

bool GMaps::ExportToGMDB(const QString &file)
{
    return Cache::Instance()->ImageCache.ExportMapDataToDB(Cache::Instance()->ImageCache.GtileCache()+QDir::separator()+"Data.qmdb",file);
}
bool GMaps::ImportFromGMDB(const QString &file)
{
    return Cache::Instance()->ImageCache.ExportMapDataToDB(file,Cache::Instance()->ImageCache.GtileCache()+QDir::separator()+"Data.qmdb");
}
