#ifndef MERCATORPROJECTIONYANDEX_H
#define MERCATORPROJECTIONYANDEX_H

#include "../internals/pureprojection.h"

class MercatorProjectionYandex:public PureProjection
{
public:
    MercatorProjectionYandex();
    virtual QString Type(){return "MercatorProjectionYandex";}
    virtual Size TileSize() const;
    virtual double Axis() const;
    virtual double Flattening()const;
    virtual Point FromLatLngToPixel(double lat, double lng, int const& zoom);
    virtual PointLatLng FromPixelToLatLng(const int &x,const int &y,const int &zoom);
    virtual  Size GetTileMatrixMinXY(const int &zoom);
    virtual  Size GetTileMatrixMaxXY(const int &zoom);
private:
    const double MinLatitude;
    const double MaxLatitude;
    const double MinLongitude;
    const double MaxLongitude;
    const double RAD_DEG;
    const double DEG_RAD;
    const double MathPiDiv4;
    double Clip(double const& n, double const& minValue, double const& maxValue)const;
    Size tileSize;
};

#endif // MERCATORPROJECTIONYANDEX_H
