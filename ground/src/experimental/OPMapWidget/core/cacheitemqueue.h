#ifndef CACHEITEMQUEUE_H
#define CACHEITEMQUEUE_H

#include "maptype.h"
#include "point.h"
#include <QByteArray>


class CacheItemQueue
{
public:
    CacheItemQueue(const MapType::Types &Type,const Point &Pos,const QByteArray &Img,const int &Zoom);
    CacheItemQueue(){};
    CacheItemQueue(const CacheItemQueue &cSource)
    {
        img=cSource.img;
        pos=cSource.pos;
        type=cSource.type;
        zoom=cSource.zoom;
    }
    CacheItemQueue& operator= (const CacheItemQueue &cSource);
    bool operator== (const CacheItemQueue &cSource);
    void SetMapType(const MapType::Types &value);
    void SetPosition(const Point &value);
    void SetImg(const QByteArray &value);
    MapType::Types GetMapType();
    Point GetPosition();
    QByteArray GetImg();
    int GetZoom(){return zoom;};
    void SetZoom(const int &value) {zoom=value;};
private:


    MapType::Types type;
    Point pos;
    QByteArray img;
    int zoom;
};

#endif // CACHEITEMQUEUE_H
