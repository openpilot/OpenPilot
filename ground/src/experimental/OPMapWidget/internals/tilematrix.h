#ifndef TILEMATRIX_H
#define TILEMATRIX_H

#include <QHash>
#include "tile.h"
#include <QList>
#include "../core/point.h"
class TileMatrix
{
public:
    TileMatrix();
    void Clear();
    void ClearPointsNotIn(QList<Point> list);
    Tile TileAt(const Point &p);
    void SetTileAt(const Point &p,const Tile &tile);
protected:
    QHash<Point,Tile> matrix;
    QList<Point> removals;
    QMutex mutex;
};

#endif // TILEMATRIX_H
