#include "platecarreeprojection.h"


PlateCarreeProjection::PlateCarreeProjection():MinLatitude(-85.05112878), MaxLatitude(85.05112878),MinLongitude(-180),
MaxLongitude(180), tileSize(512, 512)
{
}
Point PlateCarreeProjection::FromLatLngToPixel(double lat, double lng, const int &zoom)
{
    Point ret;// = Point.Empty;

    lat = Clip(lat, MinLatitude, MaxLatitude);
    lng = Clip(lng, MinLongitude, MaxLongitude);

    Size s = GetTileMatrixSizePixel(zoom);
    double mapSizeX = s.Width();
    double mapSizeY = s.Height();

    double scale = 360.0 / mapSizeX;

    ret.SetY((int) ((90.0 - lat) / scale));
    ret.SetX((int) ((lng + 180.0) / scale));

    return ret;

}
PointLatLng PlateCarreeProjection::FromPixelToLatLng(const int &x, const int &y, const int &zoom)
{
    PointLatLng ret;// = PointLatLng.Empty;

    Size s = GetTileMatrixSizePixel(zoom);
    double mapSizeX = s.Width();
    double mapSizeY = s.Height();

    double scale = 360.0 / mapSizeX;

    ret.SetLat(90 - (y * scale));
    ret.SetLng((x * scale) - 180);

    return ret;
}
double PlateCarreeProjection::Clip(const double &n, const double &minValue, const double &maxValue) const
{
    return qMin(qMax(n, minValue), maxValue);
}
Size PlateCarreeProjection::TileSize() const
{
    return tileSize;
}
double PlateCarreeProjection::Axis() const
{
    return 6378137;
}
double PlateCarreeProjection::Flattening() const
{
    return (1.0 / 298.257223563);
}
Size PlateCarreeProjection::GetTileMatrixMaxXY(const int &zoom)
{
    int y = (int) pow(2, zoom);
    return Size((2*y) - 1, y - 1);
}

Size PlateCarreeProjection::GetTileMatrixMinXY(const int &zoom)
{
    return Size(0, 0);
}
