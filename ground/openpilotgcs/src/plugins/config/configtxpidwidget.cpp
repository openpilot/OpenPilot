/**
 ******************************************************************************
 *
 * @file       configtxpidswidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to configure TxPID module
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

#include "configtxpidwidget.h"
#include "txpidsettings.h"
#include "hwsettings.h"
#include "stabilizationsettings.h"
#include "stabilizationsettingsbank1.h"
#include "stabilizationsettingsbank2.h"
#include "stabilizationsettingsbank3.h"
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/generalsettings.h>

ConfigTxPIDWidget::ConfigTxPIDWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_txpid = new Ui_TxPIDWidget();
    m_txpid->setupUi(this);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings *settings = pm->getObject<Core::Internal::GeneralSettings>();
    if (!settings->useExpertMode()) {
        m_txpid->Apply->setVisible(false);
    }
    autoLoadWidgets();
    addApplySaveButtons(m_txpid->Apply, m_txpid->Save);

    // Cannot use addUAVObjectToWidgetRelation() for OptionaModules enum because
    // QCheckBox returns bool (0 or -1) and this value is then set to enum instead
    // or enum options
    connect(HwSettings::GetInstance(getObjectManager()), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshValues()));
    connect(m_txpid->Apply, SIGNAL(clicked()), this, SLOT(applySettings()));
    connect(m_txpid->Save, SIGNAL(clicked()), this, SLOT(saveSettings()));

    connect(m_txpid->PID1, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSpinBoxProperties(int)));
    connect(m_txpid->PID2, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSpinBoxProperties(int)));
    connect(m_txpid->PID3, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSpinBoxProperties(int)));

    addWidgetBinding("TxPIDSettings", "BankNumber", m_txpid->pidBank, 0, 1, true);

    addWidgetBinding("TxPIDSettings", "Inputs", m_txpid->Input1, TxPIDSettings::INPUTS_INSTANCE1);
    addWidgetBinding("TxPIDSettings", "Inputs", m_txpid->Input2, TxPIDSettings::INPUTS_INSTANCE2);
    addWidgetBinding("TxPIDSettings", "Inputs", m_txpid->Input3, TxPIDSettings::INPUTS_INSTANCE3);

    // It's important that the PIDx values are populated before the MinPIDx and MaxPIDx,
    // otherwise the MinPIDx and MaxPIDx will be capped by the old spin box limits. The correct limits
    // are set when updateSpinBoxProperties is called when the PIDx->currentTextChanged signal is sent.
    // The binding order is reversed because the values are populated in reverse.

    addWidgetBinding("TxPIDSettings", "MinPID", m_txpid->MinPID1, TxPIDSettings::MINPID_INSTANCE1);
    addWidgetBinding("TxPIDSettings", "MinPID", m_txpid->MinPID2, TxPIDSettings::MINPID_INSTANCE2);
    addWidgetBinding("TxPIDSettings", "MinPID", m_txpid->MinPID3, TxPIDSettings::MINPID_INSTANCE3);

    addWidgetBinding("TxPIDSettings", "MaxPID", m_txpid->MaxPID1, TxPIDSettings::MAXPID_INSTANCE1);
    addWidgetBinding("TxPIDSettings", "MaxPID", m_txpid->MaxPID2, TxPIDSettings::MAXPID_INSTANCE2);
    addWidgetBinding("TxPIDSettings", "MaxPID", m_txpid->MaxPID3, TxPIDSettings::MAXPID_INSTANCE3);

    addWidgetBinding("TxPIDSettings", "PIDs", m_txpid->PID1, TxPIDSettings::PIDS_INSTANCE1);
    addWidgetBinding("TxPIDSettings", "PIDs", m_txpid->PID2, TxPIDSettings::PIDS_INSTANCE2);
    addWidgetBinding("TxPIDSettings", "PIDs", m_txpid->PID3, TxPIDSettings::PIDS_INSTANCE3);

    addWidgetBinding("TxPIDSettings", "ThrottleRange", m_txpid->ThrottleMin, TxPIDSettings::THROTTLERANGE_MIN);
    addWidgetBinding("TxPIDSettings", "ThrottleRange", m_txpid->ThrottleMax, TxPIDSettings::THROTTLERANGE_MAX);

    addWidgetBinding("TxPIDSettings", "UpdateMode", m_txpid->UpdateMode);

    addWidget(m_txpid->TxPIDEnable);

    enableControls(false);
    populateWidgets();
    refreshWidgetsValues();

    disableMouseWheelEvents();
}

ConfigTxPIDWidget::~ConfigTxPIDWidget()
{
    // Do nothing
}

static bool isResponsivenessOption(int pidOption)
{
    switch (pidOption) {
    case TxPIDSettings::PIDS_ROLLRATERESP:
    case TxPIDSettings::PIDS_PITCHRATERESP:
    case TxPIDSettings::PIDS_ROLLPITCHRATERESP:
    case TxPIDSettings::PIDS_YAWRATERESP:
    case TxPIDSettings::PIDS_ROLLATTITUDERESP:
    case TxPIDSettings::PIDS_PITCHATTITUDERESP:
    case TxPIDSettings::PIDS_ROLLPITCHATTITUDERESP:
    case TxPIDSettings::PIDS_YAWATTITUDERESP:
        return true;

    default:
        return false;
    }
}

static bool isAttitudeOption(int pidOption)
{
    switch (pidOption) {
    case TxPIDSettings::PIDS_ROLLATTITUDEKP:
    case TxPIDSettings::PIDS_PITCHATTITUDEKP:
    case TxPIDSettings::PIDS_ROLLPITCHATTITUDEKP:
    case TxPIDSettings::PIDS_YAWATTITUDEKP:
    case TxPIDSettings::PIDS_ROLLATTITUDEKI:
    case TxPIDSettings::PIDS_PITCHATTITUDEKI:
    case TxPIDSettings::PIDS_ROLLPITCHATTITUDEKI:
    case TxPIDSettings::PIDS_YAWATTITUDEKI:
    case TxPIDSettings::PIDS_ROLLATTITUDEILIMIT:
    case TxPIDSettings::PIDS_PITCHATTITUDEILIMIT:
    case TxPIDSettings::PIDS_ROLLPITCHATTITUDEILIMIT:
    case TxPIDSettings::PIDS_YAWATTITUDEILIMIT:
    case TxPIDSettings::PIDS_ROLLATTITUDERESP:
    case TxPIDSettings::PIDS_PITCHATTITUDERESP:
    case TxPIDSettings::PIDS_ROLLPITCHATTITUDERESP:
    case TxPIDSettings::PIDS_YAWATTITUDERESP:
        return true;

    default:
        return false;
    }
}

static bool isExpoOption(int pidOption)
{
    switch (pidOption) {
    case TxPIDSettings::PIDS_ROLLEXPO:
    case TxPIDSettings::PIDS_PITCHEXPO:
    case TxPIDSettings::PIDS_ROLLPITCHEXPO:
    case TxPIDSettings::PIDS_YAWEXPO:
        return true;

    default:
        return false;
    }
}

static bool isAcroPlusFactorOption(int pidOption)
{
    return pidOption == TxPIDSettings::PIDS_ACROPLUSFACTOR;
}

template <class StabilizationSettingsBankX>
static float defaultValueForPidOption(const StabilizationSettingsBankX *bank, int pidOption)
{
    switch (pidOption) {
    case TxPIDSettings::PIDS_DISABLED:
        return 0.0f;

    case TxPIDSettings::PIDS_ROLLRATEKP:
        return bank->getRollRatePID_Kp();

    case TxPIDSettings::PIDS_PITCHRATEKP:
        return bank->getPitchRatePID_Kp();

    case TxPIDSettings::PIDS_ROLLPITCHRATEKP:
        return bank->getRollRatePID_Kp();

    case TxPIDSettings::PIDS_YAWRATEKP:
        return bank->getYawRatePID_Kp();

    case TxPIDSettings::PIDS_ROLLRATEKI:
        return bank->getRollRatePID_Ki();

    case TxPIDSettings::PIDS_PITCHRATEKI:
        return bank->getPitchRatePID_Ki();

    case TxPIDSettings::PIDS_ROLLPITCHRATEKI:
        return bank->getRollRatePID_Ki();

    case TxPIDSettings::PIDS_YAWRATEKI:
        return bank->getYawRatePID_Ki();

    case TxPIDSettings::PIDS_ROLLRATEKD:
        return bank->getRollRatePID_Kd();

    case TxPIDSettings::PIDS_PITCHRATEKD:
        return bank->getPitchRatePID_Kd();

    case TxPIDSettings::PIDS_ROLLPITCHRATEKD:
        return bank->getRollRatePID_Kd();

    case TxPIDSettings::PIDS_YAWRATEKD:
        return bank->getYawRatePID_Kd();

    case TxPIDSettings::PIDS_ROLLRATEILIMIT:
        return bank->getRollRatePID_ILimit();

    case TxPIDSettings::PIDS_PITCHRATEILIMIT:
        return bank->getPitchRatePID_ILimit();

    case TxPIDSettings::PIDS_ROLLPITCHRATEILIMIT:
        return bank->getRollRatePID_ILimit();

    case TxPIDSettings::PIDS_YAWRATEILIMIT:
        return bank->getYawRatePID_ILimit();

    case TxPIDSettings::PIDS_ROLLRATERESP:
        return bank->getManualRate_Roll();

    case TxPIDSettings::PIDS_PITCHRATERESP:
        return bank->getManualRate_Pitch();

    case TxPIDSettings::PIDS_ROLLPITCHRATERESP:
        return bank->getManualRate_Roll();

    case TxPIDSettings::PIDS_YAWRATERESP:
        return bank->getManualRate_Yaw();

    case TxPIDSettings::PIDS_ROLLATTITUDEKP:
        return bank->getRollPI_Kp();

    case TxPIDSettings::PIDS_PITCHATTITUDEKP:
        return bank->getPitchPI_Kp();

    case TxPIDSettings::PIDS_ROLLPITCHATTITUDEKP:
        return bank->getRollPI_Kp();

    case TxPIDSettings::PIDS_YAWATTITUDEKP:
        return bank->getYawPI_Kp();

    case TxPIDSettings::PIDS_ROLLATTITUDEKI:
        return bank->getRollPI_Ki();

    case TxPIDSettings::PIDS_PITCHATTITUDEKI:
        return bank->getPitchPI_Ki();

    case TxPIDSettings::PIDS_ROLLPITCHATTITUDEKI:
        return bank->getRollPI_Ki();

    case TxPIDSettings::PIDS_YAWATTITUDEKI:
        return bank->getYawPI_Ki();

    case TxPIDSettings::PIDS_ROLLATTITUDEILIMIT:
        return bank->getRollPI_ILimit();

    case TxPIDSettings::PIDS_PITCHATTITUDEILIMIT:
        return bank->getPitchPI_ILimit();

    case TxPIDSettings::PIDS_ROLLPITCHATTITUDEILIMIT:
        return bank->getRollPI_ILimit();

    case TxPIDSettings::PIDS_YAWATTITUDEILIMIT:
        return bank->getYawPI_ILimit();

    case TxPIDSettings::PIDS_ROLLATTITUDERESP:
        return (float)bank->getRollMax();

    case TxPIDSettings::PIDS_PITCHATTITUDERESP:
        return (float)bank->getPitchMax();

    case TxPIDSettings::PIDS_ROLLPITCHATTITUDERESP:
        return (float)bank->getRollMax();

    case TxPIDSettings::PIDS_YAWATTITUDERESP:
        return bank->getYawMax();

    case TxPIDSettings::PIDS_ROLLEXPO:
        return bank->getStickExpo_Roll();

    case TxPIDSettings::PIDS_PITCHEXPO:
        return bank->getStickExpo_Pitch();

    case TxPIDSettings::PIDS_ROLLPITCHEXPO:
        return bank->getStickExpo_Roll();

    case TxPIDSettings::PIDS_YAWEXPO:
        return bank->getStickExpo_Yaw();

    case -1: // The PID Option field was uninitialized.
        return 0.0f;

    default:
        Q_ASSERT_X(false, "getDefaultValueForOption", "Incorrect PID option");
        return 0.0f;
    }
}

float ConfigTxPIDWidget::getDefaultValueForPidOption(int pidOption)
{
    if (pidOption == TxPIDSettings::PIDS_GYROTAU) {
        StabilizationSettings *stab = qobject_cast<StabilizationSettings *>(getObject(QString("StabilizationSettings")));
        return stab->getGyroTau();
    }

    int pidBankIndex = m_txpid->pidBank->currentIndex();

    if (pidBankIndex == -1) {
        // The pidBank field was uninitilized.
        return 0.0f;
    }

    int bankNumber = pidBankIndex + 1;

    if (bankNumber == 1) {
        StabilizationSettingsBank1 *bank = qobject_cast<StabilizationSettingsBank1 *>(getObject(QString("StabilizationSettingsBank1")));
        return defaultValueForPidOption(bank, pidOption);
    } else if (bankNumber == 2) {
        StabilizationSettingsBank2 *bank = qobject_cast<StabilizationSettingsBank2 *>(getObject(QString("StabilizationSettingsBank2")));
        return defaultValueForPidOption(bank, pidOption);
    } else if (bankNumber == 3) {
        StabilizationSettingsBank3 *bank = qobject_cast<StabilizationSettingsBank3 *>(getObject(QString("StabilizationSettingsBank3")));
        return defaultValueForPidOption(bank, pidOption);
    } else {
        Q_ASSERT_X(false, "getDefaultValueForPidOption", "Incorrect bank number");
        return 0.0f;
    }
}

void ConfigTxPIDWidget::updateSpinBoxProperties(int selectedPidOption)
{
    QObject *PIDx = sender();

    QDoubleSpinBox *minPID;
    QDoubleSpinBox *maxPID;

    if (PIDx == m_txpid->PID1) {
        minPID = m_txpid->MinPID1;
        maxPID = m_txpid->MaxPID1;
    } else if (PIDx == m_txpid->PID2) {
        minPID = m_txpid->MinPID2;
        maxPID = m_txpid->MaxPID2;
    } else if (PIDx == m_txpid->PID3) {
        minPID = m_txpid->MinPID3;
        maxPID = m_txpid->MaxPID3;
    } else {
        Q_ASSERT_X(false, "updateSpinBoxProperties", "Incorrect sender object");
        return;
    }

    // The ranges need to be setup before the values can be set,
    // otherwise the value might be incorrectly capped.

    if (isResponsivenessOption(selectedPidOption)) {
        if (isAttitudeOption(selectedPidOption)) {
            // Limit to 180 degrees.
            minPID->setRange(0, 180);
            maxPID->setRange(0, 180);
        } else {
            minPID->setRange(0, 999);
            maxPID->setRange(0, 999);
        }
        minPID->setSingleStep(1);
        maxPID->setSingleStep(1);
        minPID->setDecimals(0);
        maxPID->setDecimals(0);
    } else if (isExpoOption(selectedPidOption)) {
        minPID->setRange(-100, 100);
        maxPID->setRange(-100, 100);
        minPID->setSingleStep(1);
        maxPID->setSingleStep(1);
        minPID->setDecimals(0);
        maxPID->setDecimals(0);
    } else if (isAcroPlusFactorOption(selectedPidOption)) {
        minPID->setRange(0, 1);
        maxPID->setRange(0, 1);
        minPID->setSingleStep(0.01);
        maxPID->setSingleStep(0.01);
        minPID->setDecimals(2);
        maxPID->setDecimals(2);
    } else {
        minPID->setRange(0, 99.99);
        maxPID->setRange(0, 99.99);
        minPID->setSingleStep(0.000100);
        maxPID->setSingleStep(0.000100);
        minPID->setDecimals(6);
        maxPID->setDecimals(6);
    }

    float value = getDefaultValueForPidOption(selectedPidOption);

    minPID->setValue(value);
    maxPID->setValue(value);
}

void ConfigTxPIDWidget::refreshValues()
{
    HwSettings *hwSettings = HwSettings::GetInstance(getObjectManager());
    HwSettings::DataFields hwSettingsData = hwSettings->getData();

    m_txpid->TxPIDEnable->setChecked(
        hwSettingsData.OptionalModules[HwSettings::OPTIONALMODULES_TXPID] == HwSettings::OPTIONALMODULES_ENABLED);
}

void ConfigTxPIDWidget::applySettings()
{
    HwSettings *hwSettings = HwSettings::GetInstance(getObjectManager());
    HwSettings::DataFields hwSettingsData = hwSettings->getData();

    hwSettingsData.OptionalModules[HwSettings::OPTIONALMODULES_TXPID] =
        m_txpid->TxPIDEnable->isChecked() ? HwSettings::OPTIONALMODULES_ENABLED : HwSettings::OPTIONALMODULES_DISABLED;
    hwSettings->setData(hwSettingsData);
}

void ConfigTxPIDWidget::saveSettings()
{
    applySettings();
    UAVObject *obj = HwSettings::GetInstance(getObjectManager());
    saveObjectToSD(obj);
}
