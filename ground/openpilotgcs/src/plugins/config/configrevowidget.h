/**
 ******************************************************************************
 *
 * @file       configahrstwidget.h
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
#ifndef CONFIGREVOWIDGET_H
#define CONFIGREVOWIDGET_H

#include "ui_revosensors.h"
#include "configtaskwidget.h"
#include "../uavobjectwidgetutils/configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <QWidget>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>
#include <QList>
#include <QTimer>
#include <QMutex>
#include "calibration/thermal/thermalcalibrationmodel.h"
#include "calibration/sixpointcalibrationmodel.h"
#include "calibration/levelcalibrationmodel.h"
#include "calibration/gyrobiascalibrationmodel.h"
class Ui_Widget;


class ConfigRevoWidget : public ConfigTaskWidget {
    Q_OBJECT

public:
    ConfigRevoWidget(QWidget *parent = 0);
    ~ConfigRevoWidget();

private:
    OpenPilot::SixPointCalibrationModel *m_sixPointCalibrationModel;
    OpenPilot::ThermalCalibrationModel *m_thermalCalibrationModel;
    OpenPilot::LevelCalibrationModel *m_levelCalibrationModel;
    OpenPilot::GyroBiasCalibrationModel *m_gyroBiasCalibrationModel;

    Ui_RevoSensorsWidget *m_ui;

    // Board rotation store/recall
    qint16 storedBoardRotation[3];
    bool isBoardRotationStored;

private slots:
    void displayVisualHelp(QString elementID);
    void storeAndClearBoardRotation();
    void recallBoardRotation();
    void displayInstructions(QString instructions = QString(), bool replace = false);

    // ! Overriden method from the configTaskWidget to update UI
    virtual void refreshWidgetsValues(UAVObject *object = NULL);

    // Slot for clearing home location
    void clearHomeLocation();

    void disableAllCalibrations();
    void enableAllCalibrations();

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);
};

#endif // ConfigRevoWidget_H
