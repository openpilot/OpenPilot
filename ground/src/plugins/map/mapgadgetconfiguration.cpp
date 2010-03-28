/**
 ******************************************************************************
 *
 * @file       mapgadgetconfiguration.cpp
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

#include "mapgadgetconfiguration.h"
#include <QtCore/QDataStream>

MapGadgetConfiguration::MapGadgetConfiguration(QString classId, const QByteArray &state, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_defaultZoom(10)
{
    if (state.count() > 0) {
        QDataStream stream(state);
        int zoom;
        double latitude;
        double longitude;
        stream >> zoom;
        stream >> latitude;
        stream >> longitude;
        m_defaultZoom = zoom;
        m_defaultLatitude = latitude;
        m_defaultLongitude = longitude;

    }
}

IUAVGadgetConfiguration *MapGadgetConfiguration::clone()
{
    MapGadgetConfiguration *m = new MapGadgetConfiguration(this->classId());
    m->m_defaultZoom = m_defaultZoom;
    m->m_defaultLatitude = m_defaultLatitude;
    m->m_defaultLongitude = m_defaultLongitude;
    return m;
}

QByteArray MapGadgetConfiguration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << m_defaultZoom;
    stream << m_defaultLatitude;
    stream << m_defaultLongitude;
    return bytes;
}

