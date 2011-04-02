/**
 ******************************************************************************
 *
 * @file       opmapgadget.cpp
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
#include "opmapgadget.h"
#include "opmapgadgetwidget.h"
#include "opmapgadgetconfiguration.h"

OPMapGadget::OPMapGadget(QString classId, OPMapGadgetWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
        m_widget(widget)
{
}

OPMapGadget::~OPMapGadget()
{
    delete m_widget;
}

void OPMapGadget::loadConfiguration(IUAVGadgetConfiguration *config)
{
    OPMapGadgetConfiguration *m = qobject_cast<OPMapGadgetConfiguration*>(config);

    m_widget->setMapProvider(m->mapProvider());
    m_widget->setZoom(m->zoom());
    m_widget->setPosition(QPointF(m->longitude(), m->latitude()));
    m_widget->setUseOpenGL(m->useOpenGL());
    m_widget->setShowTileGridLines(m->showTileGridLines());
    m_widget->setAccessMode(m->accessMode());
    m_widget->setUseMemoryCache(m->useMemoryCache());
    m_widget->setCacheLocation(m->cacheLocation());
    m_widget->SetUavPic(m->uavSymbol());
}

