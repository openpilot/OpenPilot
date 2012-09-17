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
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/generalsettings.h>

ConfigTxPIDWidget::ConfigTxPIDWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_txpid = new Ui_TxPIDWidget();
    m_txpid->setupUi(this);
    
    ExtensionSystem::PluginManager *pm=ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings * settings=pm->getObject<Core::Internal::GeneralSettings>();
    if(!settings->useExpertMode())
        m_txpid->Apply->setVisible(false);
    autoLoadWidgets();
    addApplySaveButtons(m_txpid->Apply, m_txpid->Save);

    // Cannot use addUAVObjectToWidgetRelation() for OptionaModules enum because
    // QCheckBox returns bool (0 or -1) and this value is then set to enum instead
    // or enum options
    connect(HwSettings::GetInstance(getObjectManager()), SIGNAL(objectUpdated(UAVObject *)), this, SLOT(refreshValues()));
    connect(m_txpid->Apply, SIGNAL(clicked()), this, SLOT(applySettings()));
    connect(m_txpid->Save, SIGNAL(clicked()), this, SLOT(saveSettings()));

    addUAVObjectToWidgetRelation("TxPIDSettings", "PIDs", m_txpid->PID1, TxPIDSettings::PIDS_INSTANCE1);
    addUAVObjectToWidgetRelation("TxPIDSettings", "PIDs", m_txpid->PID2, TxPIDSettings::PIDS_INSTANCE2);
    addUAVObjectToWidgetRelation("TxPIDSettings", "PIDs", m_txpid->PID3, TxPIDSettings::PIDS_INSTANCE3);

    addUAVObjectToWidgetRelation("TxPIDSettings", "Inputs", m_txpid->Input1, TxPIDSettings::INPUTS_INSTANCE1);
    addUAVObjectToWidgetRelation("TxPIDSettings", "Inputs", m_txpid->Input2, TxPIDSettings::INPUTS_INSTANCE2);
    addUAVObjectToWidgetRelation("TxPIDSettings", "Inputs", m_txpid->Input3, TxPIDSettings::INPUTS_INSTANCE3);

    addUAVObjectToWidgetRelation("TxPIDSettings", "MinPID", m_txpid->MinPID1, TxPIDSettings::MINPID_INSTANCE1);
    addUAVObjectToWidgetRelation("TxPIDSettings", "MinPID", m_txpid->MinPID2, TxPIDSettings::MINPID_INSTANCE2);
    addUAVObjectToWidgetRelation("TxPIDSettings", "MinPID", m_txpid->MinPID3, TxPIDSettings::MINPID_INSTANCE3);

    addUAVObjectToWidgetRelation("TxPIDSettings", "MaxPID", m_txpid->MaxPID1, TxPIDSettings::MAXPID_INSTANCE1);
    addUAVObjectToWidgetRelation("TxPIDSettings", "MaxPID", m_txpid->MaxPID2, TxPIDSettings::MAXPID_INSTANCE2);
    addUAVObjectToWidgetRelation("TxPIDSettings", "MaxPID", m_txpid->MaxPID3, TxPIDSettings::MAXPID_INSTANCE3);

    addUAVObjectToWidgetRelation("TxPIDSettings", "ThrottleRange", m_txpid->ThrottleMin, TxPIDSettings::THROTTLERANGE_MIN);
    addUAVObjectToWidgetRelation("TxPIDSettings", "ThrottleRange", m_txpid->ThrottleMax, TxPIDSettings::THROTTLERANGE_MAX);

    addUAVObjectToWidgetRelation("TxPIDSettings", "UpdateMode", m_txpid->UpdateMode);

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


/**
  @}
  @}
  */
