/**
 ******************************************************************************
 *
 * @file       configrevohwwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Revolution hardware configuration panel
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
#ifndef CONFIGREVOHWWIDGET_H
#define CONFIGREVOHWWIDGET_H

#include "ui_configrevohwwidget.h"
#include "../uavobjectwidgetutils/configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <QtGui/QWidget>
#include <QList>


class ConfigRevoHWWidget: public ConfigTaskWidget
{
    Q_OBJECT

public:
    ConfigRevoHWWidget(QWidget *parent = 0);
    ~ConfigRevoHWWidget();

private:
    Ui_RevoHWWidget *m_ui;
    void setupCustomCombos();

protected slots:
    void refreshWidgetsValues(UAVObject * obj = NULL);
    void updateObjectsFromWidgets();

private slots:
    void usbVCPPortChanged(int index);
    void usbHIDPortChanged(int index);
    void flexiPortChanged(int index);
    void mainPortChanged(int index);
    void modemPortChanged(int index);
    void openHelp();

};

#endif // CONFIGREVOHWWIDGET_H
