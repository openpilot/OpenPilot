#include "rectlatlng.h"

RectLatLng::RectLatLng():lng(0),lat(0),widthLng(0),heightLat(0)
{
}
RectLatLng RectLatLng::Empty=RectLatLng();
uint qHash(RectLatLng const& rect)
{
    return (int) (((((uint) rect.Lng()) ^ ((((uint) rect.Lat()) << 13) | (((uint) rect.Lat()) >> 0x13))) ^ ((((uint) rect.WidthLng()) << 0x1a) | (((uint) rect.WidthLng()) >> 6))) ^ ((((uint) rect.HeightLat()) << 7) | (((uint) rect.HeightLat()) >> 0x19)));
}

bool operator==(RectLatLng const& left,RectLatLng const& right)
{
    return ((((left.Lng() == right.Lng()) && (left.Lat() == right.Lat())) && (left.WidthLng() == right.WidthLng())) && (left.HeightLat() == right.HeightLat()));
}

bool operator!=(RectLatLng const& left,RectLatLng const& right)
{
    return !(left == right);
}

