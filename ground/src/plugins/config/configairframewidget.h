/**
 ******************************************************************************
 *
 * @file       configairframetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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
#ifndef CONFIGAIRFRAMEWIDGET_H
#define CONFIGAIRFRAMEWIDGET_H

#include "ui_airframe.h"
#include "configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"
#include <QtGui/QWidget>
#include <QList>

class Ui_Widget;

class ConfigAirframeWidget: public ConfigTaskWidget
{
    Q_OBJECT

public:
    ConfigAirframeWidget(QWidget *parent = 0);
    ~ConfigAirframeWidget();

private:
    Ui_AircraftWidget *m_aircraft;
    bool setupFrameFixedWing();
    bool setupFrameElevon();
    bool setupFrameVtail();
    bool setupQuad(bool pLayout);
    bool setupHexa();
    bool setupOcto();

    void resetField(UAVObjectField * field);
    void resetMixer (MixerCurveWidget *mixer, int numElements);
    void resetActuators();
    //void setMixerChannel(int channelNumber, bool channelIsMotor, QList<double> vector);
    void setupQuadMotor(int channel, double roll, double pitch, double yaw);

    QStringList mixerTypes;
    QStringList mixerVectors;
    QGraphicsSvgItem *quad;

private slots:
    void requestAircraftUpdate();
    void sendAircraftUpdate();
    void saveAircraftUpdate();
    void setupAirframeUI(QString type);
    void toggleAileron2(int index);
    void toggleElevator2(int index);
    void switchAirframeType(int index);
    void resetFwMixer();
    void resetMrMixer();
    void updateFwThrottleCurveValue(QList<double> list, double value);
    void updateMrThrottleCurveValue(QList<double> list, double value);

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);


};

#endif // CONFIGAIRFRAMEWIDGET_H
