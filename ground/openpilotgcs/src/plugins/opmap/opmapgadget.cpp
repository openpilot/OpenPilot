/**
 ******************************************************************************
 *
 * @file       opmapgadget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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
#include "opmapgadget.h"
#include "opmapgadgetwidget.h"

OPMapGadget::OPMapGadget(QString classId, OPMapGadgetWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
    m_widget(widget),m_config(NULL)
{
    connect(m_widget,SIGNAL(defaultLocationAndZoomChanged(double,double,double)),this,SLOT(saveDefaultLocation(double,double,double)));
    connect(m_widget,SIGNAL(overlayOpacityChanged(qreal)),this,SLOT(saveOpacity(qreal)));
}

OPMapGadget::~OPMapGadget()
{
    delete m_widget;
}
void OPMapGadget::saveDefaultLocation(double lng,double lat,double zoom)
{
    if(m_config)
    {
        m_config->setLatitude(lat);
        m_config->setLongitude(lng);
        m_config->setZoom(zoom);
        m_config->saveConfig();
    }
}

void OPMapGadget::saveOpacity(qreal value)
{
    if(m_config)
    {
        m_config->setOpacity(value);
    }
}
void OPMapGadget::loadConfiguration(IUAVGadgetConfiguration *config)
{
    m_config = qobject_cast<OPMapGadgetConfiguration*>(config);
    m_widget->setMapProvider(m_config->mapProvider());
    m_widget->setUseOpenGL(m_config->useOpenGL());
    m_widget->setShowTileGridLines(m_config->showTileGridLines());
    m_widget->setAccessMode(m_config->accessMode());
    m_widget->setUseMemoryCache(m_config->useMemoryCache());
    m_widget->setCacheLocation(m_config->cacheLocation());
    m_widget->SetUavPic(m_config->uavSymbol());
    m_widget->setZoom(m_config->zoom());
    m_widget->setPosition(QPointF(m_config->longitude(), m_config->latitude()));
    m_widget->setHomePosition(QPointF(m_config->longitude(), m_config->latitude()));
    m_widget->setOverlayOpacity(m_config->opacity());
}

