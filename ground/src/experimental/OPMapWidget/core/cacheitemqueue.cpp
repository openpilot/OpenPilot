#include "cacheitemqueue.h"

CacheItemQueue::CacheItemQueue(const MapType::Types &Type, const Point &Pos, const QByteArray &Img, const int &Zoom)
{
    type=Type;
    pos=Pos;
    img=Img;
    zoom=Zoom;

}

QByteArray CacheItemQueue::GetImg()
{
    return img;
}

MapType::Types CacheItemQueue::GetMapType()
{
    return type;
}
Point CacheItemQueue::GetPosition()
{
    return pos;
}
void CacheItemQueue::SetImg(const QByteArray &value)
{
    img=value;
}
void CacheItemQueue::SetMapType(const MapType::Types &value)
{
    type=value;
}
void CacheItemQueue::SetPosition(const Point &value)
{
    pos=value;
}

CacheItemQueue& CacheItemQueue::operator =(const CacheItemQueue &cSource)
                                          {
    img=cSource.img;
    pos=cSource.pos;
    type=cSource.type;
    zoom=cSource.zoom;
}
bool CacheItemQueue::operator ==(const CacheItemQueue &cSource)
{
    bool b=(img==cSource.img)&& (pos==cSource.pos) && (type==cSource.type) && (zoom==cSource.zoom);
    return b;
}
