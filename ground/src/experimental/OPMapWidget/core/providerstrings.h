#ifndef PROVIDERSTRINGS_H
#define PROVIDERSTRINGS_H

#include <QString>

class ProviderStrings
{
public:
    ProviderStrings();
    static const QString levelsForSigPacSpainMap[];
    QString GoogleMapsAPIKey;
    // Google version strings
    QString VersionGoogleMap;
    QString VersionGoogleSatellite;
    QString VersionGoogleLabels;
    QString VersionGoogleTerrain;
    QString SecGoogleWord;

    // Google (China) version strings
    QString VersionGoogleMapChina;
    QString VersionGoogleSatelliteChina;
    QString VersionGoogleLabelsChina;
    QString VersionGoogleTerrainChina;

    // Google (Korea) version strings
    QString VersionGoogleMapKorea;
    QString VersionGoogleSatelliteKorea;
    QString VersionGoogleLabelsKorea;

    /// <summary>
    /// Google Maps API generated using http://greatmaps.codeplex.com/
    /// from http://code.google.com/intl/en-us/apis/maps/signup.html
    /// </summary>


    // Yahoo version strings
    QString VersionYahooMap;
    QString VersionYahooSatellite;
    QString VersionYahooLabels;

    // BingMaps
    QString VersionBingMaps;

    // YandexMap
    QString VersionYandexMap;



    /// <summary>
    /// Bing Maps Customer Identification, more info here
    /// http://msdn.microsoft.com/en-us/library/bb924353.aspx
    /// </summary>
    QString BingMapsClientToken;
};

#endif // PROVIDERSTRINGS_H
