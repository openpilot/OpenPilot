/**
******************************************************************************
*
* @file       providerstrings.cpp
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
* @brief      
* @see        The GNU Public License (GPL) Version 3
* @defgroup   OPMapWidget
* @{
* 
*****************************************************************************/
/* 
* This program is free software; you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by 
* the Free Software Foundation; either version 3 of the License, or 
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful, but 
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
* for more details.
* 
* You should have received a copy of the GNU General Public License along 
* with this program; if not, write to the Free Software Foundation, Inc., 
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "providerstrings.h"


namespace core {
    const QString ProviderStrings::levelsForSigPacSpainMap[] = {"0", "1", "2", "3", "4",
                                                                "MTNSIGPAC",
                                                                "MTN2000", "MTN2000", "MTN2000", "MTN2000", "MTN2000",
                                                                "MTN200", "MTN200", "MTN200",
                                                                "MTN25", "MTN25",
                                                                "ORTOFOTOS","ORTOFOTOS","ORTOFOTOS","ORTOFOTOS"};

    ProviderStrings::ProviderStrings()
    {
        // Google version strings
        VersionGoogleMap = "m@126";
        VersionGoogleSatellite = "61";
        VersionGoogleLabels = "h@126";
        VersionGoogleTerrain = "t@125,r@126";
        SecGoogleWord = "Galileo";

        // Google (China) version strings
        VersionGoogleMapChina = "m@126";
        VersionGoogleSatelliteChina = "s@61";
        VersionGoogleLabelsChina = "h@126";
        VersionGoogleTerrainChina = "t@125,r@126";

        // Google (Korea) version strings
        VersionGoogleMapKorea = "kr1.12";
        VersionGoogleSatelliteKorea = "61";
        VersionGoogleLabelsKorea = "kr1t.12";

        /// <summary>
        /// Google Maps API generated using http://greatmaps.codeplex.com/
        /// from http://code.google.com/intl/en-us/apis/maps/signup.html
        /// </summary>
        GoogleMapsAPIKey = "ABQIAAAAWaQgWiEBF3lW97ifKnAczhRAzBk5Igf8Z5n2W3hNnMT0j2TikxTLtVIGU7hCLLHMAuAMt-BO5UrEWA";

        // Yahoo version strings
        VersionYahooMap = "4.3";
        VersionYahooSatellite = "1.9";
        VersionYahooLabels = "4.3";

        // BingMaps
        VersionBingMaps = "470";

        // YandexMap
        VersionYandexMap = "2.10.2";

        /// <summary>
        /// Bing Maps Customer Identification, more info here
        /// http://msdn.microsoft.com/en-us/library/bb924353.aspx
        /// </summary>
        BingMapsClientToken = "";

    }
}
