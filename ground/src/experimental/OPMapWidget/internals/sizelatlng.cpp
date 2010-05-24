#include "sizelatlng.h"
#include "pointlatlng.h"
SizeLatLng::SizeLatLng():heightLat(0),widthLng(0)
{

}
SizeLatLng::SizeLatLng(PointLatLng const&  pt)
{
  this->heightLat = pt.Lat();
  this->widthLng = pt.Lng();
}
SizeLatLng operator+(SizeLatLng const&  sz1, SizeLatLng const&  sz2)
{
    return SizeLatLng::Add(sz1, sz2);
}

SizeLatLng operator-(SizeLatLng const&  sz1, SizeLatLng const&  sz2)
{
  return SizeLatLng::Subtract(sz1, sz2);
}

bool operator==(SizeLatLng const&  sz1, SizeLatLng const&  sz2)
{
  return ((sz1.WidthLng() == sz2.WidthLng()) && (sz1.HeightLat() == sz2.HeightLat()));
}

bool operator!=(SizeLatLng const&  sz1, SizeLatLng const&  sz2)
{
  return !(sz1 == sz2);
}
SizeLatLng SizeLatLng::Empty=SizeLatLng();
