/**
 ******************************************************************************
 *
 * @file       uavobjectbrowserconfiguration.cpp
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

#include "uavobjectbrowserconfiguration.h"

UAVObjectBrowserConfiguration::UAVObjectBrowserConfiguration(QString classId, QSettings* qSettings, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_recentlyUpdatedColor(QColor(255, 230, 230)),
    m_manuallyChangedColor(QColor(230, 230, 255)),
    m_onlyHilightChangedValues(false),
    m_recentlyUpdatedTimeout(500),
    m_useCategorizedView(false),
    m_useScientificView(false),
    m_showMetaData(false)
{
    //if a saved configuration exists load it
    if(qSettings != 0) {
        QColor recent = qSettings->value("recentlyUpdatedColor").value<QColor>();
        QColor manual = qSettings->value("manuallyChangedColor").value<QColor>();
        int timeout = qSettings->value("recentlyUpdatedTimeout").toInt();
        bool hilight = qSettings->value("onlyHilightChangedValues").toBool();

        m_useCategorizedView = qSettings->value("CategorizedView").toBool();
        m_useScientificView = qSettings->value("ScientificView").toBool();
        m_showMetaData = qSettings->value("showMetaData").toBool();
        m_recentlyUpdatedColor = recent;
        m_manuallyChangedColor = manual;
        m_recentlyUpdatedTimeout = timeout;
        m_onlyHilightChangedValues = hilight;
    }
}

IUAVGadgetConfiguration *UAVObjectBrowserConfiguration::clone()
{
    UAVObjectBrowserConfiguration *m = new UAVObjectBrowserConfiguration(this->classId());
    m->m_recentlyUpdatedColor = m_recentlyUpdatedColor;
    m->m_manuallyChangedColor = m_manuallyChangedColor;
    m->m_recentlyUpdatedTimeout = m_recentlyUpdatedTimeout;
    m->m_onlyHilightChangedValues = m_onlyHilightChangedValues;
    m->m_useCategorizedView = m_useCategorizedView;
    m->m_useScientificView = m_useScientificView;
    m->m_showMetaData = m_showMetaData;
    return m;
}

/**
 * Saves a configuration.
 *
 */
void UAVObjectBrowserConfiguration::saveConfig(QSettings* qSettings) const {
    qSettings->setValue("recentlyUpdatedColor", m_recentlyUpdatedColor);
    qSettings->setValue("manuallyChangedColor", m_manuallyChangedColor);
    qSettings->setValue("recentlyUpdatedTimeout", m_recentlyUpdatedTimeout);
    qSettings->setValue("onlyHilightChangedValues", m_onlyHilightChangedValues);
    qSettings->setValue("CategorizedView", m_useCategorizedView);
    qSettings->setValue("ScientificView", m_useScientificView);
    qSettings->setValue("showMetaData", m_showMetaData);
}
