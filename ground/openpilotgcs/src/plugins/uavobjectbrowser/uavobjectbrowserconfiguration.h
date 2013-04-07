/**
 ******************************************************************************
 *
 * @file       uavobjectbrowserconfiguration.h
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

#ifndef UAVOBJECTBROWSERCONFIGURATION_H
#define UAVOBJECTBROWSERCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include <QtGui/QColor>

using namespace Core;

class UAVObjectBrowserConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
Q_PROPERTY(QColor m_recentlyUpdatedColor READ recentlyUpdatedColor WRITE setRecentlyUpdatedColor)
Q_PROPERTY(QColor m_manuallyChangedColor READ manuallyChangedColor WRITE setManuallyChangedColor)
Q_PROPERTY(int m_recentlyUpdatedTimeout READ recentlyUpdatedTimeout WRITE setRecentlyUpdatedTimeout)
Q_PROPERTY(bool m_onlyHilightChangedValues READ onlyHighlightChangedValues WRITE setOnlyHighlightChangedValues)
Q_PROPERTY(bool m_useCategorizedView READ categorizedView WRITE setCategorizedView)
Q_PROPERTY(bool m_useScientificView READ scientificView WRITE setScientificView)
Q_PROPERTY(bool m_showMetaData READ showMetaData WRITE setShowMetaData)
public:
    explicit UAVObjectBrowserConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();

    QColor recentlyUpdatedColor() const { return m_recentlyUpdatedColor; }
    QColor manuallyChangedColor() const { return m_manuallyChangedColor; }
    int recentlyUpdatedTimeout() const { return m_recentlyUpdatedTimeout; }
    bool onlyHighlightChangedValues() const {return m_onlyHilightChangedValues;}
    bool categorizedView() const { return m_useCategorizedView; }
    bool scientificView() const { return m_useScientificView; }
    bool showMetaData() const { return m_showMetaData; }

signals:

public slots:
    void setRecentlyUpdatedColor(QColor color) { m_recentlyUpdatedColor = color; }
    void setManuallyChangedColor(QColor color) { m_manuallyChangedColor = color; }
    void setRecentlyUpdatedTimeout(int timeout) { m_recentlyUpdatedTimeout = timeout; }
    void setOnlyHighlightChangedValues(bool hilight) { m_onlyHilightChangedValues = hilight; }
    void setCategorizedView(bool value) { m_useCategorizedView = value; }
    void setScientificView(bool value) { m_useScientificView = value; }
    void setShowMetaData(bool value) { m_showMetaData = value; }

private:
    QColor m_recentlyUpdatedColor;
    QColor m_manuallyChangedColor;
    int m_recentlyUpdatedTimeout;
    bool m_onlyHilightChangedValues;
    bool m_useCategorizedView;
    bool m_useScientificView;
    bool m_showMetaData;
};

#endif // UAVOBJECTBROWSERCONFIGURATION_H
