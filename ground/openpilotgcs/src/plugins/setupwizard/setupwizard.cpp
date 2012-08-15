/**
 ******************************************************************************
 *
 * @file       setupwizard.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Setup Wizard  Plugin
 * @{
 * @brief A Wizard to make the initial setup easy for everyone.
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

#include "setupwizard.h"
#include "pages/startpage.h"
#include "pages/endpage.h"
#include "pages/controllerpage.h"
#include "pages/vehiclepage.h"
#include "pages/multipage.h"
#include "pages/fixedwingpage.h"
#include "pages/helipage.h"
#include "pages/surfacepage.h"
#include "pages/inputpage.h"
#include "pages/outputpage.h"
#include "pages/levellingpage.h"
#include "pages/summarypage.h"
#include "pages/flashpage.h"
#include "pages/notyetimplementedpage.h"
#include "extensionsystem/pluginmanager.h"
#include "hwsettings.h"
#include "actuatorsettings.h"
#include "attitudesettings.h"

SetupWizard::SetupWizard(QWidget *parent) : QWizard(parent),
    m_controllerSelectionMode(CONTROLLER_SELECTION_UNKNOWN), m_controllerType(CONTROLLER_UNKNOWN),
    m_vehicleType(VEHICLE_UNKNOWN), m_inputType(INPUT_UNKNOWN), m_escType(ESC_UNKNOWN),
    m_levellingPerformed(false), m_connectionManager(0)
{
    setWindowTitle("OpenPilot Setup Wizard");
    setOption(QWizard::IndependentPages, false);
    createPages();
}

int SetupWizard::nextId() const
{
    switch (currentId()) {
        case PAGE_START:
            return PAGE_CONTROLLER;
        case PAGE_CONTROLLER: {
            switch(getControllerType())
            {
                case CONTROLLER_CC:
                case CONTROLLER_CC3D:
                    return PAGE_INPUT;
                case CONTROLLER_REVO:
                case CONTROLLER_PIPX:
                default:
                    return PAGE_NOTYETIMPLEMENTED;
            }
        }
        case PAGE_VEHICLES: {
            switch(getVehicleType())
            {
                case VEHICLE_MULTI:
                    return PAGE_MULTI;
                case VEHICLE_FIXEDWING:
                    return PAGE_FIXEDWING;
                case VEHICLE_HELI:
                    return PAGE_HELI;
                case VEHICLE_SURFACE:
                    return PAGE_SURFACE;
                default:
                    return PAGE_NOTYETIMPLEMENTED;
            }
        }
        case PAGE_MULTI:
            return PAGE_OUTPUT;
        case PAGE_INPUT:
            return PAGE_VEHICLES;
        case PAGE_OUTPUT:
        {
            if(getControllerSelectionMode() == CONTROLLER_SELECTION_AUTOMATIC) {
                return PAGE_LEVELLING;
            } else {
                return PAGE_SUMMARY;
            }
        }
        case PAGE_LEVELLING:
            return PAGE_SUMMARY;
        case PAGE_SUMMARY:
            return PAGE_FLASH;
        case PAGE_FLASH:
            return PAGE_END;
        case PAGE_NOTYETIMPLEMENTED:
            return PAGE_END;
        default:
            return -1;
    }
}

QString SetupWizard::getSummaryText()
{
    QString summary = "";
    summary.append(tr("Controller type: "));
    switch(getControllerType())
    {
        case CONTROLLER_CC:
            summary.append(tr("OpenPilot CopterControl"));
            break;
        case CONTROLLER_CC3D:
            summary.append(tr("OpenPilot CopterControl 3D"));
            break;
        case CONTROLLER_REVO:
            summary.append(tr("OpenPilot Revolution"));
            break;
        case CONTROLLER_PIPX:
            summary.append(tr("OpenPilot PipX Radio Modem"));
            break;
        default:
            summary.append(tr("Unknown"));
            break;
    }

    summary.append('\n');
    summary.append(tr("Vehicle type: "));
    switch (getVehicleType())
    {
        case VEHICLE_MULTI:
            summary.append(tr("Multirotor"));
            break;
        case VEHICLE_FIXEDWING:
            summary.append(tr("Fixed wing"));
            break;
        case VEHICLE_HELI:
            summary.append(tr("Helicopter"));
            break;
        case VEHICLE_SURFACE:
            summary.append(tr("Surface vehicle"));
            break;
        default:
            summary.append(tr("Unknown"));
    }

    summary.append('\n');
    summary.append(tr("Input type: "));
    switch (getInputType())
    {
        case INPUT_PWM:
            summary.append(tr("PWM (One cable per channel)"));
            break;
        case INPUT_PPM:
            summary.append(tr("PPM (One cable for all channels)"));
            break;
        case INPUT_SBUS:
            summary.append(tr("Futaba S.Bus"));
            break;
        case INPUT_DSM:
            summary.append(tr("Spectrum satellite"));
            break;
        default:
            summary.append(tr("Unknown"));
    }

    summary.append('\n');
    summary.append(tr("ESC type: "));
    switch (getESCType())
    {
        case ESC_DEFAULT:
            summary.append(tr("Default ESC (50 Hz)"));
            break;
        case ESC_RAPID:
            summary.append(tr("Rapid ESC (400 Hz)"));
            break;
        default:
            summary.append(tr("Unknown"));
    }

    summary.append('\n');
    summary.append(tr("Accel & Gyro bias calibrated: "));
    if (isLevellingPerformed()) {
        summary.append(tr("Yes"));
    }
    else {
        summary.append(tr("No"));
    }

    return summary;
}

void SetupWizard::applyConfiguration()
{
    UAVObjectManager* uavoMgr = getUAVObjectManager();
    applyHardwareConfiguration(uavoMgr);
    applyVehicleConfiguration(uavoMgr);
    applyOutputConfiguration(uavoMgr);
    applyLevellingConfiguration(uavoMgr);
}

UAVObjectManager* SetupWizard::getUAVObjectManager()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * uavObjectManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(uavObjectManager);
    return uavObjectManager;
}

void SetupWizard::applyHardwareConfiguration(UAVObjectManager* uavoMgr)
{
    HwSettings* hwSettings = HwSettings::GetInstance(uavoMgr);
    HwSettings::DataFields data = hwSettings->getData();
    switch(getControllerType())
    {
        case CONTROLLER_CC:
        case CONTROLLER_CC3D:
            // Reset all ports
            data.CC_RcvrPort = HwSettings::CC_RCVRPORT_DISABLED;
            data.CC_FlexiPort = HwSettings::CC_FLEXIPORT_DISABLED;
            data.CC_MainPort = HwSettings::CC_MAINPORT_DISABLED;
            switch(getInputType())
            {
                case INPUT_PWM:
                    data.CC_RcvrPort = HwSettings::CC_RCVRPORT_PWM;
                    break;
                case INPUT_PPM:
                    data.CC_RcvrPort = HwSettings::CC_RCVRPORT_PPM;
                    break;
                case INPUT_SBUS:
                    data.CC_MainPort = HwSettings::CC_MAINPORT_SBUS;
                    break;
                case INPUT_DSM:
                    // TODO: Handle all of the DSM types ?? Which is most common?
                    data.CC_MainPort = HwSettings::CC_MAINPORT_DSM2;
                    break;
            }
            break;
        case CONTROLLER_REVO:
            // TODO: Implement Revo settings
            break;
    }
    hwSettings->setData(data);
}

void SetupWizard::applyVehicleConfiguration(UAVObjectManager *uavoMgr)
{

    switch(getVehicleType())
    {
        case VEHICLE_MULTI:
        {
            resetVehicleConfig(uavoMgr);

            switch(getVehicleSubType())
            {
                case SetupWizard::MULTI_ROTOR_TRI_Y:
                    setupTriCopter(uavoMgr);
                    break;
                case SetupWizard::MULTI_ROTOR_QUAD_X:
                case SetupWizard::MULTI_ROTOR_QUAD_PLUS:
                    setupQuadCopter(uavoMgr);
                    break;
                case SetupWizard::MULTI_ROTOR_HEXA:
                case SetupWizard::MULTI_ROTOR_HEXA_COAX_Y:
                case SetupWizard::MULTI_ROTOR_HEXA_H:
                    setupHexaCopter(uavoMgr);
                    break;
                case SetupWizard::MULTI_ROTOR_OCTO:
                case SetupWizard::MULTI_ROTOR_OCTO_COAX_X:
                case SetupWizard::MULTI_ROTOR_OCTO_COAX_PLUS:
                case SetupWizard::MULTI_ROTOR_OCTO_V:
                    setupOctoCopter(uavoMgr);
                    break;
            }
            break;
        }
        case VEHICLE_FIXEDWING:
        case VEHICLE_HELI:
        case VEHICLE_SURFACE:
            // TODO: Implement settings for other vehicle types?
            break;
    }
}

void SetupWizard::applyOutputConfiguration(UAVObjectManager *uavoMgr)
{
    ActuatorSettings* actSettings = ActuatorSettings::GetInstance(uavoMgr);
    switch(getVehicleType())
    {
        case VEHICLE_MULTI:
        {
            ActuatorSettings::DataFields data = actSettings->getData();

            data.ChannelUpdateFreq[0] = DEFAULT_ESC_FREQUENCE;
            data.ChannelUpdateFreq[1] = DEFAULT_ESC_FREQUENCE;
            data.ChannelUpdateFreq[3] = DEFAULT_ESC_FREQUENCE;
            data.ChannelUpdateFreq[4] = DEFAULT_ESC_FREQUENCE;

            qint16 updateFrequence = DEFAULT_ESC_FREQUENCE;
            switch(getESCType())
            {
                case ESC_DEFAULT:
                    updateFrequence = DEFAULT_ESC_FREQUENCE;
                    break;
                case ESC_RAPID:
                    updateFrequence = RAPID_ESC_FREQUENCE;
                    break;
            }

            switch(getVehicleSubType())
            {
                case SetupWizard::MULTI_ROTOR_TRI_Y:
                    data.ChannelUpdateFreq[0] = updateFrequence;
                    break;
                case SetupWizard::MULTI_ROTOR_QUAD_X:
                case SetupWizard::MULTI_ROTOR_QUAD_PLUS:
                    data.ChannelUpdateFreq[0] = updateFrequence;
                    data.ChannelUpdateFreq[1] = updateFrequence;
                    break;
                case SetupWizard::MULTI_ROTOR_HEXA:
                case SetupWizard::MULTI_ROTOR_HEXA_COAX_Y:
                case SetupWizard::MULTI_ROTOR_HEXA_H:
                case SetupWizard::MULTI_ROTOR_OCTO:
                case SetupWizard::MULTI_ROTOR_OCTO_COAX_X:
                case SetupWizard::MULTI_ROTOR_OCTO_COAX_PLUS:
                case SetupWizard::MULTI_ROTOR_OCTO_V:
                    data.ChannelUpdateFreq[0] = updateFrequence;
                    data.ChannelUpdateFreq[1] = updateFrequence;
                    data.ChannelUpdateFreq[3] = updateFrequence;
                    data.ChannelUpdateFreq[4] = updateFrequence;
                    break;
            }
            actSettings->setData(data);
            break;
        }
        case VEHICLE_FIXEDWING:
        case VEHICLE_HELI:
        case VEHICLE_SURFACE:
            // TODO: Implement settings for other vehicle types?
            break;
    }
}

void SetupWizard::applyLevellingConfiguration(UAVObjectManager *uavoMgr)
{
    if(isLevellingPerformed())
    {
        accelGyroBias bias = getLevellingBias();
        AttitudeSettings::DataFields attitudeSettingsData = AttitudeSettings::GetInstance(uavoMgr)->getData();
        attitudeSettingsData.AccelBias[0] += bias.m_accelerometerXBias;
        attitudeSettingsData.AccelBias[1] += bias.m_accelerometerYBias;
        attitudeSettingsData.AccelBias[2] += bias.m_accelerometerZBias;
        attitudeSettingsData.GyroBias[0] = -bias.m_gyroXBias;
        attitudeSettingsData.GyroBias[1] = -bias.m_gyroYBias;
        attitudeSettingsData.GyroBias[2] = -bias.m_gyroZBias;
        AttitudeSettings::GetInstance(uavoMgr)->setData(attitudeSettingsData);
    }
}


void SetupWizard::resetVehicleConfig(UAVObjectManager *uavoMgr)
{
    // Reset all mixers
    MixerSettings* mSettings = MixerSettings::GetInstance(uavoMgr);

    QString mixerTypePattern = "Mixer%1Type";
    QString mixerVectorPattern = "Mixer%1Vector";
    for(int i = 1; i <= 10; i++) {
        UAVObjectField *field = mSettings->getField(mixerTypePattern.arg(i));
        Q_ASSERT(field);
        field->setValue(field->getOptions().at(0));

        field = mSettings->getField(mixerVectorPattern.arg(i));
        Q_ASSERT(field);
        for(int i = 0; i < field->getNumElements(); i++){
            field->setValue(0, i);
        }
    }
    mSettings->setData(mSettings->getData());

}


void SetupWizard::setupTriCopter(UAVObjectManager *uavoMgr)
{
    // Typical vehicle setup
    // 1. Setup and apply mixer
    // 2. Setup GUI data

    double mixer [8][3] = {
        {  0.5,  1,  0},
        {  0.5, -1,  0},
        { -1,    0,  0},
        {  0,    0,  0},
        {  0,    0,  0},
        {  0,    0,  0},
        {  0,    0,  0},
        {  0,    0,  0}
    };

}

void SetupWizard::setupQuadCopter(UAVObjectManager *uavoMgr)
{
}

void SetupWizard::setupHexaCopter(UAVObjectManager *uavoMgr)
{
}

void SetupWizard::setupOctoCopter(UAVObjectManager *uavoMgr)
{
}

void SetupWizard::exportConfiguration()
{
    applyConfiguration();
    // Call export configuration function...
}

void SetupWizard::writeConfiguration()
{
    applyConfiguration();
    // Call Save UAVOs to controller
}

void SetupWizard::createPages()
{
    setPage(PAGE_START, new StartPage(this));
    setPage(PAGE_CONTROLLER, new ControllerPage(this));
    setPage(PAGE_VEHICLES, new VehiclePage(this));
    setPage(PAGE_MULTI, new MultiPage(this));
    setPage(PAGE_FIXEDWING, new FixedWingPage(this));
    setPage(PAGE_HELI, new HeliPage(this));
    setPage(PAGE_SURFACE, new SurfacePage(this));
    setPage(PAGE_INPUT, new InputPage(this));
    setPage(PAGE_OUTPUT, new OutputPage(this));
    setPage(PAGE_LEVELLING, new LevellingPage(this));
    setPage(PAGE_SUMMARY, new SummaryPage(this));
    setPage(PAGE_FLASH, new FlashPage(this));
    setPage(PAGE_NOTYETIMPLEMENTED, new NotYetImplementedPage(this));
    setPage(PAGE_END, new EndPage(this));

    setStartId(PAGE_START);
}
