/**
 ******************************************************************************
 *
 * @file       uavobjectbrowser.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectBrowserPlugin UAVObject Browser Plugin
 * @{
 * @brief The UAVObject Browser gadget plugin
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
#include "uavobjectbrowser.h"
#include "uavobjectbrowserwidget.h"

UAVObjectBrowser::UAVObjectBrowser(QString classId, UAVObjectBrowserWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
        m_widget(widget),
        m_config(NULL)
{
    connect(m_widget,SIGNAL(viewOptionsChanged(bool,bool,bool)),this,SLOT(viewOptionsChangedSlot(bool,bool,bool)));
}

UAVObjectBrowser::~UAVObjectBrowser()
{
    delete m_widget;
}

void UAVObjectBrowser::loadConfiguration(IUAVGadgetConfiguration* config)
{
    UAVObjectBrowserConfiguration *m = qobject_cast<UAVObjectBrowserConfiguration*>(config);
    m_config=m;
    m_widget->setRecentlyUpdatedColor(m->recentlyUpdatedColor());
    m_widget->setManuallyChangedColor(m->manuallyChangedColor());
    m_widget->setRecentlyUpdatedTimeout(m->recentlyUpdatedTimeout());
    m_widget->setOnlyHilightChangedValues(m->onlyHighlightChangedValues());
    m_widget->setViewOptions(m->categorizedView(),m->scientificView(),m->showMetaData());
}

void UAVObjectBrowser::viewOptionsChangedSlot(bool categorized, bool scientific, bool metadata)
{
    if(m_config)
    {
        m_config->setCategorizedView(categorized);
        m_config->setScientificView(scientific);
        m_config->setShowMetaData(metadata);
    }
}

