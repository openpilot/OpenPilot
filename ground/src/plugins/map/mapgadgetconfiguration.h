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

using namespace Core;

class MapGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit MapGadgetConfiguration(QString classId, const QByteArray &state = 0, QObject *parent = 0);
    int zoom() { return m_defaultZoom; }
    void setZoom(int zoom) { m_defaultZoom = zoom; }
    double latitude() { return m_defaultLatitude; }
    void setLatitude(double lat) { m_defaultLatitude = lat; }
    double longitude() { return m_defaultLongitude; }
    void setLongitude(double lon) { m_defaultLongitude = lon; }
    QByteArray saveState() const;
    IUAVGadgetConfiguration *clone(QString name);
signals:

public slots:

private:
    int m_defaultZoom;
    double m_defaultLatitude;
    double m_defaultLongitude;

};

#endif // MAPGADGETCONFIGURATION_H
