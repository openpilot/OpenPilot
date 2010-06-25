/**
 ******************************************************************************
 *
 * @file       opmapgadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   opmap
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

#include "opmapgadgetconfiguration.h"
#include <QtCore/QDataStream>
#include <QDir>

OPMapGadgetConfiguration::OPMapGadgetConfiguration(QString classId, const QByteArray &state, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_mapProvider("GoogleHybrid"),
    m_defaultZoom(2),
    m_defaultLatitude(0),
    m_defaultLongitude(0),
    m_accessMode("ServerAndCache"),
    m_useMemoryCache(true),
    m_cacheLocation(QDir::currentPath() + QDir::separator() + "mapscache" + QDir::separator())
{
    if (state.count() > 0)
    {
        QDataStream stream(state);

        int zoom;
        double latitude;
        double longitude;
        QString mapProvider;
	QString accessMode;
	bool useMemoryCache;
	QString cacheLocation;

	stream >> zoom;
        stream >> latitude;
        stream >> longitude;
        stream >> mapProvider;
	stream >> accessMode;
	stream >> useMemoryCache;
	stream >> cacheLocation;

	m_defaultZoom = zoom;
        m_defaultLatitude = latitude;
        m_defaultLongitude = longitude;
	if (!mapProvider.isEmpty()) m_mapProvider = mapProvider;
	if (!accessMode.isEmpty()) m_accessMode = accessMode;
	m_useMemoryCache = useMemoryCache;
	if (!cacheLocation.isEmpty()) m_cacheLocation = cacheLocation;
    }
}

IUAVGadgetConfiguration * OPMapGadgetConfiguration::clone()
{
    OPMapGadgetConfiguration *m = new OPMapGadgetConfiguration(this->classId());

    m->m_defaultZoom = m_defaultZoom;
    m->m_defaultLatitude = m_defaultLatitude;
    m->m_defaultLongitude = m_defaultLongitude;
    m->m_mapProvider = m_mapProvider;
    m->m_accessMode = m_accessMode;
    m->m_useMemoryCache = m_useMemoryCache;
    m->m_cacheLocation = m_cacheLocation;

    return m;
}

QByteArray OPMapGadgetConfiguration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);

    stream << m_defaultZoom;
    stream << m_defaultLatitude;
    stream << m_defaultLongitude;
    stream << m_mapProvider;
    stream << m_accessMode;
    stream << m_useMemoryCache;
    stream << m_cacheLocation;

    return bytes;
}
