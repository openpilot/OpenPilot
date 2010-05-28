#ifndef TILE_H
#define TILE_H

#include "QList"
#include <QImage>
#include "../core/point.h"
#include <QMutex>
#include <QDebug>
class Tile
{
public:
    Tile(int zoom,Point pos);
    Tile();
    void Clear();
    int GetZoom(){return zoom;}
    Point GetPos(){return pos;}
    void SetZoom(const int &value){zoom=value;}
    void SetPos(const Point &value){pos=value;}
    Tile& operator= (const Tile &cSource);
    Tile(const Tile &cSource)
    {
        this->zoom=cSource.zoom;
        this->pos=cSource.pos;
    }
    bool HasValue(){return !(zoom==0);}
    QList<QByteArray> Overlays;
protected:

    QMutex mutex;
private:
    int zoom;
    Point pos;


};

#endif // TILE_H
