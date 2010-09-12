/**
 ******************************************************************************
 *
 * @file       actuatorsettings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: actuatorsettings.xml. 
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
#ifndef ACTUATORSETTINGS_H
#define ACTUATORSETTINGS_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT ActuatorSettings: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint8 FixedWingRoll1;
        quint8 FixedWingRoll2;
        quint8 FixedWingPitch1;
        quint8 FixedWingPitch2;
        quint8 FixedWingYaw;
        quint8 FixedWingThrottle;
        quint8 VTOLMotorN;
        quint8 VTOLMotorNE;
        quint8 VTOLMotorE;
        quint8 VTOLMotorSE;
        quint8 VTOLMotorS;
        quint8 VTOLMotorSW;
        quint8 VTOLMotorW;
        quint8 VTOLMotorNW;
        quint8 CCPMYawStabilizationInManualMode;
        quint8 CCPMFlybarless;
        float CCPMThrottleCurve[5];
        float CCPMPitchCurve[5];
        float CCPMCollectiveConstant;
        float CCPMCorrectionAngle;
        float CCPMAngleW;
        float CCPMAngleX;
        float CCPMAngleY;
        float CCPMAngleZ;
        quint8 CCPMServoW;
        quint8 CCPMServoX;
        quint8 CCPMServoY;
        quint8 CCPMServoZ;
        quint8 CCPMThrottle;
        quint8 CCPMTailRotor;
        quint16 UpdatePeriod;
        qint16 ChannelUpdateFreq[2];
        qint16 ChannelMax[8];
        qint16 ChannelNeutral[8];
        qint16 ChannelMin[8];

    } __attribute__((packed)) DataFields;

    // Field information
    // Field FixedWingRoll1 information
    /* Enumeration options for field FixedWingRoll1 */
    typedef enum { FIXEDWINGROLL1_CHANNEL0=0, FIXEDWINGROLL1_CHANNEL1=1, FIXEDWINGROLL1_CHANNEL2=2, FIXEDWINGROLL1_CHANNEL3=3, FIXEDWINGROLL1_CHANNEL4=4, FIXEDWINGROLL1_CHANNEL5=5, FIXEDWINGROLL1_CHANNEL6=6, FIXEDWINGROLL1_CHANNEL7=7, FIXEDWINGROLL1_NONE=8 } FixedWingRoll1Options;
    // Field FixedWingRoll2 information
    /* Enumeration options for field FixedWingRoll2 */
    typedef enum { FIXEDWINGROLL2_CHANNEL0=0, FIXEDWINGROLL2_CHANNEL1=1, FIXEDWINGROLL2_CHANNEL2=2, FIXEDWINGROLL2_CHANNEL3=3, FIXEDWINGROLL2_CHANNEL4=4, FIXEDWINGROLL2_CHANNEL5=5, FIXEDWINGROLL2_CHANNEL6=6, FIXEDWINGROLL2_CHANNEL7=7, FIXEDWINGROLL2_NONE=8 } FixedWingRoll2Options;
    // Field FixedWingPitch1 information
    /* Enumeration options for field FixedWingPitch1 */
    typedef enum { FIXEDWINGPITCH1_CHANNEL0=0, FIXEDWINGPITCH1_CHANNEL1=1, FIXEDWINGPITCH1_CHANNEL2=2, FIXEDWINGPITCH1_CHANNEL3=3, FIXEDWINGPITCH1_CHANNEL4=4, FIXEDWINGPITCH1_CHANNEL5=5, FIXEDWINGPITCH1_CHANNEL6=6, FIXEDWINGPITCH1_CHANNEL7=7, FIXEDWINGPITCH1_NONE=8 } FixedWingPitch1Options;
    // Field FixedWingPitch2 information
    /* Enumeration options for field FixedWingPitch2 */
    typedef enum { FIXEDWINGPITCH2_CHANNEL0=0, FIXEDWINGPITCH2_CHANNEL1=1, FIXEDWINGPITCH2_CHANNEL2=2, FIXEDWINGPITCH2_CHANNEL3=3, FIXEDWINGPITCH2_CHANNEL4=4, FIXEDWINGPITCH2_CHANNEL5=5, FIXEDWINGPITCH2_CHANNEL6=6, FIXEDWINGPITCH2_CHANNEL7=7, FIXEDWINGPITCH2_NONE=8 } FixedWingPitch2Options;
    // Field FixedWingYaw information
    /* Enumeration options for field FixedWingYaw */
    typedef enum { FIXEDWINGYAW_CHANNEL0=0, FIXEDWINGYAW_CHANNEL1=1, FIXEDWINGYAW_CHANNEL2=2, FIXEDWINGYAW_CHANNEL3=3, FIXEDWINGYAW_CHANNEL4=4, FIXEDWINGYAW_CHANNEL5=5, FIXEDWINGYAW_CHANNEL6=6, FIXEDWINGYAW_CHANNEL7=7, FIXEDWINGYAW_NONE=8 } FixedWingYawOptions;
    // Field FixedWingThrottle information
    /* Enumeration options for field FixedWingThrottle */
    typedef enum { FIXEDWINGTHROTTLE_CHANNEL0=0, FIXEDWINGTHROTTLE_CHANNEL1=1, FIXEDWINGTHROTTLE_CHANNEL2=2, FIXEDWINGTHROTTLE_CHANNEL3=3, FIXEDWINGTHROTTLE_CHANNEL4=4, FIXEDWINGTHROTTLE_CHANNEL5=5, FIXEDWINGTHROTTLE_CHANNEL6=6, FIXEDWINGTHROTTLE_CHANNEL7=7, FIXEDWINGTHROTTLE_NONE=8 } FixedWingThrottleOptions;
    // Field VTOLMotorN information
    /* Enumeration options for field VTOLMotorN */
    typedef enum { VTOLMOTORN_CHANNEL0=0, VTOLMOTORN_CHANNEL1=1, VTOLMOTORN_CHANNEL2=2, VTOLMOTORN_CHANNEL3=3, VTOLMOTORN_CHANNEL4=4, VTOLMOTORN_CHANNEL5=5, VTOLMOTORN_CHANNEL6=6, VTOLMOTORN_CHANNEL7=7, VTOLMOTORN_NONE=8 } VTOLMotorNOptions;
    // Field VTOLMotorNE information
    /* Enumeration options for field VTOLMotorNE */
    typedef enum { VTOLMOTORNE_CHANNEL0=0, VTOLMOTORNE_CHANNEL1=1, VTOLMOTORNE_CHANNEL2=2, VTOLMOTORNE_CHANNEL3=3, VTOLMOTORNE_CHANNEL4=4, VTOLMOTORNE_CHANNEL5=5, VTOLMOTORNE_CHANNEL6=6, VTOLMOTORNE_CHANNEL7=7, VTOLMOTORNE_NONE=8 } VTOLMotorNEOptions;
    // Field VTOLMotorE information
    /* Enumeration options for field VTOLMotorE */
    typedef enum { VTOLMOTORE_CHANNEL0=0, VTOLMOTORE_CHANNEL1=1, VTOLMOTORE_CHANNEL2=2, VTOLMOTORE_CHANNEL3=3, VTOLMOTORE_CHANNEL4=4, VTOLMOTORE_CHANNEL5=5, VTOLMOTORE_CHANNEL6=6, VTOLMOTORE_CHANNEL7=7, VTOLMOTORE_NONE=8 } VTOLMotorEOptions;
    // Field VTOLMotorSE information
    /* Enumeration options for field VTOLMotorSE */
    typedef enum { VTOLMOTORSE_CHANNEL0=0, VTOLMOTORSE_CHANNEL1=1, VTOLMOTORSE_CHANNEL2=2, VTOLMOTORSE_CHANNEL3=3, VTOLMOTORSE_CHANNEL4=4, VTOLMOTORSE_CHANNEL5=5, VTOLMOTORSE_CHANNEL6=6, VTOLMOTORSE_CHANNEL7=7, VTOLMOTORSE_NONE=8 } VTOLMotorSEOptions;
    // Field VTOLMotorS information
    /* Enumeration options for field VTOLMotorS */
    typedef enum { VTOLMOTORS_CHANNEL0=0, VTOLMOTORS_CHANNEL1=1, VTOLMOTORS_CHANNEL2=2, VTOLMOTORS_CHANNEL3=3, VTOLMOTORS_CHANNEL4=4, VTOLMOTORS_CHANNEL5=5, VTOLMOTORS_CHANNEL6=6, VTOLMOTORS_CHANNEL7=7, VTOLMOTORS_NONE=8 } VTOLMotorSOptions;
    // Field VTOLMotorSW information
    /* Enumeration options for field VTOLMotorSW */
    typedef enum { VTOLMOTORSW_CHANNEL0=0, VTOLMOTORSW_CHANNEL1=1, VTOLMOTORSW_CHANNEL2=2, VTOLMOTORSW_CHANNEL3=3, VTOLMOTORSW_CHANNEL4=4, VTOLMOTORSW_CHANNEL5=5, VTOLMOTORSW_CHANNEL6=6, VTOLMOTORSW_CHANNEL7=7, VTOLMOTORSW_NONE=8 } VTOLMotorSWOptions;
    // Field VTOLMotorW information
    /* Enumeration options for field VTOLMotorW */
    typedef enum { VTOLMOTORW_CHANNEL0=0, VTOLMOTORW_CHANNEL1=1, VTOLMOTORW_CHANNEL2=2, VTOLMOTORW_CHANNEL3=3, VTOLMOTORW_CHANNEL4=4, VTOLMOTORW_CHANNEL5=5, VTOLMOTORW_CHANNEL6=6, VTOLMOTORW_CHANNEL7=7, VTOLMOTORW_NONE=8 } VTOLMotorWOptions;
    // Field VTOLMotorNW information
    /* Enumeration options for field VTOLMotorNW */
    typedef enum { VTOLMOTORNW_CHANNEL0=0, VTOLMOTORNW_CHANNEL1=1, VTOLMOTORNW_CHANNEL2=2, VTOLMOTORNW_CHANNEL3=3, VTOLMOTORNW_CHANNEL4=4, VTOLMOTORNW_CHANNEL5=5, VTOLMOTORNW_CHANNEL6=6, VTOLMOTORNW_CHANNEL7=7, VTOLMOTORNW_NONE=8 } VTOLMotorNWOptions;
    // Field CCPMYawStabilizationInManualMode information
    /* Enumeration options for field CCPMYawStabilizationInManualMode */
    typedef enum { CCPMYAWSTABILIZATIONINMANUALMODE_FALSE=0, CCPMYAWSTABILIZATIONINMANUALMODE_TRUE=1 } CCPMYawStabilizationInManualModeOptions;
    // Field CCPMFlybarless information
    /* Enumeration options for field CCPMFlybarless */
    typedef enum { CCPMFLYBARLESS_FALSE=0, CCPMFLYBARLESS_TRUE=1 } CCPMFlybarlessOptions;
    // Field CCPMThrottleCurve information
    /* Number of elements for field CCPMThrottleCurve */
    static const quint32 CCPMTHROTTLECURVE_NUMELEM = 5;
    // Field CCPMPitchCurve information
    /* Number of elements for field CCPMPitchCurve */
    static const quint32 CCPMPITCHCURVE_NUMELEM = 5;
    // Field CCPMCollectiveConstant information
    // Field CCPMCorrectionAngle information
    // Field CCPMAngleW information
    // Field CCPMAngleX information
    // Field CCPMAngleY information
    // Field CCPMAngleZ information
    // Field CCPMServoW information
    /* Enumeration options for field CCPMServoW */
    typedef enum { CCPMSERVOW_CHANNEL0=0, CCPMSERVOW_CHANNEL1=1, CCPMSERVOW_CHANNEL2=2, CCPMSERVOW_CHANNEL3=3, CCPMSERVOW_CHANNEL4=4, CCPMSERVOW_CHANNEL5=5, CCPMSERVOW_CHANNEL6=6, CCPMSERVOW_CHANNEL7=7, CCPMSERVOW_NONE=8 } CCPMServoWOptions;
    // Field CCPMServoX information
    /* Enumeration options for field CCPMServoX */
    typedef enum { CCPMSERVOX_CHANNEL0=0, CCPMSERVOX_CHANNEL1=1, CCPMSERVOX_CHANNEL2=2, CCPMSERVOX_CHANNEL3=3, CCPMSERVOX_CHANNEL4=4, CCPMSERVOX_CHANNEL5=5, CCPMSERVOX_CHANNEL6=6, CCPMSERVOX_CHANNEL7=7, CCPMSERVOX_NONE=8 } CCPMServoXOptions;
    // Field CCPMServoY information
    /* Enumeration options for field CCPMServoY */
    typedef enum { CCPMSERVOY_CHANNEL0=0, CCPMSERVOY_CHANNEL1=1, CCPMSERVOY_CHANNEL2=2, CCPMSERVOY_CHANNEL3=3, CCPMSERVOY_CHANNEL4=4, CCPMSERVOY_CHANNEL5=5, CCPMSERVOY_CHANNEL6=6, CCPMSERVOY_CHANNEL7=7, CCPMSERVOY_NONE=8 } CCPMServoYOptions;
    // Field CCPMServoZ information
    /* Enumeration options for field CCPMServoZ */
    typedef enum { CCPMSERVOZ_CHANNEL0=0, CCPMSERVOZ_CHANNEL1=1, CCPMSERVOZ_CHANNEL2=2, CCPMSERVOZ_CHANNEL3=3, CCPMSERVOZ_CHANNEL4=4, CCPMSERVOZ_CHANNEL5=5, CCPMSERVOZ_CHANNEL6=6, CCPMSERVOZ_CHANNEL7=7, CCPMSERVOZ_NONE=8 } CCPMServoZOptions;
    // Field CCPMThrottle information
    /* Enumeration options for field CCPMThrottle */
    typedef enum { CCPMTHROTTLE_CHANNEL0=0, CCPMTHROTTLE_CHANNEL1=1, CCPMTHROTTLE_CHANNEL2=2, CCPMTHROTTLE_CHANNEL3=3, CCPMTHROTTLE_CHANNEL4=4, CCPMTHROTTLE_CHANNEL5=5, CCPMTHROTTLE_CHANNEL6=6, CCPMTHROTTLE_CHANNEL7=7, CCPMTHROTTLE_NONE=8 } CCPMThrottleOptions;
    // Field CCPMTailRotor information
    /* Enumeration options for field CCPMTailRotor */
    typedef enum { CCPMTAILROTOR_CHANNEL0=0, CCPMTAILROTOR_CHANNEL1=1, CCPMTAILROTOR_CHANNEL2=2, CCPMTAILROTOR_CHANNEL3=3, CCPMTAILROTOR_CHANNEL4=4, CCPMTAILROTOR_CHANNEL5=5, CCPMTAILROTOR_CHANNEL6=6, CCPMTAILROTOR_CHANNEL7=7, CCPMTAILROTOR_NONE=8 } CCPMTailRotorOptions;
    // Field UpdatePeriod information
    // Field ChannelUpdateFreq information
    /* Number of elements for field ChannelUpdateFreq */
    static const quint32 CHANNELUPDATEFREQ_NUMELEM = 2;
    // Field ChannelMax information
    /* Number of elements for field ChannelMax */
    static const quint32 CHANNELMAX_NUMELEM = 8;
    // Field ChannelNeutral information
    /* Number of elements for field ChannelNeutral */
    static const quint32 CHANNELNEUTRAL_NUMELEM = 8;
    // Field ChannelMin information
    /* Number of elements for field ChannelMin */
    static const quint32 CHANNELMIN_NUMELEM = 8;

  
    // Constants
    static const quint32 OBJID = 562991684U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 1;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    ActuatorSettings();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static ActuatorSettings* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // ACTUATORSETTINGS_H
