/**
 ******************************************************************************
 *
 * @file       aerosimrcdatastruct.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2012.
 * @addtogroup 3rdParty Third-party integration
 * @{
 * @addtogroup AeroSimRC AeroSimRC proxy plugin
 * @{
 * @brief AeroSimRC simulator to HITL proxy plugin
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

#ifndef AEROSIMRCDATASTRUCT_H
#define AEROSIMRCDATASTRUCT_H

#include <QtCore>

const quint8 AEROSIMRC_MAX_CHANNELS = 39;
const quint16 DBG_BUFFER_MAX_SIZE = 4096;

#define MAX_DLL_USER_MENU_ITEMS 16
#define OBSOLETE_MIT_COMMAND    (1 << 0)
#define OBSOLETE_MIT_CHECKBOX   (1 << 1)
#define OBSOLETE_MIT_SEPARATOR  (1 << 7)

#define PACK_STRUCT __attribute__((packed))

struct simToPlugin
{
    quint16 structSize;
    float simTimeStep;
    float chSimTX[AEROSIMRC_MAX_CHANNELS];
    float chSimRX[AEROSIMRC_MAX_CHANNELS];
    uchar *OSDVideoBuf;
    quint32 simMenuStatus;
    float initPosX;
    float initPosY;
    float initPosZ;
    float initHeading;
    float initPitch;
    float initRoll;
    float wpHomeX;
    float wpHomeY;
    float wpHomeLat;
    float wpHomeLong;
    const char *wpHomeDesc; // (m, deg, string)
    float wpAX;
    float wpAY;
    float wpALat;
    float wpALong;
    const char *wpADesc;    // (m, deg, string)
    float wpBX;
    float wpBY;
    float wpBLat;
    float wpBLong;
    const char *wpBDesc;    // (m, deg, string)
    float wpCX;
    float wpCY;
    float wpCLat;
    float wpCLong;
    const char *wpCDesc;    // (m, deg, string)
    float wpDX;
    float wpDY;
    float wpDLat;
    float wpDLong;
    const char *wpDDesc;    // (m, deg, string)
    float posX;
    float posY;
    float posZ;
    float velX;
    float velY;
    float velZ;
    float angVelX;
    float angVelY;
    float angVelZ;
    float accelX;
    float accelY;
    float accelZ;
    qreal latitude;
    qreal longitude;
    float AGL;
    float heading;
    float pitch;
    float roll;
    float windVelX;
    float windVelY;
    float windVelZ;
    float eng1RPM;
    float eng2RPM;
    float eng3RPM;
    float eng4RPM;
    float voltage;          // V
    float current;          // A
    float consumedCharge;   // Ah
    float capacity;         // Ah
    float fuelConsumed;     // l
    float fuelTankCapacity; // l
    // ver 3.83
    qint16 screenW;
    qint16 screenH;
    // Model Orientation Matrix (X=Right, Y=Front, Z=Up)
    float axisXx;
    float axisXy;
    float axisXz;
    float axisYx;
    float axisYy;
    float axisYz;
    float axisZx;
    float axisZy;
    float axisZz;
    // Model data in body frame coordinates (X=Right, Y=Front, Z=Up)
    float velXm;    // m/s   Model velocity in body coordinates
    float velYm;
    float velZm;
    float angVelXm; // rad/s Model angular velocity in body coordinates
    float angVelYm;
    float angVelZm;
    float accelXm;  // m/s/s Model acceleration in body coordinates
    float accelYm;
    float accelZm;
    // ver 3.90
    quint32 OSDVideoBufSize;
} PACK_STRUCT ;     // normal - 592, packed - 582 OK (3.81)
                    // normal - ???, packed - 658 OK (3.83)
                    // normal - ???, packed - 662 OK (3.90)

struct pluginToSim
{
    quint16 structSize;
    const char *dbgInfoText;
    uchar chOverTX[AEROSIMRC_MAX_CHANNELS];
    float chNewTX[AEROSIMRC_MAX_CHANNELS];
    uchar chOverRX[AEROSIMRC_MAX_CHANNELS];
    float chNewRX[AEROSIMRC_MAX_CHANNELS];
    float newPosX;      // m
    float newPosY;
    float newPosZ;
    float newVelX;      // m/s
    float newVelY;
    float newVelZ;
    float newAngVelX;   // rad/s
    float newAngVelY;
    float newAngVelZ;
    float newHeading;   // rad
    float newPitch;
    float newRoll;
    quint32 modelOverrideFlags;
    quint32 newMenuStatus;
    quint8 isOSDShow;
    quint8 isOSDChanged;
    quint16 OSDWindow_DX;
    quint16 OSDWindow_DY;
    float OSDScale;
    float newWindVelX;
    float newWindVelY;
    float newWindVelZ;
    float newEng1RPM;
    float newEng2RPM;
    float newEng3RPM;
    float newEng4RPM;
    float newVoltage;
    float newCurrent;
    float newConsumedCharge;
    float newFuelConsumed;
    quint8 modelCrashInhibit;
    // ver 3.83
    qint16 newScreenW;     // Simulator window position and size on screen
    qint16 newScreenH;
    qint16 newScreenX;
    qint16 newScreenY;
} PACK_STRUCT ;     // normal 516, packed 507 OK (3.81)
                    // normal ???, packed 515 OK (3.83 & 3.90)

struct TPluginMenuItem
{
    quint32 OBSOLETE_eType;
    char *OBSOLETE_strName;
} PACK_STRUCT ;

struct pluginInit
{
    quint32 nStructSize;
    char *OBSOLETE_strMenuTitle;
    TPluginMenuItem OBSOLETE_atMenuItem[MAX_DLL_USER_MENU_ITEMS];
    const char *strPluginFolder;
    const char *strOutputFolder;
} PACK_STRUCT ;     // normal - 144, packed - 144 OK (3.81 & 3.83 & 3.90)

#undef PACK_STRUCT

#endif // AEROSIMRCDATASTRUCT_H
