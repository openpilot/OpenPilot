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

#include "cfg_vehicletypes/vehicleconfig.h"
#include "ui_airframe_fixedwing.h"
#include "../uavobjectwidgetutils/configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"

#include <QWidget>
#include <QList>
#include <QItemDelegate>

class Ui_Widget;

class ConfigFixedWingWidget : public VehicleConfig {
    Q_OBJECT

public:
    static const QString CHANNELBOXNAME;
    static QStringList getChannelDescriptions();

    ConfigFixedWingWidget(QWidget *parent = 0);
    ~ConfigFixedWingWidget();

    virtual void refreshWidgetsValues(QString frameType);
    virtual QString updateConfigObjectsFromWidgets();

protected:      									      
    void showEvent(QShowEvent *event);					      
    void resizeEvent(QResizeEvent *event);
    void enableControls(bool enable);

private:
    Ui_FixedWingConfigWidget *m_aircraft;
    QGraphicsSvgItem *plane;

    virtual void registerWidgets(ConfigTaskWidget &parent);
    virtual void resetActuators(GUIConfigDataUnion *configData);

    bool setupFrameFixedWing(QString airframeType);
    bool setupFrameVtail(QString airframeType);
    bool setupFixedWingMixer(double mixerFactors[8][3]);
    void setupMotors(QList<QString> motorList);

    void updateAirframe(QString multiRotorType);
    void setupEnabledControls(QString airframeType);
private slots:
    virtual void setupUI(QString airframeType);
    virtual bool throwConfigError(QString airframeType);
};

#endif // CONFIGFIXEDWINGWIDGET_H
