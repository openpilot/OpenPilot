/**
 ******************************************************************************
 *
 * @file       inputpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup InputPage
 * @{
 * @brief
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

#include "inputpage.h"
#include "ui_inputpage.h"
#include "setupwizard.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "hwsettings.h"

InputPage::InputPage(SetupWizard *wizard, QWidget *parent) :
        AbstractWizardPage(wizard, parent),

    ui(new Ui::InputPage)
{
    ui->setupUi(this);
}

InputPage::~InputPage()
{
    delete ui;
}

bool InputPage::validatePage()
{
    if(ui->pwmButton->isChecked()) {
        getWizard()->setInputType(SetupWizard::INPUT_PWM);
    }
    else if(ui->ppmButton->isChecked()) {
        getWizard()->setInputType(SetupWizard::INPUT_PPM);
    }
    else if(ui->sbusButton->isChecked()) {
        getWizard()->setInputType(SetupWizard::INPUT_SBUS);
    }
    else if(ui->spectrumButton->isChecked()) {
        getWizard()->setInputType(SetupWizard::INPUT_DSM2);
    }
    else {
        getWizard()->setInputType(SetupWizard::INPUT_PWM);
    }
    getWizard()->setRestartNeeded(getWizard()->isRestartNeeded() || restartNeeded(getWizard()->getInputType()));

    return true;
}

bool InputPage::restartNeeded(VehicleConfigurationSource::INPUT_TYPE selectedType)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *uavoManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(uavoManager);
    HwSettings* hwSettings = HwSettings::GetInstance(uavoManager);
    HwSettings::DataFields data = hwSettings->getData();
    switch(selectedType)
    {
        case VehicleConfigurationSource::INPUT_PWM:
            return data.CC_RcvrPort != HwSettings::CC_RCVRPORT_PWM;
        case VehicleConfigurationSource::INPUT_PPM:
            return data.CC_RcvrPort != HwSettings::CC_RCVRPORT_PPM;
        case VehicleConfigurationSource::INPUT_SBUS:
            return data.CC_MainPort != HwSettings::CC_MAINPORT_SBUS;
        case VehicleConfigurationSource::INPUT_DSM2:
            // TODO: Handle all of the DSM types ?? Which is most common?
            return data.CC_MainPort != HwSettings::CC_MAINPORT_DSM2;
        default:
            return true;
    }
}
