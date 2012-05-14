
/**
 ******************************************************************************
 *
 * @file       guiconfigdata.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief bit storage of config ui settings
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
#include "guiconfigdata.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"

#include "systemsettings.h"

#include <QDebug>

GUIConfigDataManager::GUIConfigDataManager()
{
}

GUIConfigDataManager::~GUIConfigDataManager()
{
   // Do nothing
}

GUIConfigDataUnion GUIConfigDataManager::GetConfigData() {

    int i;
    GUIConfigDataUnion configData;

    // get an instance of systemsettings
    SystemSettings * systemSettings = SystemSettings::GetInstance(getObjectManager());
    Q_ASSERT(systemSettings);
    SystemSettings::DataFields systemSettingsData = systemSettings->getData();

    // copy systemsettings -> local configData
    for(i = 0; i < (int)(SystemSettings::GUICONFIGDATA_NUMELEM); i++)
        configData.UAVObject[i]=systemSettingsData.GUIConfigData[i];

    // sanity check
    Q_ASSERT(SystemSettings::GUICONFIGDATA_NUMELEM ==
             (sizeof(configData.UAVObject) / sizeof(configData.UAVObject[0])));

    return configData;
}

void GUIConfigDataManager::SetConfigData(GUIConfigDataUnion configData) {

    int i;

    // sanity check
    Q_ASSERT(SystemSettings::GUICONFIGDATA_NUMELEM ==
             (sizeof(configData.UAVObject) / sizeof(configData.UAVObject[0])));

    // get an instance of systemsettings
    SystemSettings * systemSettings = SystemSettings::GetInstance(getObjectManager());
    Q_ASSERT(systemSettings);
    SystemSettings::DataFields systemSettingsData = systemSettings->getData();

    // copy parameter configData -> systemsettings
    for (i = 0; i < (int)(SystemSettings::GUICONFIGDATA_NUMELEM); i++)
        systemSettingsData.GUIConfigData[i] = configData.UAVObject[i];

    systemSettings->setData(systemSettingsData);
    systemSettings->updated();
}

void GUIConfigDataManager::ResetActuators()
{
    // get the gui config data
    GUIConfigDataUnion configData = GetConfigData();

    // reset the actuators by airframe type
    ResetActuators(&configData);

    // set the gui config data
    SetConfigData(configData);

}

void GUIConfigDataManager::ResetActuators(GUIConfigDataUnion* configData)
{
    // get systemsettings for airframe type
    SystemSettings * systemSettings = SystemSettings::GetInstance(getObjectManager());
    Q_ASSERT(systemSettings);
    SystemSettings::DataFields systemSettingsData = systemSettings->getData();

    switch (systemSettingsData.AirframeType)
    {
        // fixed wing
        case SystemSettings::AIRFRAMETYPE_FIXEDWING:
        case SystemSettings::AIRFRAMETYPE_FIXEDWINGELEVON:
        case SystemSettings::AIRFRAMETYPE_FIXEDWINGVTAIL:
        {
            configData->fixed.FixedWingPitch1 = 0;
            configData->fixed.FixedWingPitch2 = 0;
            configData->fixed.FixedWingRoll1 = 0;
            configData->fixed.FixedWingRoll2 = 0;
            configData->fixed.FixedWingYaw1 = 0;
            configData->fixed.FixedWingYaw2 = 0;
            configData->fixed.FixedWingThrottle = 0;
        }
        break;

        // helicp
        case SystemSettings::AIRFRAMETYPE_HELICP:
        {
            configData->heli.Throttle = 0;
            configData->heli.Tail = 0;
            configData->heli.ServoIndexW = 0;
            configData->heli.ServoIndexX = 0;
            configData->heli.ServoIndexY = 0;
            configData->heli.ServoIndexZ = 0;
        }
        break;

        //multirotor
        case SystemSettings::AIRFRAMETYPE_VTOL:
        case SystemSettings::AIRFRAMETYPE_TRI:
        case SystemSettings::AIRFRAMETYPE_QUADX:
        case SystemSettings::AIRFRAMETYPE_QUADP:
        case SystemSettings::AIRFRAMETYPE_OCTOV:
        case SystemSettings::AIRFRAMETYPE_OCTOCOAXX:
        case SystemSettings::AIRFRAMETYPE_OCTOCOAXP:
        case SystemSettings::AIRFRAMETYPE_OCTO:
        case SystemSettings::AIRFRAMETYPE_HEXAX:
        case SystemSettings::AIRFRAMETYPE_HEXACOAX:
        case SystemSettings::AIRFRAMETYPE_HEXA:
        {
            configData->multi.VTOLMotorN = 0;
            configData->multi.VTOLMotorNE = 0;
            configData->multi.VTOLMotorE = 0;
            configData->multi.VTOLMotorSE = 0;
            configData->multi.VTOLMotorS = 0;
            configData->multi.VTOLMotorSW = 0;
            configData->multi.VTOLMotorW = 0;
            configData->multi.VTOLMotorNW = 0;
            configData->multi.TRIYaw = 0;
        }
        break;

        // ground
        case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLECAR:
        case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLEDIFFERENTIAL:
        case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLEMOTORCYCLE:
        {
            configData->ground.GroundVehicleSteering1 = 0;
            configData->ground.GroundVehicleSteering2 = 0;
            configData->ground.GroundVehicleThrottle1 = 0;
            configData->ground.GroundVehicleThrottle2 = 0;
        }
        break;

    }
}

QStringList GUIConfigDataManager::getChannelDescriptions()
{
    int i;
    QStringList channelDesc;

    // init a channel_numelem list of channel desc defaults
    for (i=0; i < (int)(GUIConfigDataManager::CHANNEL_NUMELEM); i++)
    {
        channelDesc.append(QString("-"));
    }

    // get the gui config data
    GUIConfigDataUnion configData = GetConfigData();

    // get systemsettings for airframe type
    SystemSettings * systemSettings = SystemSettings::GetInstance(getObjectManager());
    Q_ASSERT(systemSettings);
    SystemSettings::DataFields systemSettingsData = systemSettings->getData();

    switch (systemSettingsData.AirframeType)
    {
        // fixed wing
        case SystemSettings::AIRFRAMETYPE_FIXEDWING:
        case SystemSettings::AIRFRAMETYPE_FIXEDWINGELEVON:
        case SystemSettings::AIRFRAMETYPE_FIXEDWINGVTAIL:
        {
            if (configData.fixed.FixedWingPitch1 > 0)
                channelDesc[configData.fixed.FixedWingPitch1-1] = QString("FixedWingPitch1");
            if (configData.fixed.FixedWingPitch2 > 0)
                channelDesc[configData.fixed.FixedWingPitch2-1] = QString("FixedWingPitch2");
            if (configData.fixed.FixedWingRoll1 > 0)
                channelDesc[configData.fixed.FixedWingRoll1-1] = QString("FixedWingRoll1");
            if (configData.fixed.FixedWingRoll2 > 0)
                channelDesc[configData.fixed.FixedWingRoll2-1] = QString("FixedWingRoll2");
            if (configData.fixed.FixedWingYaw1 > 0)
                channelDesc[configData.fixed.FixedWingYaw1-1] = QString("FixedWingYaw1");
            if (configData.fixed.FixedWingYaw2 > 0)
                channelDesc[configData.fixed.FixedWingYaw2-1] = QString("FixedWingYaw2");
            if (configData.fixed.FixedWingThrottle > 0)
                channelDesc[configData.fixed.FixedWingThrottle-1] = QString("FixedWingThrottle");

            return channelDesc;
        }
        break;

        // helicp
        case SystemSettings::AIRFRAMETYPE_HELICP:
        {
            channelDesc[configData.heli.Throttle] = QString("Throttle");
            channelDesc[configData.heli.Tail] = QString("Tail");

            switch(configData.heli.FirstServoIndex)
            {
                case 0:  //front
                    channelDesc[configData.heli.ServoIndexW] = QString("Elevator");
                    channelDesc[configData.heli.ServoIndexX] = QString("Roll1");
                    channelDesc[configData.heli.ServoIndexY] = QString("Roll2");
                break;

                case 1:  //right
                    channelDesc[configData.heli.ServoIndexW] = QString("ServoW");
                    channelDesc[configData.heli.ServoIndexX] = QString("ServoX");
                    channelDesc[configData.heli.ServoIndexY] = QString("ServoY");
                break;

                case 2:  //rear
                    channelDesc[configData.heli.ServoIndexW] = QString("Elevator");
                    channelDesc[configData.heli.ServoIndexX] = QString("Roll1");
                    channelDesc[configData.heli.ServoIndexY] = QString("Roll2");
                break;

                case 3:  //left
                    channelDesc[configData.heli.ServoIndexW] = QString("ServoW");
                    channelDesc[configData.heli.ServoIndexX] = QString("ServoX");
                    channelDesc[configData.heli.ServoIndexY] = QString("ServoY");
                break;

            }
            if (configData.heli.ServoIndexZ < 8)
                channelDesc[configData.heli.ServoIndexZ] = QString("ServoZ");

            return channelDesc;
        }
        break;

        //multirotor
        case SystemSettings::AIRFRAMETYPE_VTOL:
        case SystemSettings::AIRFRAMETYPE_TRI:
        case SystemSettings::AIRFRAMETYPE_QUADX:
        case SystemSettings::AIRFRAMETYPE_QUADP:
        case SystemSettings::AIRFRAMETYPE_OCTOV:
        case SystemSettings::AIRFRAMETYPE_OCTOCOAXX:
        case SystemSettings::AIRFRAMETYPE_OCTOCOAXP:
        case SystemSettings::AIRFRAMETYPE_OCTO:
        case SystemSettings::AIRFRAMETYPE_HEXAX:
        case SystemSettings::AIRFRAMETYPE_HEXACOAX:
        case SystemSettings::AIRFRAMETYPE_HEXA:
        {
            multiGUISettingsStruct multi = configData.multi;

            if (multi.VTOLMotorN > 0 && multi.VTOLMotorN < GUIConfigDataManager::CHANNEL_NUMELEM)
                channelDesc[multi.VTOLMotorN-1] = QString("VTOLMotorN");
            if (multi.VTOLMotorNE > 0 && multi.VTOLMotorNE < GUIConfigDataManager::CHANNEL_NUMELEM)
                channelDesc[multi.VTOLMotorNE-1] = QString("VTOLMotorNE");
            if (multi.VTOLMotorNW > 0 && multi.VTOLMotorNW < GUIConfigDataManager::CHANNEL_NUMELEM)
                channelDesc[multi.VTOLMotorNW-1] = QString("VTOLMotorNW");
            if (multi.VTOLMotorS > 0 && multi.VTOLMotorS < GUIConfigDataManager::CHANNEL_NUMELEM)
                channelDesc[multi.VTOLMotorS-1] = QString("VTOLMotorS");
            if (multi.VTOLMotorSE > 0 && multi.VTOLMotorSE < GUIConfigDataManager::CHANNEL_NUMELEM)
                channelDesc[multi.VTOLMotorSE-1] = QString("VTOLMotorSE");
            if (multi.VTOLMotorSW > 0 && multi.VTOLMotorSW < GUIConfigDataManager::CHANNEL_NUMELEM)
                channelDesc[multi.VTOLMotorSW-1] = QString("VTOLMotorSW");
            if (multi.VTOLMotorW > 0 && multi.VTOLMotorW < GUIConfigDataManager::CHANNEL_NUMELEM)
                channelDesc[multi.VTOLMotorW-1] = QString("VTOLMotorW");
            if (multi.VTOLMotorE > 0 && multi.VTOLMotorE < GUIConfigDataManager::CHANNEL_NUMELEM)
                channelDesc[multi.VTOLMotorE-1] = QString("VTOLMotorE");

            return channelDesc;
        }
        break;

        // ground
        case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLECAR:
        case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLEDIFFERENTIAL:
        case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLEMOTORCYCLE:
        {
            if (configData.ground.GroundVehicleSteering1 > 0)
                channelDesc[configData.ground.GroundVehicleSteering1-1] = QString("GroundSteering1");
            if (configData.ground.GroundVehicleSteering2 > 0)
                channelDesc[configData.ground.GroundVehicleSteering2-1] = QString("GroundSteering2");
            if (configData.ground.GroundVehicleThrottle1 > 0)
                channelDesc[configData.ground.GroundVehicleThrottle1-1] = QString("GroundThrottle1");
            if (configData.ground.GroundVehicleThrottle2 > 0)
                channelDesc[configData.ground.GroundVehicleThrottle2-1] = QString("GroundThrottle2");

            return channelDesc;
        }
        break;
    }

    return channelDesc;
}
/**
 * Util function to get a pointer to the object manager
 * @return pointer to the UAVObjectManager
 */
UAVObjectManager* GUIConfigDataManager::getObjectManager() {
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr);
    return objMngr;
}
