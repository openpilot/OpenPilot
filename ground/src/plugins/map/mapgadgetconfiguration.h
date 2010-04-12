/**
 ******************************************************************************
 *
 * @file       mapgadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   map
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

#ifndef MAPGADGETCONFIGURATION_H
#define MAPGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include <QtCore/QString>

using namespace Core;

class MapGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
Q_PROPERTY(QString mapProvider READ mapProvider WRITE setMapProvider)
Q_PROPERTY(int zoommo READ zoom WRITE setZoom)
Q_PROPERTY(double latitude READ latitude WRITE setLatitude)
Q_PROPERTY(double longitude READ longitude WRITE setLongitude)
public:
    explicit MapGadgetConfiguration(QString classId, const QByteArray &state = 0, QObject *parent = 0);
    QByteArray saveState() const;
    IUAVGadgetConfiguration *clone();

    QString mapProvider() const { return m_mapProvider; }
    int zoom() const { return m_defaultZoom; }
    double latitude() const { return m_defaultLatitude; }
    double longitude() const { return m_defaultLongitude; }

public slots:
    void setMapProvider(QString provider) { m_mapProvider = provider; }
    void setZoom(int zoom) { m_defaultZoom = zoom; }
    void setLatitude(double latitude) { m_defaultLatitude = latitude; }
    void setLongitude(double longitude) { m_defaultLongitude = longitude; }

private:
    QString m_mapProvider;
    int m_defaultZoom;
    double m_defaultLatitude;
    double m_defaultLongitude;

};

#endif // MAPGADGETCONFIGURATION_H
