/**
 ******************************************************************************
 *
 * @file       configairframetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Airframe configuration panel
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
#ifndef CONFIGFIXEDWINGWIDGET_H
#define CONFIGFIXEDWINGWIDGET_H

#include "ui_airframe.h"
#include "../uavobjectwidgetutils/configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "uavtalk/telemetrymanager.h"
#include <QtGui/QWidget>
#include <QList>
#include <QItemDelegate>

class Ui_Widget;

class ConfigFixedWingWidget: public VehicleConfig
{
    Q_OBJECT

public:
    ConfigFixedWingWidget(Ui_AircraftWidget *aircraft = 0, QWidget *parent = 0);
    ~ConfigFixedWingWidget();

    friend class ConfigVehicleTypeWidget;

private:
    Ui_AircraftWidget *m_aircraft;

    bool setupFrameFixedWing(QString airframeType);
    bool setupFrameElevon(QString airframeType);
    bool setupFrameVtail(QString airframeType);

    virtual void ResetActuators(GUIConfigDataUnion* configData);
    static QStringList getChannelDescriptions();

private slots:
    virtual void setupUI(QString airframeType);
    virtual void refreshWidgetsValues(QString frameType);
    virtual QString updateConfigObjectsFromWidgets();
    virtual bool throwConfigError(QString airframeType);


protected:

};


#endif // CONFIGFIXEDWINGWIDGET_H
