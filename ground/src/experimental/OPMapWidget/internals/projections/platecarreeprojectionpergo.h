#ifndef PLATECARREEPROJECTIONPERGO_H
#define PLATECARREEPROJECTIONPERGO_H

#include "../internals/pureprojection.h"

class PlateCarreeProjectionPergo:public PureProjection
{
public:
    PlateCarreeProjectionPergo();
    virtual QString Type(){return "PlateCarreeProjectionPergo";}
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
#endif // PLATECARREEPROJECTIONPERGO_H
