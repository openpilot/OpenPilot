/**
 ******************************************************************************
 *
 * @file       opmapgadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin OpenPilot Map Plugin
 * @{
 * @brief The OpenPilot Map plugin 
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

#ifndef OPMAP_GADGETCONFIGURATION_H
#define OPMAP_GADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include <QtCore/QString>

using namespace Core;

class OPMapGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT

Q_PROPERTY(QString mapProvider READ mapProvider WRITE setMapProvider)
Q_PROPERTY(int zoommo READ zoom WRITE setZoom)
Q_PROPERTY(double latitude READ latitude WRITE setLatitude)
Q_PROPERTY(double longitude READ longitude WRITE setLongitude)
Q_PROPERTY(bool useOpenGL READ useOpenGL WRITE setUseOpenGL)
Q_PROPERTY(bool showTileGridLines READ showTileGridLines WRITE setShowTileGridLines)
Q_PROPERTY(QString accessMode READ accessMode WRITE setAccessMode)
Q_PROPERTY(bool useMemoryCache READ useMemoryCache WRITE setUseMemoryCache)
Q_PROPERTY(QString cacheLocation READ cacheLocation WRITE setCacheLocation)
Q_PROPERTY(QString uavSymbol READ uavSymbol WRITE setUavSymbol)

public:
    explicit OPMapGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();

    QString mapProvider() const { return m_mapProvider; }
    int zoom() const { return m_defaultZoom; }
    double latitude() const { return m_defaultLatitude; }
    double longitude() const { return m_defaultLongitude; }
    bool useOpenGL() const { return m_useOpenGL; }
    bool showTileGridLines() const { return m_showTileGridLines; }
    QString accessMode() const { return m_accessMode; }
    bool useMemoryCache() const { return m_useMemoryCache; }
    QString cacheLocation() const { return m_cacheLocation; }
    QString uavSymbol() const { return m_uavSymbol; }

public slots:
    void setMapProvider(QString provider) { m_mapProvider = provider; }
    void setZoom(int zoom) { m_defaultZoom = zoom; }
    void setLatitude(double latitude) { m_defaultLatitude = latitude; }
    void setLongitude(double longitude) { m_defaultLongitude = longitude; }
    void setUseOpenGL(bool useOpenGL) { m_useOpenGL = useOpenGL; }
    void setShowTileGridLines(bool showTileGridLines) { m_showTileGridLines = showTileGridLines; }
    void setAccessMode(QString accessMode) { m_accessMode = accessMode; }
    void setUseMemoryCache(bool useMemoryCache) { m_useMemoryCache = useMemoryCache; }
    void setCacheLocation(QString cacheLocation);
    void setUavSymbol(QString symbol){m_uavSymbol=symbol;}
private:
    QString m_mapProvider;
    int m_defaultZoom;
    double m_defaultLatitude;
    double m_defaultLongitude;
    bool m_useOpenGL;
    bool m_showTileGridLines;
    QString m_accessMode;
    bool m_useMemoryCache;
    QString m_cacheLocation;
    QString m_uavSymbol;

};

#endif // OPMAP_GADGETCONFIGURATION_H
