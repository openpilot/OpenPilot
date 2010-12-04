/**
 ******************************************************************************
 *
 * @file       manualcontrolsettings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: manualcontrolsettings.xml. 
 *             This is an automatically generated file.
 *             DO NOT modify manually.
 *
 * @brief      The UAVUObjects GCS plugin 
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
#ifndef MANUALCONTROLSETTINGS_H
#define MANUALCONTROLSETTINGS_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT ManualControlSettings: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint8 InputMode;
        quint8 Roll;
        quint8 Pitch;
        quint8 Yaw;
        quint8 Throttle;
        quint8 FlightMode;
        quint8 Accessory1;
        quint8 Accessory2;
        quint8 Accessory3;
        quint8 Pos1StabilizationSettings[3];
        quint8 Pos2StabilizationSettings[3];
        quint8 Pos3StabilizationSettings[3];
        quint8 Pos1FlightMode;
        quint8 Pos2FlightMode;
        quint8 Pos3FlightMode;
        qint16 ChannelMax[8];
        qint16 ChannelNeutral[8];
        qint16 ChannelMin[8];
        quint16 ArmedTimeout;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field InputMode information
    /* Enumeration options for field InputMode */
    typedef enum { INPUTMODE_PWM=0, INPUTMODE_PPM=1, INPUTMODE_SPEKTRUM=2 } InputModeOptions;
    // Field Roll information
    /* Enumeration options for field Roll */
    typedef enum { ROLL_CHANNEL0=0, ROLL_CHANNEL1=1, ROLL_CHANNEL2=2, ROLL_CHANNEL3=3, ROLL_CHANNEL4=4, ROLL_CHANNEL5=5, ROLL_CHANNEL6=6, ROLL_CHANNEL7=7, ROLL_NONE=8 } RollOptions;
    // Field Pitch information
    /* Enumeration options for field Pitch */
    typedef enum { PITCH_CHANNEL0=0, PITCH_CHANNEL1=1, PITCH_CHANNEL2=2, PITCH_CHANNEL3=3, PITCH_CHANNEL4=4, PITCH_CHANNEL5=5, PITCH_CHANNEL6=6, PITCH_CHANNEL7=7, PITCH_NONE=8 } PitchOptions;
    // Field Yaw information
    /* Enumeration options for field Yaw */
    typedef enum { YAW_CHANNEL0=0, YAW_CHANNEL1=1, YAW_CHANNEL2=2, YAW_CHANNEL3=3, YAW_CHANNEL4=4, YAW_CHANNEL5=5, YAW_CHANNEL6=6, YAW_CHANNEL7=7, YAW_NONE=8 } YawOptions;
    // Field Throttle information
    /* Enumeration options for field Throttle */
    typedef enum { THROTTLE_CHANNEL0=0, THROTTLE_CHANNEL1=1, THROTTLE_CHANNEL2=2, THROTTLE_CHANNEL3=3, THROTTLE_CHANNEL4=4, THROTTLE_CHANNEL5=5, THROTTLE_CHANNEL6=6, THROTTLE_CHANNEL7=7, THROTTLE_NONE=8 } ThrottleOptions;
    // Field FlightMode information
    /* Enumeration options for field FlightMode */
    typedef enum { FLIGHTMODE_CHANNEL0=0, FLIGHTMODE_CHANNEL1=1, FLIGHTMODE_CHANNEL2=2, FLIGHTMODE_CHANNEL3=3, FLIGHTMODE_CHANNEL4=4, FLIGHTMODE_CHANNEL5=5, FLIGHTMODE_CHANNEL6=6, FLIGHTMODE_CHANNEL7=7, FLIGHTMODE_NONE=8 } FlightModeOptions;
    // Field Accessory1 information
    /* Enumeration options for field Accessory1 */
    typedef enum { ACCESSORY1_CHANNEL0=0, ACCESSORY1_CHANNEL1=1, ACCESSORY1_CHANNEL2=2, ACCESSORY1_CHANNEL3=3, ACCESSORY1_CHANNEL4=4, ACCESSORY1_CHANNEL5=5, ACCESSORY1_CHANNEL6=6, ACCESSORY1_CHANNEL7=7, ACCESSORY1_NONE=8 } Accessory1Options;
    // Field Accessory2 information
    /* Enumeration options for field Accessory2 */
    typedef enum { ACCESSORY2_CHANNEL0=0, ACCESSORY2_CHANNEL1=1, ACCESSORY2_CHANNEL2=2, ACCESSORY2_CHANNEL3=3, ACCESSORY2_CHANNEL4=4, ACCESSORY2_CHANNEL5=5, ACCESSORY2_CHANNEL6=6, ACCESSORY2_CHANNEL7=7, ACCESSORY2_NONE=8 } Accessory2Options;
    // Field Accessory3 information
    /* Enumeration options for field Accessory3 */
    typedef enum { ACCESSORY3_CHANNEL0=0, ACCESSORY3_CHANNEL1=1, ACCESSORY3_CHANNEL2=2, ACCESSORY3_CHANNEL3=3, ACCESSORY3_CHANNEL4=4, ACCESSORY3_CHANNEL5=5, ACCESSORY3_CHANNEL6=6, ACCESSORY3_CHANNEL7=7, ACCESSORY3_NONE=8 } Accessory3Options;
    // Field Pos1StabilizationSettings information
    /* Enumeration options for field Pos1StabilizationSettings */
    typedef enum { POS1STABILIZATIONSETTINGS_NONE=0, POS1STABILIZATIONSETTINGS_RATE=1, POS1STABILIZATIONSETTINGS_ATTITUDE=2 } Pos1StabilizationSettingsOptions;
    /* Array element names for field Pos1StabilizationSettings */
    typedef enum { POS1STABILIZATIONSETTINGS_ROLL=0, POS1STABILIZATIONSETTINGS_PITCH=1, POS1STABILIZATIONSETTINGS_YAW=2 } Pos1StabilizationSettingsElem;
    /* Number of elements for field Pos1StabilizationSettings */
    static const quint32 POS1STABILIZATIONSETTINGS_NUMELEM = 3;
    // Field Pos2StabilizationSettings information
    /* Enumeration options for field Pos2StabilizationSettings */
    typedef enum { POS2STABILIZATIONSETTINGS_NONE=0, POS2STABILIZATIONSETTINGS_RATE=1, POS2STABILIZATIONSETTINGS_ATTITUDE=2 } Pos2StabilizationSettingsOptions;
    /* Array element names for field Pos2StabilizationSettings */
    typedef enum { POS2STABILIZATIONSETTINGS_ROLL=0, POS2STABILIZATIONSETTINGS_PITCH=1, POS2STABILIZATIONSETTINGS_YAW=2 } Pos2StabilizationSettingsElem;
    /* Number of elements for field Pos2StabilizationSettings */
    static const quint32 POS2STABILIZATIONSETTINGS_NUMELEM = 3;
    // Field Pos3StabilizationSettings information
    /* Enumeration options for field Pos3StabilizationSettings */
    typedef enum { POS3STABILIZATIONSETTINGS_NONE=0, POS3STABILIZATIONSETTINGS_RATE=1, POS3STABILIZATIONSETTINGS_ATTITUDE=2 } Pos3StabilizationSettingsOptions;
    /* Array element names for field Pos3StabilizationSettings */
    typedef enum { POS3STABILIZATIONSETTINGS_ROLL=0, POS3STABILIZATIONSETTINGS_PITCH=1, POS3STABILIZATIONSETTINGS_YAW=2 } Pos3StabilizationSettingsElem;
    /* Number of elements for field Pos3StabilizationSettings */
    static const quint32 POS3STABILIZATIONSETTINGS_NUMELEM = 3;
    // Field Pos1FlightMode information
    /* Enumeration options for field Pos1FlightMode */
    typedef enum { POS1FLIGHTMODE_MANUAL=0, POS1FLIGHTMODE_STABILIZED=1, POS1FLIGHTMODE_AUTO=2 } Pos1FlightModeOptions;
    // Field Pos2FlightMode information
    /* Enumeration options for field Pos2FlightMode */
    typedef enum { POS2FLIGHTMODE_MANUAL=0, POS2FLIGHTMODE_STABILIZED=1, POS2FLIGHTMODE_AUTO=2 } Pos2FlightModeOptions;
    // Field Pos3FlightMode information
    /* Enumeration options for field Pos3FlightMode */
    typedef enum { POS3FLIGHTMODE_MANUAL=0, POS3FLIGHTMODE_STABILIZED=1, POS3FLIGHTMODE_AUTO=2 } Pos3FlightModeOptions;
    // Field ChannelMax information
    /* Number of elements for field ChannelMax */
    static const quint32 CHANNELMAX_NUMELEM = 8;
    // Field ChannelNeutral information
    /* Number of elements for field ChannelNeutral */
    static const quint32 CHANNELNEUTRAL_NUMELEM = 8;
    // Field ChannelMin information
    /* Number of elements for field ChannelMin */
    static const quint32 CHANNELMIN_NUMELEM = 8;
    // Field ArmedTimeout information

  
    // Constants
    static const quint32 OBJID = 157988682U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 1;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    ManualControlSettings();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static ManualControlSettings* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // MANUALCONTROLSETTINGS_H
