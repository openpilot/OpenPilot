#ifndef RAWTILE_H
#define RAWTILE_H

#include "maptype.h"
#include "point.h"
#include <QString>
#include <QHash>
class RawTile
{
    friend uint qHash(RawTile const& tile);
    friend bool operator==(RawTile const& lhs,RawTile const& rhs);

public:
    RawTile(const MapType::Types &Type,const Point &Pos,const int &Zoom);
    QString ToString(void);
    MapType::Types Type();
    Point Pos();
    int Zoom();
    void setType(const MapType::Types &value);
    void setPos(const Point &value);
    void setZoom(const int &value);
private:
    MapType::Types type;
    Point pos;
    int zoom;
};
//uint qHash(RawTile const& tile);
//bool operator==(RawTile const& lhs,RawTile const& rhs);

#endif // RAWTILE_H
