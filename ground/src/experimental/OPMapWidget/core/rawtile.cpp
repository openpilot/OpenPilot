#include "rawtile.h"

RawTile::RawTile(const MapType::Types &Type, const Point &Pos, const int &Zoom)
{
    zoom=Zoom;
    type=Type;
    pos=Pos;
}
QString RawTile::ToString()
{
    return QString("%1 at zoom %2, pos:%3,%4").arg(type).arg(zoom).arg(pos.X()).arg(pos.Y());
}
Point RawTile::Pos()
{
    return pos;
}
MapType::Types RawTile::Type()
{
    return type;
}
int RawTile::Zoom()
{
    return zoom;
}
void RawTile::setType(const MapType::Types &value)
{
    type=value;
}
void RawTile::setPos(const Point &value)
{
    pos=value;
}
void RawTile::setZoom(const int &value)
{
    zoom=value;
}
uint qHash(RawTile const& tile)
{
    // RawTile tile=tilee;
    quint64 tmp=(((quint64)(tile.zoom))<<54)+(((int)(tile.type))<<36)+(((quint64)(tile.pos.X()))<<18)+(((quint64)(tile.pos.Y())));
  //  quint64 tmp5=tmp+tmp2+tmp3+tmp4;
    return ::qHash(tmp);
}
bool operator==(RawTile const &lhs,RawTile const &rhs)
{
    return (lhs.pos==rhs.pos && lhs.zoom==rhs.zoom && lhs.type==rhs.type);
}
