#include "tile.h"

Tile::Tile(int zoom, Point pos)
{
    this->zoom=zoom;
    this->pos=pos;
}
void Tile::Clear()
{
    qDebug()<<"Tile:Clear Overlays";
    mutex.lock();
    foreach(QByteArray img, Overlays)
    {
        img.~QByteArray();
    }
    Overlays.clear();
    mutex.unlock();
}
Tile::Tile():zoom(0),pos(0,0)
{

}
Tile& Tile::operator =(const Tile &cSource)
{
    this->zoom=cSource.zoom;
    this->pos=cSource.pos;
    return *this;
}

