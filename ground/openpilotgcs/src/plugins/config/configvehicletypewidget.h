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
#ifndef CONFIGVEHICLETYPEWIDGET_H
#define CONFIGVEHICLETYPEWIDGET_H

#include "ui_airframe.h"
#include "cfg_vehicletypes/vehicleconfig.h"
#include "uavobject.h"
#include "../uavobjectwidgetutils/configtaskwidget.h"

#include <QComboBox>
#include <QString>
#include <QStringList>
#include <QWidget>

class ConfigVehicleTypeWidget: public ConfigTaskWidget
{
    Q_OBJECT

public:
    static QStringList getChannelDescriptions();
    static void setComboCurrentIndex(QComboBox *box, int index);

    ConfigVehicleTypeWidget(QWidget *parent = 0);
    ~ConfigVehicleTypeWidget();

public slots:
    virtual void refreshWidgetsValues(UAVObject *o = NULL);
    virtual void updateObjectsFromWidgets();

private:
    Ui_AircraftWidget *m_aircraft;

    VehicleConfig *m_heli;
    VehicleConfig *m_fixedwing;
    VehicleConfig *m_multirotor;
    VehicleConfig *m_groundvehicle;
    VehicleConfig *m_custom;

    void updateFeedForwardUI();

    QString frameCategory(QString frameType);

    bool ffTuningInProgress;
    bool ffTuningPhase;
    UAVObject::Metadata accInitialData;

private slots:
    void switchAirframeType(int index);
    void enableFFTest();
    void openHelp();

};

#endif // CONFIGVEHICLETYPEWIDGET_H
