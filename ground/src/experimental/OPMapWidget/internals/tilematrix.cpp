#include "tilematrix.h"

TileMatrix::TileMatrix()
{
}
void TileMatrix::Clear()
{
    mutex.lock();
    foreach(Tile* t,matrix.values())
    {
        delete t;
        t=0;
    }
    matrix.clear();
    mutex.unlock();
}

void TileMatrix::ClearPointsNotIn(QList<Point>list)
{
    removals.clear();
    mutex.lock();
    foreach(Point p, matrix.keys())
    {
        if(!list.contains(p))
        {
            removals.append(p);
        }
    }
    mutex.unlock();
    foreach(Point p,removals)
    {
        Tile* t=TileAt(p);
        if(t!=0)
        {
            mutex.lock();
            delete t;
            t=0;
            matrix.remove(p);
            mutex.unlock();
        }

    }
    removals.clear();
}
Tile* TileMatrix::TileAt(const Point &p)
{
    qDebug()<<"TileMatrix:TileAt:"<<p.ToString();
    Tile* ret;
    mutex.lock();
    ret=matrix.value(p,0);
    mutex.unlock();
    return ret;
}
void TileMatrix::SetTileAt(const Point &p, Tile* tile)
{
    mutex.lock();
    matrix.insert(p,tile);
    mutex.unlock();
}
