#include "tilematrix.h"

TileMatrix::TileMatrix()
{
}
void TileMatrix::Clear()
{
    mutex.lock();
    foreach(Tile t,matrix.values())
    {
        t.Clear();
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
        Tile t=TileAt(p);
        if(t.GetZoom()!=0)
        {
            mutex.lock();
            t.Clear();
            t.SetZoom(NULL);
            matrix.remove(p);
            mutex.unlock();
        }

    }
    removals.clear();
}
Tile TileMatrix::TileAt(const Point &p)
{
    Tile ret;
    mutex.lock();
    ret=matrix.value(p);
    mutex.unlock();
    return ret;
}
void TileMatrix::SetTileAt(const Point &p, const Tile &tile)
{
    mutex.lock();
    matrix.insert(p,tile);
    mutex.unlock();
}
