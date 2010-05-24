#ifndef MERCATORPROJECTION_H
#define MERCATORPROJECTION_H
#include "../internals/pureprojection.h"
class MercatorProjection:public PureProjection
{
public:
    MercatorProjection();
    virtual QString Type(){return "MercatorProjection";}
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
    double Clip(double const& n, double const& minValue, double const& maxValue)const;
    Size tileSize;
};

#endif // MERCATORPROJECTION_H
