#include "platecarreeprojectionpergo.h"

PlateCarreeProjectionPergo::PlateCarreeProjectionPergo():MinLatitude(-85.05112878), MaxLatitude(85.05112878),MinLongitude(-180),
MaxLongitude(180), tileSize(256, 256)
{
}
Point PlateCarreeProjectionPergo::FromLatLngToPixel(double lat, double lng, const int &zoom)
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
PointLatLng PlateCarreeProjectionPergo::FromPixelToLatLng(const int &x, const int &y, const int &zoom)
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

double PlateCarreeProjectionPergo::Clip(const double &n, const double &minValue, const double &maxValue) const
{
    return qMin(qMax(n, minValue), maxValue);
}
Size PlateCarreeProjectionPergo::TileSize() const
{
    return tileSize;
}
double PlateCarreeProjectionPergo::Axis() const
{
    return 6378137;
}
double PlateCarreeProjectionPergo::Flattening() const
{
    return (1.0 / 298.257223563);
}
Size PlateCarreeProjectionPergo::GetTileMatrixMaxXY(const int &zoom)
{
    int y = (int) pow(2, zoom);
    return Size((2*y) - 1, y - 1);
}

Size PlateCarreeProjectionPergo::GetTileMatrixMinXY(const int &zoom)
{
    return Size(0, 0);
}
