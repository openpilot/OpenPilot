/**
 ******************************************************************************
 *
 * @file       vehicleconfig.h
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
#ifndef GUIVEHICLECONFIG_H
#define GUIVEHICLECONFIG_H

#include "../uavobjectwidgetutils/configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"


typedef struct {
    uint VTOLMotorN:4;
    uint VTOLMotorS:4;
    uint VTOLMotorE:4;
    uint VTOLMotorW:4;
    uint VTOLMotorNW:4;
    uint VTOLMotorNE:4;
    uint VTOLMotorSW:4;
    uint VTOLMotorSE:4;         //32bits
    uint TRIYaw:4;
    quint32 padding:28;         //64bits
    quint32 padding1;
    quint32 padding2;       //128bits
} __attribute__((packed))  multiGUISettingsStruct;

typedef struct {
    uint SwashplateType:3;
    uint FirstServoIndex:2;
    uint CorrectionAngle:9;
    uint ccpmCollectivePassthroughState:1;
    uint ccpmLinkCyclicState:1;
    uint ccpmLinkRollState:1;
    uint SliderValue0:7;
    uint SliderValue1:7;
    uint SliderValue2:7;//41bits
    uint ServoIndexW:4;
    uint ServoIndexX:4;
    uint ServoIndexY:4;
    uint ServoIndexZ:4;//57bits
    uint Throttle:4;
    uint Tail:4;       //65bits
    quint32 padding:31;  //96bits
    quint32 padding1;  //128bits
} __attribute__((packed))  heliGUISettingsStruct;

typedef struct {
    uint FixedWingThrottle:4;
    uint FixedWingRoll1:4;
    uint FixedWingRoll2:4;
    uint FixedWingPitch1:4;
    uint FixedWingPitch2:4;
    uint FixedWingYaw1:4;
    uint FixedWingYaw2:4;
    uint padding:4;         //32bits
    quint32 padding1;
    quint32 padding2;
    quint32 padding3;       //128bits
} __attribute__((packed))  fixedGUISettingsStruct;

typedef struct {
    uint GroundVehicleThrottle1:4;
    uint GroundVehicleThrottle2:4;
    uint GroundVehicleSteering1:4;
    uint GroundVehicleSteering2:4;
    uint padding:16;         //32bits
    quint32 padding1;
    quint32 padding2;
    quint32 padding3;       //128bits
} __attribute__((packed))  groundGUISettingsStruct;

typedef union
{
    uint                    UAVObject[4];   //32bits * 4
    heliGUISettingsStruct   heli;           //128bits
    fixedGUISettingsStruct  fixedwing;
    multiGUISettingsStruct  multi;
    groundGUISettingsStruct ground;
} GUIConfigDataUnion;


class VehicleConfig: public ConfigTaskWidget
{
    Q_OBJECT

    public:
        VehicleConfig(QWidget *parent = 0);
        ~VehicleConfig();

        static GUIConfigDataUnion GetConfigData();
        static void SetConfigData(GUIConfigDataUnion configData);
        static void resetField(UAVObjectField * field);
        static void setComboCurrentIndex(QComboBox* box, int index);
        static void enableComboBox(QWidget* owner, QString boxName, bool enable);

        virtual void ResetActuators(GUIConfigDataUnion* configData);
        virtual QStringList getChannelDescriptions();

        QStringList channelNames;
        QStringList mixerTypes;
        QStringList mixerVectors;

        static const quint32 CHANNEL_NUMELEM = 10;

    private:

        static UAVObjectManager* getUAVObjectManager();

    private slots:

    public slots:

    signals:
        //void ConfigurationChanged();

protected:

};

#endif // GUIVEHICLECONFIG_H
