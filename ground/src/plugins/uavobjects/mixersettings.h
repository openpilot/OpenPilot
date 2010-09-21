/**
 ******************************************************************************
 *
 * @file       mixersettings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: mixersettings.xml. 
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
#ifndef MIXERSETTINGS_H
#define MIXERSETTINGS_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT MixerSettings: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        float MaxAccel;
        float FeedForward;
        float AccelTime;
        float DecelTime;
        float ThrottleCurve1[5];
        float ThrottleCurve2[5];
        quint8 Mixer0Type;
        float Mixer0Matrix[5];
        quint8 Mixer1Type;
        float Mixer1Matrix[5];
        quint8 Mixer2Type;
        float Mixer2Matrix[5];
        quint8 Mixer3Type;
        float Mixer3Matrix[5];
        quint8 Mixer4Type;
        float Mixer4Matrix[5];
        quint8 Mixer5Type;
        float Mixer5Matrix[5];
        quint8 Mixer6Type;
        float Mixer6Matrix[5];
        quint8 Mixer7Type;
        float Mixer7Matrix[5];

    } __attribute__((packed)) DataFields;

    // Field information
    // Field MaxAccel information
    // Field FeedForward information
    // Field AccelTime information
    // Field DecelTime information
    // Field ThrottleCurve1 information
    /* Array element names for field ThrottleCurve1 */
    typedef enum { THROTTLECURVE1_0=0, THROTTLECURVE1_25=1, THROTTLECURVE1_50=2, THROTTLECURVE1_75=3, THROTTLECURVE1_100=4 } ThrottleCurve1Elem;
    /* Number of elements for field ThrottleCurve1 */
    static const quint32 THROTTLECURVE1_NUMELEM = 5;
    // Field ThrottleCurve2 information
    /* Array element names for field ThrottleCurve2 */
    typedef enum { THROTTLECURVE2_0=0, THROTTLECURVE2_25=1, THROTTLECURVE2_50=2, THROTTLECURVE2_75=3, THROTTLECURVE2_100=4 } ThrottleCurve2Elem;
    /* Number of elements for field ThrottleCurve2 */
    static const quint32 THROTTLECURVE2_NUMELEM = 5;
    // Field Mixer0Type information
    /* Enumeration options for field Mixer0Type */
    typedef enum { MIXER0TYPE_DISABLED=0, MIXER0TYPE_MOTOR=1, MIXER0TYPE_SERVO=2 } Mixer0TypeOptions;
    // Field Mixer0Matrix information
    /* Array element names for field Mixer0Matrix */
    typedef enum { MIXER0MATRIX_THROTTLECURVE1=0, MIXER0MATRIX_THROTTLECURVE2=1, MIXER0MATRIX_ROLL=2, MIXER0MATRIX_PITCH=3, MIXER0MATRIX_YAW=4 } Mixer0MatrixElem;
    /* Number of elements for field Mixer0Matrix */
    static const quint32 MIXER0MATRIX_NUMELEM = 5;
    // Field Mixer1Type information
    /* Enumeration options for field Mixer1Type */
    typedef enum { MIXER1TYPE_DISABLED=0, MIXER1TYPE_MOTOR=1, MIXER1TYPE_SERVO=2 } Mixer1TypeOptions;
    // Field Mixer1Matrix information
    /* Array element names for field Mixer1Matrix */
    typedef enum { MIXER1MATRIX_THROTTLECURVE1=0, MIXER1MATRIX_THROTTLECURVE2=1, MIXER1MATRIX_ROLL=2, MIXER1MATRIX_PITCH=3, MIXER1MATRIX_YAW=4 } Mixer1MatrixElem;
    /* Number of elements for field Mixer1Matrix */
    static const quint32 MIXER1MATRIX_NUMELEM = 5;
    // Field Mixer2Type information
    /* Enumeration options for field Mixer2Type */
    typedef enum { MIXER2TYPE_DISABLED=0, MIXER2TYPE_MOTOR=1, MIXER2TYPE_SERVO=2 } Mixer2TypeOptions;
    // Field Mixer2Matrix information
    /* Array element names for field Mixer2Matrix */
    typedef enum { MIXER2MATRIX_THROTTLECURVE1=0, MIXER2MATRIX_THROTTLECURVE2=1, MIXER2MATRIX_ROLL=2, MIXER2MATRIX_PITCH=3, MIXER2MATRIX_YAW=4 } Mixer2MatrixElem;
    /* Number of elements for field Mixer2Matrix */
    static const quint32 MIXER2MATRIX_NUMELEM = 5;
    // Field Mixer3Type information
    /* Enumeration options for field Mixer3Type */
    typedef enum { MIXER3TYPE_DISABLED=0, MIXER3TYPE_MOTOR=1, MIXER3TYPE_SERVO=2 } Mixer3TypeOptions;
    // Field Mixer3Matrix information
    /* Array element names for field Mixer3Matrix */
    typedef enum { MIXER3MATRIX_THROTTLECURVE1=0, MIXER3MATRIX_THROTTLECURVE2=1, MIXER3MATRIX_ROLL=2, MIXER3MATRIX_PITCH=3, MIXER3MATRIX_YAW=4 } Mixer3MatrixElem;
    /* Number of elements for field Mixer3Matrix */
    static const quint32 MIXER3MATRIX_NUMELEM = 5;
    // Field Mixer4Type information
    /* Enumeration options for field Mixer4Type */
    typedef enum { MIXER4TYPE_DISABLED=0, MIXER4TYPE_MOTOR=1, MIXER4TYPE_SERVO=2 } Mixer4TypeOptions;
    // Field Mixer4Matrix information
    /* Array element names for field Mixer4Matrix */
    typedef enum { MIXER4MATRIX_THROTTLECURVE1=0, MIXER4MATRIX_THROTTLECURVE2=1, MIXER4MATRIX_ROLL=2, MIXER4MATRIX_PITCH=3, MIXER4MATRIX_YAW=4 } Mixer4MatrixElem;
    /* Number of elements for field Mixer4Matrix */
    static const quint32 MIXER4MATRIX_NUMELEM = 5;
    // Field Mixer5Type information
    /* Enumeration options for field Mixer5Type */
    typedef enum { MIXER5TYPE_DISABLED=0, MIXER5TYPE_MOTOR=1, MIXER5TYPE_SERVO=2 } Mixer5TypeOptions;
    // Field Mixer5Matrix information
    /* Array element names for field Mixer5Matrix */
    typedef enum { MIXER5MATRIX_THROTTLECURVE1=0, MIXER5MATRIX_THROTTLECURVE2=1, MIXER5MATRIX_ROLL=2, MIXER5MATRIX_PITCH=3, MIXER5MATRIX_YAW=4 } Mixer5MatrixElem;
    /* Number of elements for field Mixer5Matrix */
    static const quint32 MIXER5MATRIX_NUMELEM = 5;
    // Field Mixer6Type information
    /* Enumeration options for field Mixer6Type */
    typedef enum { MIXER6TYPE_DISABLED=0, MIXER6TYPE_MOTOR=1, MIXER6TYPE_SERVO=2 } Mixer6TypeOptions;
    // Field Mixer6Matrix information
    /* Array element names for field Mixer6Matrix */
    typedef enum { MIXER6MATRIX_THROTTLECURVE1=0, MIXER6MATRIX_THROTTLECURVE2=1, MIXER6MATRIX_ROLL=2, MIXER6MATRIX_PITCH=3, MIXER6MATRIX_YAW=4 } Mixer6MatrixElem;
    /* Number of elements for field Mixer6Matrix */
    static const quint32 MIXER6MATRIX_NUMELEM = 5;
    // Field Mixer7Type information
    /* Enumeration options for field Mixer7Type */
    typedef enum { MIXER7TYPE_DISABLED=0, MIXER7TYPE_MOTOR=1, MIXER7TYPE_SERVO=2 } Mixer7TypeOptions;
    // Field Mixer7Matrix information
    /* Array element names for field Mixer7Matrix */
    typedef enum { MIXER7MATRIX_THROTTLECURVE1=0, MIXER7MATRIX_THROTTLECURVE2=1, MIXER7MATRIX_ROLL=2, MIXER7MATRIX_PITCH=3, MIXER7MATRIX_YAW=4 } Mixer7MatrixElem;
    /* Number of elements for field Mixer7Matrix */
    static const quint32 MIXER7MATRIX_NUMELEM = 5;

  
    // Constants
    static const quint32 OBJID = 1614220618U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 1;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    MixerSettings();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static MixerSettings* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // MIXERSETTINGS_H
