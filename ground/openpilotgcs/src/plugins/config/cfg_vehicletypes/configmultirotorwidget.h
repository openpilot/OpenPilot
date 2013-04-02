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
#ifndef CONFIGMULTIROTORWIDGET_H
#define CONFIGMULTIROTORWIDGET_H

#include "cfg_vehicletypes/vehicleconfig.h"
#include "ui_airframe_multirotor.h"
#include "../uavobjectwidgetutils/configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "uavtalk/telemetrymanager.h"

#include <QtGui/QWidget>
#include <QList>
#include <QItemDelegate>

class Ui_Widget;

class ConfigMultiRotorWidget: public VehicleConfig
{
    Q_OBJECT

public:

    static QStringList getChannelDescriptions();
    static const QString CHANNELBOXNAME;

    ConfigMultiRotorWidget(QWidget *parent = 0);
    ~ConfigMultiRotorWidget();

    virtual void setupUI(QString airframeType);
    virtual void refreshWidgetsValues(QString frameType);
    virtual QString updateConfigObjectsFromWidgets();

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    virtual void resetActuators(GUIConfigDataUnion *configData);

    Ui_MultiRotorConfigWidget *m_aircraft;

    QWidget *uiowner;
    QGraphicsSvgItem *quad;

    bool setupQuad(bool pLayout);
    bool setupHexa(bool pLayout);
    bool setupOcto();
    bool setupMultiRotorMixer(double mixerFactors[8][3]);
    void setupMotors(QList<QString> motorList);
    void setupQuadMotor(int channel, double roll, double pitch, double yaw);

    float invertMotors;

    void setYawMixLevel(int);

    void drawAirframe(QString multiRotorType);

private slots:
    virtual bool throwConfigError(int numMotors);

    void reverseMultirotorMotor();
};

#endif // CONFIGMULTIROTORWIDGET_H
