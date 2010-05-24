#include "mercatorprojection.h"

MercatorProjection::MercatorProjection():MinLatitude(-85.05112878), MaxLatitude(85.05112878),MinLongitude(-177),
MaxLongitude(177), tileSize(256, 256)
{
}
Point MercatorProjection::FromLatLngToPixel(double lat, double lng, const int &zoom)
{
    Point ret;// = Point.Empty;

    lat = Clip(lat, MinLatitude, MaxLatitude);
    lng = Clip(lng, MinLongitude, MaxLongitude);

    double x = (lng + 180) / 360;
    double sinLatitude = sin(lat * M_PI / 180);
    double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * M_PI);

    Size s = GetTileMatrixSizePixel(zoom);
    int mapSizeX = s.Width();
    int mapSizeY = s.Height();

    ret.SetX((int) Clip(x * mapSizeX + 0.5, 0, mapSizeX - 1));
    ret.SetY((int) Clip(y * mapSizeY + 0.5, 0, mapSizeY - 1));

    return ret;
}
PointLatLng MercatorProjection::FromPixelToLatLng(const int &x, const int &y, const int &zoom)
{
    PointLatLng ret;// = PointLatLng.Empty;

    Size s = GetTileMatrixSizePixel(zoom);
    double mapSizeX = s.Width();
    double mapSizeY = s.Height();

    double xx = (Clip(x, 0, mapSizeX - 1) / mapSizeX) - 0.5;
    double yy = 0.5 - (Clip(y, 0, mapSizeY - 1) / mapSizeY);

    ret.SetLat(90 - 360 * atan(exp(-yy * 2 * M_PI)) / M_PI);
    ret.SetLng(360 * xx);

    return ret;
}
double MercatorProjection::Clip(const double &n, const double &minValue, const double &maxValue) const
{
    return qMin(qMax(n, minValue), maxValue);
}
Size MercatorProjection::TileSize() const
{
    return tileSize;
}
double MercatorProjection::Axis() const
{
    return 6378137;
}
double MercatorProjection::Flattening() const
{
    return (1.0 / 298.257223563);
}
Size MercatorProjection::GetTileMatrixMaxXY(const int &zoom)
{
    int xy = (1 << zoom);
    return  Size(xy - 1, xy - 1);
}
Size MercatorProjection::GetTileMatrixMinXY(const int &zoom)
{
    return Size(0, 0);
}
