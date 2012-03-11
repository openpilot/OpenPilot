/**
 ******************************************************************************
 *
 * @file       configtelemetrytwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Telemetry configuration panel
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
#ifndef CONFIGCCHWWIDGET_H
#define CONFIGCCHWWIDGET_H

#include "ui_cc_hw_settings.h"
#include "../uavobjectwidgetutils/configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <QtGui/QWidget>
#include <QList>
#include "smartsavebutton.h"

class ConfigCCHWWidget: public ConfigTaskWidget
{
    Q_OBJECT

public:
    ConfigCCHWWidget(QWidget *parent = 0);
    ~ConfigCCHWWidget();
private slots:
    void openHelp();
    void refreshValues();
    void widgetsContentsChanged();

private:
    Ui_CC_HW_Widget *m_telemetry;
    QSvgRenderer *m_renderer;
};

#endif // CONFIGCCHWWIDGET_H
