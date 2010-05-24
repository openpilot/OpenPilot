#ifndef LKS94PROJECTION_H
#define LKS94PROJECTION_H
#include <QVector>
#include "cmath"
#include "../internals/pureprojection.h"

class LKS94Projection:public PureProjection
{
public:
    LKS94Projection();
    double GetTileMatrixResolution(int const& zoom);
    virtual QString Type(){return "LKS94Projection";}
    virtual Size TileSize() const;
    virtual double Axis() const;
    virtual double Flattening() const;
    virtual Point FromLatLngToPixel(double lat, double lng, int const& zoom);
    virtual PointLatLng FromPixelToLatLng(int const& x, int const&  y, int const&  zoom);
    virtual double GetGroundResolution(int const& zoom, double const& latitude);
    virtual Size GetTileMatrixMinXY(int const& zoom);
    virtual Size GetTileMatrixMaxXY(int const& zoom);

private:
         const double MinLatitude;
         const double MaxLatitude;
         const double MinLongitude;
         const double MaxLongitude;
         const double orignX;
         const double orignY;
         Size tileSize;
         QVector <double> DTM10(const QVector <double>& lonlat);
         QVector <double> MTD10(QVector <double>&  pnt);
         QVector <double> DTM00(QVector <double>& lonlat);
         QVector <double> DTM01(QVector <double>& lonlat);
         QVector <double> MTD01(QVector <double>& pnt);
         QVector <double> MTD11(QVector <double>& p);
         double Clip(double const& n, double const& minValue, double const& maxValue);
};

#endif // LKS94PROJECTION_H




