/**
 ******************************************************************************
 *
 * @file       vehicleconfigurationhelper.h
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

#ifndef VEHICLECONFIGURATIONHELPER_H
#define VEHICLECONFIGURATIONHELPER_H

#include "vehicleconfigurationsource.h"
#include "uavobjectmanager.h"

struct channelSettings {
    int type;
    int throttle1;
    int throttle2;
    int roll;
    int pitch;
    int yaw;

    channelSettings() : type(), throttle1(), throttle2(), roll(), pitch(), yaw() {}

    channelSettings(int t, int th1, int th2, int r, int p, int y)
        : type(t), throttle1(th1), throttle2(th2), roll(r), pitch(p), yaw(y) {}
};

struct mixerSettings {
    channelSettings channels[10];
};

class VehicleConfigurationHelper
{
public:
    VehicleConfigurationHelper(VehicleConfigurationSource* configSource);
    void setupVehicle();
private:
    static const qint16 LEGACY_ESC_FREQUENCE = 50;
    static const qint16 RAPID_ESC_FREQUENCE = 400;

    VehicleConfigurationSource *m_configSource;
    UAVObjectManager *m_uavoManager;

    void applyHardwareConfiguration();
    void applyVehicleConfiguration();
    void applyOutputConfiguration();
    void applyFlighModeConfiguration();
    void applyLevellingConfiguration();

    void resetVehicleConfig();
    void resetGUIData();

    void setupTriCopter();
    void setupQuadCopter();
    void setupHexaCopter();
    void setupOctoCopter();

};

#endif // VEHICLECONFIGURATIONHELPER_H
