/**
 ******************************************************************************
 *
 * @file       vehicleconfigurationhelper.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup VehicleConfigurationHelper
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

#include "vehicleconfigurationhelper.h"
#include "extensionsystem/pluginmanager.h"
#include "hwsettings.h"
#include "actuatorsettings.h"
#include "attitudesettings.h"
#include "mixersettings.h"
#include "systemsettings.h"

VehicleConfigurationHelper::VehicleConfigurationHelper(VehicleConfigurationSource *configSource)
    : m_configSource(configSource), m_uavoManager(0),
      m_transactionOK(false), m_transactionTimeout(false), m_currentTransactionObjectID(-1),
      m_progress(0)
{
    Q_ASSERT(m_configSource);
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_uavoManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(m_uavoManager);
}

bool VehicleConfigurationHelper::setupVehicle()
{
    m_progress = 0;
    clearModifiedObjects();
    resetVehicleConfig();
    resetGUIData();
    if(!saveChangesToController())
    {
        return false;
    }

    applyHardwareConfiguration();
    applyVehicleConfiguration();
    applyOutputConfiguration();
    applyFlighModeConfiguration();
    applyLevellingConfiguration();

    bool result = saveChangesToController();
    if(result) {
        emit saveProgress(PROGRESS_STEPS, ++m_progress, tr("Done!"));
    }
    else {
        emit saveProgress(PROGRESS_STEPS, ++m_progress, tr("Failed!"));
    }
    return result;
}

void VehicleConfigurationHelper::addModifiedObject(UAVDataObject *object, QString description)
{
    m_modifiedObjects << new QPair<UAVDataObject*, QString>(object, description);
}

void VehicleConfigurationHelper::clearModifiedObjects()
{
    for(int i = 0; i < m_modifiedObjects.count(); i++) {
        QPair<UAVDataObject*, QString> *pair = m_modifiedObjects.at(i);
        delete pair;
    }
    m_modifiedObjects.clear();
}

void VehicleConfigurationHelper::applyHardwareConfiguration()
{
    HwSettings* hwSettings = HwSettings::GetInstance(m_uavoManager);
    HwSettings::DataFields data = hwSettings->getData();
    switch(m_configSource->getControllerType())
    {
        case VehicleConfigurationSource::CONTROLLER_CC:
        case VehicleConfigurationSource::CONTROLLER_CC3D:
            // Reset all ports
            data.CC_RcvrPort = HwSettings::CC_RCVRPORT_DISABLED;
            data.CC_FlexiPort = HwSettings::CC_FLEXIPORT_DISABLED;
            data.CC_MainPort = HwSettings::CC_MAINPORT_DISABLED;
            switch(m_configSource->getInputType())
            {
                case VehicleConfigurationSource::INPUT_PWM:
                    data.CC_RcvrPort = HwSettings::CC_RCVRPORT_PWM;
                    break;
                case VehicleConfigurationSource::INPUT_PPM:
                    data.CC_RcvrPort = HwSettings::CC_RCVRPORT_PPM;
                    break;
                case VehicleConfigurationSource::INPUT_SBUS:
                    data.CC_MainPort = HwSettings::CC_MAINPORT_SBUS;
                    break;
                case VehicleConfigurationSource::INPUT_DSM:
                    // TODO: Handle all of the DSM types ?? Which is most common?
                    data.CC_MainPort = HwSettings::CC_MAINPORT_DSM2;
                    break;
                default:
                    break;
            }
            break;
        case VehicleConfigurationSource::CONTROLLER_REVO:
            // TODO: Implement Revo settings
            break;
        default:
            break;
    }
    //hwSettings->setData(data);
    addModifiedObject(hwSettings, tr("Writing hardware settings"));
}

void VehicleConfigurationHelper::applyVehicleConfiguration()
{

    switch(m_configSource->getVehicleType())
    {
        case VehicleConfigurationSource::VEHICLE_MULTI:
        {
            switch(m_configSource->getVehicleSubType())
            {
                case VehicleConfigurationSource::MULTI_ROTOR_TRI_Y:
                    setupTriCopter();
                    break;
                case VehicleConfigurationSource::MULTI_ROTOR_QUAD_X:
                case VehicleConfigurationSource::MULTI_ROTOR_QUAD_PLUS:
                    setupQuadCopter();
                    break;
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA:
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA_COAX_Y:
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA_H:
                    setupHexaCopter();
                    break;
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_X:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_PLUS:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_V:
                    setupOctoCopter();
                    break;
                default:
                    break;
            }
            break;
        }
        case VehicleConfigurationSource::VEHICLE_FIXEDWING:
        case VehicleConfigurationSource::VEHICLE_HELI:
        case VehicleConfigurationSource::VEHICLE_SURFACE:
            // TODO: Implement settings for other vehicle types?
            break;
        default:
            break;
    }
}

void VehicleConfigurationHelper::applyOutputConfiguration()
{
    ActuatorSettings* actSettings = ActuatorSettings::GetInstance(m_uavoManager);
    switch(m_configSource->getVehicleType())
    {
        case VehicleConfigurationSource::VEHICLE_MULTI:
        {
            ActuatorSettings::DataFields data = actSettings->getData();

            data.ChannelUpdateFreq[0] = LEGACY_ESC_FREQUENCE;
            data.ChannelUpdateFreq[1] = LEGACY_ESC_FREQUENCE;
            data.ChannelUpdateFreq[3] = LEGACY_ESC_FREQUENCE;
            data.ChannelUpdateFreq[4] = LEGACY_ESC_FREQUENCE;

            qint16 updateFrequence = LEGACY_ESC_FREQUENCE;
            switch(m_configSource->getESCType())
            {
                case VehicleConfigurationSource::ESC_LEGACY:
                    updateFrequence = LEGACY_ESC_FREQUENCE;
                    break;
                case VehicleConfigurationSource::ESC_RAPID:
                    updateFrequence = RAPID_ESC_FREQUENCE;
                    break;
                default:
                    break;
            }

            switch(m_configSource->getVehicleSubType())
            {
                case VehicleConfigurationSource::MULTI_ROTOR_TRI_Y:
                    data.ChannelUpdateFreq[0] = updateFrequence;
                    break;
                case VehicleConfigurationSource::MULTI_ROTOR_QUAD_X:
                case VehicleConfigurationSource::MULTI_ROTOR_QUAD_PLUS:
                    data.ChannelUpdateFreq[0] = updateFrequence;
                    data.ChannelUpdateFreq[1] = updateFrequence;
                    break;
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA:
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA_COAX_Y:
                case VehicleConfigurationSource::MULTI_ROTOR_HEXA_H:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_X:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_COAX_PLUS:
                case VehicleConfigurationSource::MULTI_ROTOR_OCTO_V:
                    data.ChannelUpdateFreq[0] = updateFrequence;
                    data.ChannelUpdateFreq[1] = updateFrequence;
                    data.ChannelUpdateFreq[3] = updateFrequence;
                    data.ChannelUpdateFreq[4] = updateFrequence;
                    break;
                default:
                    break;
            }
            //actSettings->setData(data);
            addModifiedObject(actSettings, tr("Writing output rate settings"));
            break;
        }
        case VehicleConfigurationSource::VEHICLE_FIXEDWING:
        case VehicleConfigurationSource::VEHICLE_HELI:
        case VehicleConfigurationSource::VEHICLE_SURFACE:
            // TODO: Implement settings for other vehicle types?
            break;
        default:
            break;
    }
}

void VehicleConfigurationHelper::applyFlighModeConfiguration()
{
}

void VehicleConfigurationHelper::applyLevellingConfiguration()
{
    if(m_configSource->isLevellingPerformed())
    {
        accelGyroBias bias = m_configSource->getLevellingBias();
        AttitudeSettings* attitudeSettings = AttitudeSettings::GetInstance(m_uavoManager);
        Q_ASSERT(attitudeSettings);
        AttitudeSettings::DataFields data = attitudeSettings->getData();

        data.AccelBias[0] += bias.m_accelerometerXBias;
        data.AccelBias[1] += bias.m_accelerometerYBias;
        data.AccelBias[2] += bias.m_accelerometerZBias;
        data.GyroBias[0] = -bias.m_gyroXBias;
        data.GyroBias[1] = -bias.m_gyroYBias;
        data.GyroBias[2] = -bias.m_gyroZBias;

        //AttitudeSettings::GetInstance(m_uavoManager)->setData(data);
        addModifiedObject(attitudeSettings, tr("Writing levelling bias settings"));
    }
}

void VehicleConfigurationHelper::applyMixerConfiguration(mixerSettings mixer)
{
    // Set all mixer data
    MixerSettings* mSettings = MixerSettings::GetInstance(m_uavoManager);

    // Set Mixer types and values
    QString mixerTypePattern = "Mixer%1Type";
    QString mixerVectorPattern = "Mixer%1Vector";
    for(int i = 0; i < 10; i++) {
        UAVObjectField *field = mSettings->getField(mixerTypePattern.arg(i + 1));
        Q_ASSERT(field);
        field->setValue(field->getOptions().at(mixer.channels[i].type));

        field = mSettings->getField(mixerVectorPattern.arg(i + 1));
        Q_ASSERT(field);
        field->setValue((mixer.channels[i].throttle1 * 127) / 100, 0);
        field->setValue((mixer.channels[i].throttle2 * 127) / 100, 1);
        field->setValue((mixer.channels[i].roll * 127) / 100, 2);
        field->setValue((mixer.channels[i].pitch * 127) / 100, 3);
        field->setValue((mixer.channels[i].yaw *127) / 100, 4);
    }

    // Apply updates
    //mSettings->setData(mSettings->getData());
    addModifiedObject(mSettings, tr("Writing mixer settings"));

}

void VehicleConfigurationHelper::applyMultiGUISettings(SystemSettings::AirframeTypeOptions airframe, GUIConfigDataUnion guiConfig)
{
    SystemSettings * sSettings = SystemSettings::GetInstance(m_uavoManager);
    Q_ASSERT(sSettings);
    SystemSettings::DataFields data = sSettings->getData();
    data.AirframeType = airframe;

    for (int i = 0; i < (int)(SystemSettings::GUICONFIGDATA_NUMELEM); i++) {
        data.GUIConfigData[i] = guiConfig.UAVObject[i];
    }

    sSettings->setData(data);
    addModifiedObject(sSettings, tr("Writing vehicle settings"));
}

bool VehicleConfigurationHelper::saveChangesToController()
{
    qDebug() << "Saving modified objects to controller. " << m_modifiedObjects.count() << " objects in found.";
    const int OUTER_TIMEOUT = 3000 * 20; // 10 seconds timeout for saving all objects
    const int INNER_TIMEOUT = 2000; // 1 second timeout on every save attempt

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    UAVObjectUtilManager* utilMngr = pm->getObject<UAVObjectUtilManager>();
    Q_ASSERT(utilMngr);

    QTimer outerTimeoutTimer;
    outerTimeoutTimer.setSingleShot(true);

    QTimer innerTimeoutTimer;
    innerTimeoutTimer.setSingleShot(true);

    connect(utilMngr, SIGNAL(saveCompleted(int ,bool)), this, SLOT(uAVOTransactionCompleted(int, bool)));
    connect(&innerTimeoutTimer, SIGNAL(timeout()), &m_eventLoop, SLOT(quit()));
    connect(&outerTimeoutTimer, SIGNAL(timeout()), this, SLOT(saveChangesTimeout()));

    outerTimeoutTimer.start(OUTER_TIMEOUT);
    for(int i = 0; i < m_modifiedObjects.count(); i++) {
        QPair<UAVDataObject*, QString> *objPair = m_modifiedObjects.at(i);
        m_transactionOK = false;
        UAVDataObject* obj = objPair->first;
        QString objDescription = objPair->second;
        if(UAVObject::GetGcsAccess(obj->getMetadata()) != UAVObject::ACCESS_READONLY && obj->isSettings()) {

            emit saveProgress(PROGRESS_STEPS, ++m_progress, objDescription);

            m_currentTransactionObjectID = obj->getObjID();

            connect(obj, SIGNAL(transactionCompleted(UAVObject* ,bool)), this, SLOT(uAVOTransactionCompleted(UAVObject*, bool)));
            while(!m_transactionOK && !m_transactionTimeout) {
                // Allow the transaction to take some time
                innerTimeoutTimer.start(INNER_TIMEOUT);

                // Set object updated
                obj->updated();
                if(!m_transactionOK) {
                    m_eventLoop.exec();
                }
                innerTimeoutTimer.stop();
            }
            disconnect(obj, SIGNAL(transactionCompleted(UAVObject* ,bool)), this, SLOT(uAVOTransactionCompleted(UAVObject*, bool)));
            if(m_transactionOK) {
                qDebug() << "Object " << obj->getName() << " was successfully updated.";
                m_transactionOK = false;
                m_currentTransactionObjectID = obj->getObjID();
                // Try to save until success or timeout
                while(!m_transactionOK && !m_transactionTimeout) {
                    // Allow the transaction to take some time
                    innerTimeoutTimer.start(INNER_TIMEOUT);

                    // Persist object in controller
                    utilMngr->saveObjectToSD(obj);
                    if(!m_transactionOK) {
                        m_eventLoop.exec();
                    }
                    innerTimeoutTimer.stop();
                }
                m_currentTransactionObjectID = -1;
            }

            if(!m_transactionOK) {
                qDebug() << "Transaction timed out when trying to save: " << obj->getName();
            }
            else {
                qDebug() << "Object " << obj->getName() << " was successfully saved.";
            }
        }
        else {
            qDebug() << "Trying to save a UAVDataObject that is read only or is not a settings object.";
        }
        if(m_transactionTimeout) {
            qDebug() << "Transaction timed out when trying to save " << m_modifiedObjects.count() << " objects.";
            break;
        }
    }

    outerTimeoutTimer.stop();
    disconnect(&outerTimeoutTimer, SIGNAL(timeout()), this, SLOT(saveChangesTimeout()));
    disconnect(&innerTimeoutTimer, SIGNAL(timeout()), &m_eventLoop, SLOT(quit()));
    disconnect(utilMngr, SIGNAL(saveCompleted(int, bool)), this, SLOT(uAVOTransactionCompleted(int, bool)));

    qDebug() << "Finished saving modified objects to controller. Success = " << m_transactionOK;

    clearModifiedObjects();
    return m_transactionOK;
}

void VehicleConfigurationHelper::uAVOTransactionCompleted(int oid, bool success)
{
    if(oid == m_currentTransactionObjectID)
    {
        m_transactionOK = success;
        m_eventLoop.quit();
    }
}

void VehicleConfigurationHelper::uAVOTransactionCompleted(UAVObject *object, bool success)
{
    if(object) {
        uAVOTransactionCompleted(object->getObjID(), success);
    }
}

void VehicleConfigurationHelper::saveChangesTimeout()
{
    m_transactionOK = false;
    m_transactionTimeout = true;
    m_eventLoop.quit();
}

void VehicleConfigurationHelper::resetVehicleConfig()
{
    // Reset all vehicle data
    MixerSettings* mSettings = MixerSettings::GetInstance(m_uavoManager);

    // Reset feed forward, accel times etc
    mSettings->setFeedForward(0.0f);
    mSettings->setMaxAccel(1000.0f);
    mSettings->setAccelTime(0.0f);
    mSettings->setDecelTime(0.0f);

    // Reset throttle curves
    QString throttlePattern = "ThrottleCurve%1";
    for(int i = 1; i <= 2; i++) {
        UAVObjectField *field = mSettings->getField(throttlePattern.arg(i));
        Q_ASSERT(field);
        for(quint32 i = 0; i < field->getNumElements(); i++){
            field->setValue(i * ( 1.0f / (field->getNumElements() - 1)), i);
        }
    }

    // Reset Mixer types and values
    QString mixerTypePattern = "Mixer%1Type";
    QString mixerVectorPattern = "Mixer%1Vector";
    for(int i = 1; i <= 10; i++) {
        UAVObjectField *field = mSettings->getField(mixerTypePattern.arg(i));
        Q_ASSERT(field);
        field->setValue(field->getOptions().at(0));

        field = mSettings->getField(mixerVectorPattern.arg(i));
        Q_ASSERT(field);
        for(quint32 i = 0; i < field->getNumElements(); i++){
            field->setValue(0, i);
        }
    }

    // Apply updates
    //mSettings->setData(mSettings->getData());
    addModifiedObject(mSettings, tr("Preparing mixer settings"));
}

void VehicleConfigurationHelper::resetGUIData()
{
    SystemSettings * sSettings = SystemSettings::GetInstance(m_uavoManager);
    Q_ASSERT(sSettings);
    SystemSettings::DataFields data = sSettings->getData();
    data.AirframeType = SystemSettings::AIRFRAMETYPE_CUSTOM;
    for(quint32 i = 0; i < SystemSettings::GUICONFIGDATA_NUMELEM; i++) {
        data.GUIConfigData[i] = 0;
    }
    //sSettings->setData(data);
    addModifiedObject(sSettings, tr("Preparing vehicle settings"));
}


void VehicleConfigurationHelper::setupTriCopter()
{
    // Typical vehicle setup
    // 1. Setup and apply mixer
    // 2. Setup GUI data

    mixerSettings mixer;
    mixer.channels[0].type = MIXER_TYPE_MOTOR;
    mixer.channels[0].throttle1 = 100;
    mixer.channels[0].throttle2 = 0;
    mixer.channels[0].roll = 100;
    mixer.channels[0].pitch = 50;
    mixer.channels[0].yaw = 0;

    mixer.channels[1].type = MIXER_TYPE_MOTOR;
    mixer.channels[1].throttle1 = 100;
    mixer.channels[1].throttle2 = 0;
    mixer.channels[1].roll = -100;
    mixer.channels[1].pitch = 50;
    mixer.channels[1].yaw = 0;

    mixer.channels[2].type = MIXER_TYPE_MOTOR;
    mixer.channels[2].throttle1 = 100;
    mixer.channels[2].throttle2 = 0;
    mixer.channels[2].roll = 0;
    mixer.channels[2].pitch = -100;
    mixer.channels[2].yaw = 0;

    mixer.channels[3].type = MIXER_TYPE_SERVO;
    mixer.channels[3].throttle1 = 0;
    mixer.channels[3].throttle2 = 0;
    mixer.channels[3].roll = 0;
    mixer.channels[3].pitch = 0;
    mixer.channels[3].yaw = 100;

    applyMixerConfiguration(mixer);

    GUIConfigDataUnion guiSettings = getGUIConfigData();

    guiSettings.multi.VTOLMotorNW = 1;
    guiSettings.multi.VTOLMotorNE = 2;
    guiSettings.multi.VTOLMotorS = 3;
    guiSettings.multi.TRIYaw = 4;

    applyMultiGUISettings(SystemSettings::AIRFRAMETYPE_TRI, guiSettings);
}

GUIConfigDataUnion VehicleConfigurationHelper::getGUIConfigData()
{
    GUIConfigDataUnion configData;

    SystemSettings * systemSettings = SystemSettings::GetInstance(m_uavoManager);
    Q_ASSERT(systemSettings);
    SystemSettings::DataFields systemSettingsData = systemSettings->getData();

    for(int i = 0; i < (int)(SystemSettings::GUICONFIGDATA_NUMELEM); i++) {
        configData.UAVObject[i] = 0; //systemSettingsData.GUIConfigData[i];
    }

    return configData;
}

void VehicleConfigurationHelper::setupQuadCopter()
{
    mixerSettings mixer;
    GUIConfigDataUnion guiSettings = getGUIConfigData();
    SystemSettings::AirframeTypeOptions frame;

    switch(m_configSource->getVehicleSubType())
    {
        case VehicleConfigurationSource::MULTI_ROTOR_QUAD_PLUS: {
            frame = SystemSettings::AIRFRAMETYPE_QUADP;
            mixer.channels[0].type = MIXER_TYPE_MOTOR;
            mixer.channels[0].throttle1 = 100;
            mixer.channels[0].throttle2 = 0;
            mixer.channels[0].roll = 0;
            mixer.channels[0].pitch = 100;
            mixer.channels[0].yaw = -50;

            mixer.channels[1].type = MIXER_TYPE_MOTOR;
            mixer.channels[1].throttle1 = 100;
            mixer.channels[1].throttle2 = 0;
            mixer.channels[1].roll = -100;
            mixer.channels[1].pitch = 0;
            mixer.channels[1].yaw = 50;

            mixer.channels[2].type = MIXER_TYPE_MOTOR;
            mixer.channels[2].throttle1 = 100;
            mixer.channels[2].throttle2 = 0;
            mixer.channels[2].roll = 0;
            mixer.channels[2].pitch = -100;
            mixer.channels[2].yaw = -50;

            mixer.channels[3].type = MIXER_TYPE_MOTOR;
            mixer.channels[3].throttle1 = 100;
            mixer.channels[3].throttle2 = 0;
            mixer.channels[3].roll = 100;
            mixer.channels[3].pitch = 0;
            mixer.channels[3].yaw = 50;

            guiSettings.multi.VTOLMotorN = 1;
            guiSettings.multi.VTOLMotorE = 2;
            guiSettings.multi.VTOLMotorS = 3;
            guiSettings.multi.VTOLMotorW = 4;

            break;
        }
        case VehicleConfigurationSource::MULTI_ROTOR_QUAD_X: {
            frame = SystemSettings::AIRFRAMETYPE_QUADX;
            mixer.channels[0].type = MIXER_TYPE_MOTOR;
            mixer.channels[0].throttle1 = 100;
            mixer.channels[0].throttle2 = 0;
            mixer.channels[0].roll = 50;
            mixer.channels[0].pitch = 50;
            mixer.channels[0].yaw = -50;

            mixer.channels[1].type = MIXER_TYPE_MOTOR;
            mixer.channels[1].throttle1 = 100;
            mixer.channels[1].throttle2 = 0;
            mixer.channels[1].roll = -50;
            mixer.channels[1].pitch = 50;
            mixer.channels[1].yaw = 50;

            mixer.channels[2].type = MIXER_TYPE_MOTOR;
            mixer.channels[2].throttle1 = 100;
            mixer.channels[2].throttle2 = 0;
            mixer.channels[2].roll = -50;
            mixer.channels[2].pitch = -50;
            mixer.channels[2].yaw = -50;

            mixer.channels[3].type = MIXER_TYPE_MOTOR;
            mixer.channels[3].throttle1 = 100;
            mixer.channels[3].throttle2 = 0;
            mixer.channels[3].roll = 50;
            mixer.channels[3].pitch = -50;
            mixer.channels[3].yaw = 50;

            guiSettings.multi.VTOLMotorNW = 1;
            guiSettings.multi.VTOLMotorNE = 2;
            guiSettings.multi.VTOLMotorSW = 3;
            guiSettings.multi.VTOLMotorSE = 4;

            break;
        }
        default:
            break;
    }
    applyMixerConfiguration(mixer);
    applyMultiGUISettings(frame, guiSettings);

}

void VehicleConfigurationHelper::setupHexaCopter()
{
}

void VehicleConfigurationHelper::setupOctoCopter()
{
}
