
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
            channelDesc[configData.multi.VTOLMotorN] = QString("VTOLMotorN");
            channelDesc[configData.multi.VTOLMotorNE] = QString("VTOLMotorNE");
            channelDesc[configData.multi.VTOLMotorNW] = QString("VTOLMotorNW");
            channelDesc[configData.multi.VTOLMotorS] = QString("VTOLMotorS");
            channelDesc[configData.multi.VTOLMotorSE] = QString("VTOLMotorSE");
            channelDesc[configData.multi.VTOLMotorSW] = QString("VTOLMotorSW");
            channelDesc[configData.multi.VTOLMotorW] = QString("VTOLMotorW");
            channelDesc[configData.multi.VTOLMotorE] = QString("VTOLMotorE");
            return channelDesc;
        }
        break;

        // ground

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
