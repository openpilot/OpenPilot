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
#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>

/*
 * This class derives from ConfigTaskWidget and overrides its default "binding" mechanism.
 * This widget bypasses automatic synchronization of UAVObjects and UI by providing its own implementations of
 *     virtual void refreshWidgetsValues(UAVObject *obj = NULL);
 *     virtual void updateObjectsFromWidgets();
 *
 * It does use the "dirty" state management and registers its relevant widgets with ConfigTaskWidget to do so.
 *
 * This class also manages child ConfigTaskWidget : see VehicleConfig class and its derived classes.
 * Note: for "dirty" state management it is important to register the fields of child widgets with the parent
 * ConfigVehicleTypeWidget class.
 *
 * TODO consider to call "super" to benefit from default logic...
 * TODO improve handling of relationship with VehicleConfig derived classes (i.e. ConfigTaskWidget within ConfigTaskWidget)
 */
class ConfigVehicleTypeWidget: public ConfigTaskWidget
{
    Q_OBJECT

public:
    static QStringList getChannelDescriptions();
    static void setComboCurrentIndex(QComboBox *box, int index);

    ConfigVehicleTypeWidget(QWidget *parent = 0);
    ~ConfigVehicleTypeWidget();

protected slots:
    virtual void refreshWidgetsValues(UAVObject *o = NULL);
    virtual void updateObjectsFromWidgets();

private:
    Ui_AircraftWidget *m_aircraft;

    // Maps a frame category to its index in the m_aircraft->airframesWidget QStackedWidget
    QMap<QString, int> vehicleIndexMap;

    QString frameCategory(QString frameType);

    VehicleConfig *getVehicleConfigWidget(QString frameCategory);
    VehicleConfig *createVehicleConfigWidget(QString frameCategory);

    // Feed Forward
    void updateFeedForwardUI();

    bool ffTuningInProgress;
    bool ffTuningPhase;
    UAVObject::Metadata accInitialData;

private slots:
    void switchAirframeType(int index);
    void openHelp();
    void enableFFTest();

};

#endif // CONFIGVEHICLETYPEWIDGET_H
