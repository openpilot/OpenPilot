/**
******************************************************************************
*
* @file       OPMaps.cpp
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
*             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
* @brief      
* @see        The GNU Public License (GPL) Version 3
* @defgroup   OPMapWidget
* @{
* 
*****************************************************************************/
/* 
* This program is free software; you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by 
* the Free Software Foundation; either version 3 of the License, or 
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful, but 
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
* for more details.
* 
* You should have received a copy of the GNU General Public License along 
* with this program; if not, write to the Free Software Foundation, Inc., 
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "gmaps.h"

 
namespace core {
OPMaps* OPMaps::m_pInstance=0;

OPMaps* OPMaps::Instance()
{
    if(!m_pInstance)
        m_pInstance=new OPMaps;
    return m_pInstance;
}
OPMaps::OPMaps():useMemoryCache(true),MaxZoom(19),RetryLoadTile(2)
{
    accessmode=AccessMode::ServerAndCache;
    Language=LanguageType::PortuguesePortugal;
    LanguageStr=LanguageType().toString(Language);
 //   Cache::Instance()->ImageCache=PureImageCache();

}


OPMaps::~OPMaps()
{
    //delete Proxy;
}



QByteArray OPMaps::GetImageFrom(const MapType::Types &type,const Point &pos,const int &zoom)
{
#ifdef DEBUG_GMAPS
    qDebug()<<"Entered GetImageFrom";
#endif //DEBUG_GMAPS
    QByteArray ret;

    if(useMemoryCache)
    {
#ifdef DEBUG_GMAPS
        qDebug()<<"Try Tile from memory:Size="<<TilesInMemory.MemoryCacheSize();
#endif //DEBUG_GMAPS
        ret=GetTileFromMemoryCache(RawTile(type,pos,zoom));

    }
    if(ret.isEmpty())
    {
#ifdef DEBUG_GMAPS
        qDebug()<<"Tile not in memory";
#endif //DEBUG_GMAPS
        if(accessmode != (AccessMode::ServerOnly))
        {
#ifdef DEBUG_GMAPS
            qDebug()<<"Try tile from DataBase";
#endif //DEBUG_GMAPS
            ret=Cache::Instance()->ImageCache.GetImageFromCache(type,pos,zoom);
            if(!ret.isEmpty())
            {
#ifdef DEBUG_GMAPS
                qDebug()<<"Tile found in Database";
#endif //DEBUG_GMAPS
                if(useMemoryCache)
                {
#ifdef DEBUG_GMAPS
                    qDebug()<<"Add Tile to memory";
#endif //DEBUG_GMAPS
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
#ifdef DEBUG_GMAPS
            qDebug()<<"Try Tile from the Internet";
#endif //DEBUG_GMAPS
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
#ifdef DEBUG_GMAPS
            qDebug()<<"Starting get response ";//<<pos.X()+","+pos.Y();
#endif //DEBUG_GMAPS
            QTime time;
            time.start();
            while( !(reply->isFinished() | time.elapsed()>(6*Timeout)) ){QCoreApplication::processEvents(QEventLoop::AllEvents);}
#ifdef DEBUG_GMAPS
            qDebug()<<"Finished?"<<reply->error()<<" abort?"<<(time.elapsed()>Timeout*6);
#endif //DEBUG_GMAPS
            if( (reply->error()!=QNetworkReply::NoError) | (time.elapsed()>Timeout*6))
            {
#ifdef DEBUG_GMAPS
                qDebug()<<"Request timed out ";//<<pos.x+","+pos.y;
#endif //DEBUG_GMAPS
                return ret;
            }
#ifdef DEBUG_GMAPS
            qDebug()<<"Response OK ";//<<pos.x+","+pos.y;
#endif //DEBUG_GMAPS
            ret=reply->readAll();
            reply->deleteLater();//TODO can't this be global??
            if(ret.isEmpty())
            {
#ifdef DEBUG_GMAPS
                qDebug()<<"Invalid Tile";
#endif //DEBUG_GMAPS
                return ret;
            }
#ifdef DEBUG_GMAPS
            qDebug()<<"Received Tile from the Internet";
#endif //DEBUG_GMAPS
            if (useMemoryCache)
            {
#ifdef DEBUG_GMAPS
                qDebug()<<"Add Tile to memory cache";
#endif //DEBUG_GMAPS
                AddTileToMemoryCache(RawTile(type,pos,zoom),ret);
            }
            if(accessmode!=AccessMode::ServerOnly)
            {
#ifdef DEBUG_GMAPS
                qDebug()<<"Add tile to DataBase";
#endif //DEBUG_GMAPS
                CacheItemQueue item(type,pos,ret,zoom);
                TileDBcacheQueue.EnqueueCacheTask(item);
            }


        }
    }
#ifdef DEBUG_GMAPS
    qDebug()<<"Entered GetImageFrom";
#endif //DEBUG_GMAPS
    return ret;
}

bool OPMaps::ExportToGMDB(const QString &file)
{
    return Cache::Instance()->ImageCache.ExportMapDataToDB(Cache::Instance()->ImageCache.GtileCache()+QDir::separator()+"Data.qmdb",file);
}
bool OPMaps::ImportFromGMDB(const QString &file)
{
    return Cache::Instance()->ImageCache.ExportMapDataToDB(file,Cache::Instance()->ImageCache.GtileCache()+QDir::separator()+"Data.qmdb");
}
}
