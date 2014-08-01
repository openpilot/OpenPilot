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

    connect(m_txpid->PID1, SIGNAL(currentTextChanged(QString)), this, SLOT(updateSpinBoxProperties(const QString &)));
    connect(m_txpid->PID2, SIGNAL(currentTextChanged(QString)), this, SLOT(updateSpinBoxProperties(const QString &)));
    connect(m_txpid->PID3, SIGNAL(currentTextChanged(QString)), this, SLOT(updateSpinBoxProperties(const QString &)));

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

static bool isResponsivenessOption(const QString & pidOption)
{
    return pidOption.endsWith(".Resp");
}

static bool isAttitudeOption(const QString & pidOption)
{
    return pidOption.contains("Attitude");
}

template <class StabilizationSettingsBankX>
static float defaultValueForPidOption(const StabilizationSettingsBankX *bank, const QString & pidOption)
{
    if (pidOption == "Disabled") {
        return 0.0f;
    } else if (pidOption == "Roll Rate.Kp") {
        return bank->getRollRatePID_Kp();
    } else if (pidOption == "Pitch Rate.Kp") {
        return bank->getPitchRatePID_Kp();
    } else if (pidOption == "Roll+Pitch Rate.Kp") {
        return bank->getRollRatePID_Kp();
    } else if (pidOption == "Yaw Rate.Kp") {
        return bank->getYawRatePID_Kp();
    } else if (pidOption == "Roll Rate.Ki") {
        return bank->getRollRatePID_Ki();
    } else if (pidOption == "Pitch Rate.Ki") {
        return bank->getPitchRatePID_Ki();
    } else if (pidOption == "Roll+Pitch Rate.Ki") {
        return bank->getRollRatePID_Ki();
    } else if (pidOption == "Yaw Rate.Ki") {
        return bank->getYawRatePID_Ki();
    } else if (pidOption == "Roll Rate.Kd") {
        return bank->getRollRatePID_Kd();
    } else if (pidOption == "Pitch Rate.Kd") {
        return bank->getPitchRatePID_Kd();
    } else if (pidOption == "Roll+Pitch Rate.Kd") {
        return bank->getRollRatePID_Kd();
    } else if (pidOption == "Yaw Rate.Kd") {
        return bank->getYawRatePID_Kd();
    } else if (pidOption == "Roll Rate.ILimit") {
        return bank->getRollRatePID_ILimit();
    } else if (pidOption == "Pitch Rate.ILimit") {
        return bank->getPitchRatePID_ILimit();
    } else if (pidOption == "Roll+Pitch Rate.ILimit") {
        return bank->getRollRatePID_ILimit();
    } else if (pidOption == "Yaw Rate.ILimit") {
        return bank->getYawRatePID_ILimit();
    } else if (pidOption == "Roll Rate.Resp") {
        return bank->getManualRate_Roll();
    } else if (pidOption == "Pitch Rate.Resp") {
        return bank->getManualRate_Pitch();
    } else if (pidOption == "Roll+Pitch Rate.Resp") {
        return bank->getManualRate_Roll();
    } else if (pidOption == "Yaw Rate.Resp") {
        return bank->getManualRate_Yaw();
    } else if (pidOption == "Roll Attitude.Kp") {
        return bank->getRollPI_Kp();
    } else if (pidOption == "Pitch Attitude.Kp") {
        return bank->getPitchPI_Kp();
    } else if (pidOption == "Roll+Pitch Attitude.Kp") {
        return bank->getRollPI_Kp();
    } else if (pidOption == "Yaw Attitude.Kp") {
        return bank->getYawPI_Kp();
    } else if (pidOption == "Roll Attitude.Ki") {
        return bank->getRollPI_Ki();
    } else if (pidOption == "Pitch Attitude.Ki") {
        return bank->getPitchPI_Ki();
    } else if (pidOption == "Roll+Pitch Attitude.Ki") {
        return bank->getRollPI_Ki();
    } else if (pidOption == "Yaw Attitude.Ki") {
        return bank->getYawPI_Ki();
    } else if (pidOption == "Roll Attitude.ILimit") {
        return bank->getRollPI_ILimit();
    } else if (pidOption == "Pitch Attitude.ILimit") {
        return bank->getPitchPI_ILimit();
    } else if (pidOption == "Roll+Pitch Attitude.ILimit") {
        return bank->getRollPI_ILimit();
    } else if (pidOption == "Yaw Attitude.ILimit") {
        return bank->getYawPI_ILimit();
    } else if (pidOption == "Roll Attitude.Resp") {
        return (float)bank->getRollMax();
    } else if (pidOption == "Pitch Attitude.Resp") {
        return (float)bank->getPitchMax();
    } else if (pidOption == "Roll+Pitch Attitude.Resp") {
        return (float)bank->getRollMax();
    } else if (pidOption == "Yaw Attitude.Resp") {
        return bank->getYawMax();
    }

    qDebug() << "getDefaultValueForOption: Incorrect PID option" << pidOption;
    return 0.0f;
}

float ConfigTxPIDWidget::getDefaultValueForPidOption(const QString & pidOption)
{
    if (pidOption == "GyroTau") {
        StabilizationSettings *stab = qobject_cast<StabilizationSettings *>(getObject(QString("StabilizationSettings")));
        return stab->getGyroTau();
    }

    uint bankNumber = m_txpid->pidBank->currentIndex() + 1;

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
        qDebug() << "getDefaultValueForPidOption: Incorrect bank number:" << bankNumber;
        return 0.0f;
    }
}

void ConfigTxPIDWidget::updateSpinBoxProperties(const QString & selectedPidOption)
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
        qDebug() << "updateSpinBoxProperties: Incorrect sender object";
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
