#include "pointlatlng.h"

PointLatLng PointLatLng::Empty=PointLatLng();
PointLatLng::PointLatLng():lat(0),lng(0)
{

}

bool operator==(PointLatLng const& lhs,PointLatLng const& rhs)
{
   return ((lhs.Lng() == rhs.Lng()) && (lhs.Lat() == rhs.Lat()));
}

bool operator!=(PointLatLng const& left, PointLatLng const& right)
{
   return !(left == right);
}
PointLatLng operator+(PointLatLng pt, SizeLatLng sz)
{
   return PointLatLng::Add(pt, sz);
}

PointLatLng operator-(PointLatLng pt, SizeLatLng sz)
{
   return PointLatLng::Subtract(pt, sz);
}
