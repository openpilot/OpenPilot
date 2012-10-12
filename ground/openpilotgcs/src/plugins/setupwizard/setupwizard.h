/**
 ******************************************************************************
 *
 * @file       setupwizard.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Setup Wizard  Plugin
 * @{
 * @brief A Wizards to make the initial setup easy for everyone.
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

#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H

#include <QWizard>
#include "levellingutil.h"
#include <coreplugin/icore.h>
#include <coreplugin/connectionmanager.h>
#include "vehicleconfigurationsource.h"
#include "vehicleconfigurationhelper.h"

class SetupWizard : public QWizard, public VehicleConfigurationSource
{
    Q_OBJECT

public:
    SetupWizard(QWidget *parent = 0);
    int nextId() const;

    void setControllerType(SetupWizard::CONTROLLER_TYPE type) { m_controllerType = type; }
    SetupWizard::CONTROLLER_TYPE getControllerType() const { return m_controllerType; }

    void setVehicleType(SetupWizard::VEHICLE_TYPE type) { m_vehicleType = type; }
    SetupWizard::VEHICLE_TYPE getVehicleType() const { return m_vehicleType; }

    void setVehicleSubType(SetupWizard::VEHICLE_SUB_TYPE type) { m_vehicleSubType = type; }
    SetupWizard::VEHICLE_SUB_TYPE getVehicleSubType() const { return m_vehicleSubType; }

    void setInputType(SetupWizard::INPUT_TYPE type) { m_inputType = type; }
    SetupWizard::INPUT_TYPE getInputType() const { return m_inputType; }

    void setESCType(SetupWizard::ESC_TYPE type) { m_escType = type; }
    SetupWizard::ESC_TYPE getESCType() const { return m_escType; }

    void setLevellingBias(accelGyroBias bias) { m_levellingBias = bias; m_levellingPerformed = true; }
    bool isLevellingPerformed() const { return m_levellingPerformed; }
    accelGyroBias getLevellingBias() const { return m_levellingBias; }

    void setActuatorSettings(QList<actuatorChannelSettings> actuatorSettings) { m_actuatorSettings = actuatorSettings; }
    bool isMotorCalibrationPerformed() const { return m_motorCalibrationPerformed; }
    QList<actuatorChannelSettings> getActuatorSettings() const { return m_actuatorSettings; }

    void setRestartNeeded(bool needed) { m_restartNeeded = needed; }
    bool isRestartNeeded() const {return m_restartNeeded; }

    QString getSummaryText();

    Core::ConnectionManager* getConnectionManager() {
        if (!m_connectionManager) {
            m_connectionManager = Core::ICore::instance()->connectionManager();
            Q_ASSERT(m_connectionManager);
        }
        return m_connectionManager;
    }

private slots:
    void customBackClicked();
    void pageChanged(int currId);
private:
    enum {PAGE_START, PAGE_CONTROLLER, PAGE_VEHICLES, PAGE_MULTI, PAGE_FIXEDWING,
          PAGE_HELI, PAGE_SURFACE, PAGE_INPUT, PAGE_OUTPUT, PAGE_LEVELLING, PAGE_CALIBRATION,
          PAGE_SAVE, PAGE_SUMMARY, PAGE_NOTYETIMPLEMENTED, PAGE_REBOOT, PAGE_END, PAGE_UPDATE};
    void createPages();
    bool saveHardwareSettings() const;
    bool canAutoUpdate() const;

    CONTROLLER_TYPE m_controllerType;
    VEHICLE_TYPE m_vehicleType;
    VEHICLE_SUB_TYPE m_vehicleSubType;
    INPUT_TYPE m_inputType;
    ESC_TYPE m_escType;

    bool m_levellingPerformed;
    accelGyroBias m_levellingBias;

    bool m_motorCalibrationPerformed;
    QList<actuatorChannelSettings> m_actuatorSettings;

    bool m_restartNeeded;

    bool m_back;

    Core::ConnectionManager *m_connectionManager;
};

#endif // SETUPWIZARD_H
