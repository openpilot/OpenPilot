#ifndef URLFACTORY_H
#define URLFACTORY_H

#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QCoreApplication>
#include "providerstrings.h"
#include "pureimagecache.h"
#include "../internals/pointlatlng.h"
#include "geodecoderstatus.h"
#include <QTime>
#include "cache.h"
#include "placemark.h"
#include <QTextCodec>
#include "cmath"
class UrlFactory: public QObject,public ProviderStrings
{
    Q_OBJECT
public:
    /// <summary>
    /// Gets or sets the value of the User-agent HTTP header.
    /// </summary>
    QByteArray UserAgent;
    QNetworkProxy Proxy;
    UrlFactory();
    ~UrlFactory();
    QString MakeImageUrl(const MapType::Types &type,const Point &pos,const int &zoom,const QString &language);
    PointLatLng GetLatLngFromGeodecoder(const QString &keywords,GeoCoderStatusCode::Types &status);
    Placemark GetPlacemarkFromGeocoder(PointLatLng location);
    int Timeout;
private:
    void GetSecGoogleWords(const Point &pos,  QString &sec1, QString &sec2);
    int GetServerNum(const Point &pos,const int &max) const;
    void TryCorrectGoogleVersions();
    bool isCorrectedGoogleVersions;
    QString TileXYToQuadKey(const int &tileX,const int &tileY,const int &levelOfDetail) const;
    bool CorrectGoogleVersions;
    bool UseGeocoderCache; //TODO GetSet
    bool UsePlacemarkCache;//TODO GetSet
    static const double EarthRadiusKm = 6378.137; // WGS-84
    double GetDistance(PointLatLng p1,PointLatLng p2);

protected:
    static short timelapse;


    QString LanguageStr;
    bool IsCorrectGoogleVersions();
    void setIsCorrectGoogleVersions(bool value);
    QString MakeGeocoderUrl(QString keywords);
    QString MakeReverseGeocoderUrl(PointLatLng &pt,const QString &language);
    PointLatLng GetLatLngFromGeocoderUrl(const QString &url,const bool &useCache, GeoCoderStatusCode::Types &status);
    Placemark GetPlacemarkFromReverseGeocoderUrl(const QString &url,const bool &useCache);
};

#endif // URLFACTORY_H
