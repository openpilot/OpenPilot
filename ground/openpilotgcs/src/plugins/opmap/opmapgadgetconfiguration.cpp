/**
 ******************************************************************************
 *
 * @file       opmapgadgetconfiguration.cpp
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

#include "opmapgadgetconfiguration.h"
#include "utils/pathutils.h"
#include <QDir>

OPMapGadgetConfiguration::OPMapGadgetConfiguration(QString classId,  QSettings* qSettings, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_mapProvider("GoogleHybrid"),
    m_defaultZoom(2),
    m_defaultLatitude(0),
    m_defaultLongitude(0),
    m_useOpenGL(false),
    m_showTileGridLines(false),
    m_accessMode("ServerAndCache"),
    m_useMemoryCache(true),
    m_cacheLocation(Utils::PathUtils().GetStoragePath() + "mapscache" + QDir::separator()),
    m_uavSymbol(QString::fromUtf8(":/uavs/images/mapquad.png"))
{
    QSettings set(QSettings::IniFormat, QSettings::UserScope,QLatin1String("OpenPilot"), QLatin1String("OpenPilotGCS"));
    QDir dir(set.fileName());
    QFileInfo f(dir.absolutePath());
    f.dir().absolutePath();
    m_cacheLocation=f.dir().absolutePath()+QDir::separator() + "mapscache" + QDir::separator();

    //if a saved configuration exists load it
    if(qSettings != 0) {
        QString mapProvider  = qSettings->value("mapProvider").toString();
        int zoom = qSettings->value("defaultZoom").toInt();
        double latitude= qSettings->value("defaultLatitude").toDouble();
        double longitude= qSettings->value("defaultLongitude").toDouble();
        bool useOpenGL= qSettings->value("useOpenGL").toBool();
        bool showTileGridLines= qSettings->value("showTileGridLines").toBool();
        QString accessMode= qSettings->value("accessMode").toString();
        bool useMemoryCache= qSettings->value("useMemoryCache").toBool();
        QString cacheLocation= qSettings->value("cacheLocation").toString();
        QString uavSymbol=qSettings->value("uavSymbol").toString();
        if (!mapProvider.isEmpty()) m_mapProvider = mapProvider;
        m_defaultZoom = zoom;
        m_defaultLatitude = latitude;
        m_defaultLongitude = longitude;
        m_useOpenGL = useOpenGL;
        m_showTileGridLines = showTileGridLines;
        m_uavSymbol=uavSymbol;
        if (!accessMode.isEmpty()) m_accessMode = accessMode;
        m_useMemoryCache = useMemoryCache;
        if (!cacheLocation.isEmpty()) m_cacheLocation = Utils::PathUtils().InsertStoragePath(cacheLocation);
    }
}

IUAVGadgetConfiguration * OPMapGadgetConfiguration::clone()
{
    OPMapGadgetConfiguration *m = new OPMapGadgetConfiguration(this->classId());

    m->m_mapProvider = m_mapProvider;
    m->m_defaultZoom = m_defaultZoom;
    m->m_defaultLatitude = m_defaultLatitude;
    m->m_defaultLongitude = m_defaultLongitude;
    m->m_useOpenGL = m_useOpenGL;
    m->m_showTileGridLines = m_showTileGridLines;
    m->m_accessMode = m_accessMode;
    m->m_useMemoryCache = m_useMemoryCache;
    m->m_cacheLocation = m_cacheLocation;
    m->m_uavSymbol=m_uavSymbol;
    return m;
}

void OPMapGadgetConfiguration::saveConfig(QSettings* qSettings) const {
   qSettings->setValue("mapProvider", m_mapProvider);
   qSettings->setValue("defaultZoom", m_defaultZoom);
   qSettings->setValue("defaultLatitude", m_defaultLatitude);
   qSettings->setValue("defaultLongitude", m_defaultLongitude);
   qSettings->setValue("useOpenGL", m_useOpenGL);
   qSettings->setValue("showTileGridLines", m_showTileGridLines);
   qSettings->setValue("accessMode", m_accessMode);
   qSettings->setValue("useMemoryCache", m_useMemoryCache);
   qSettings->setValue("uavSymbol", m_uavSymbol);
   qSettings->setValue("cacheLocation", Utils::PathUtils().RemoveStoragePath(m_cacheLocation));
}
void OPMapGadgetConfiguration::setCacheLocation(QString cacheLocation){
    m_cacheLocation = cacheLocation;
}
