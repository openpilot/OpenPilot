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
        qint8 Mixer0Vector[5];
        quint8 Mixer1Type;
        qint8 Mixer1Vector[5];
        quint8 Mixer2Type;
        qint8 Mixer2Vector[5];
        quint8 Mixer3Type;
        qint8 Mixer3Vector[5];
        quint8 Mixer4Type;
        qint8 Mixer4Vector[5];
        quint8 Mixer5Type;
        qint8 Mixer5Vector[5];
        quint8 Mixer6Type;
        qint8 Mixer6Vector[5];
        quint8 Mixer7Type;
        qint8 Mixer7Vector[5];

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
    // Field Mixer0Vector information
    /* Array element names for field Mixer0Vector */
    typedef enum { MIXER0VECTOR_THROTTLECURVE1=0, MIXER0VECTOR_THROTTLECURVE2=1, MIXER0VECTOR_ROLL=2, MIXER0VECTOR_PITCH=3, MIXER0VECTOR_YAW=4 } Mixer0VectorElem;
    /* Number of elements for field Mixer0Vector */
    static const quint32 MIXER0VECTOR_NUMELEM = 5;
    // Field Mixer1Type information
    /* Enumeration options for field Mixer1Type */
    typedef enum { MIXER1TYPE_DISABLED=0, MIXER1TYPE_MOTOR=1, MIXER1TYPE_SERVO=2 } Mixer1TypeOptions;
    // Field Mixer1Vector information
    /* Array element names for field Mixer1Vector */
    typedef enum { MIXER1VECTOR_THROTTLECURVE1=0, MIXER1VECTOR_THROTTLECURVE2=1, MIXER1VECTOR_ROLL=2, MIXER1VECTOR_PITCH=3, MIXER1VECTOR_YAW=4 } Mixer1VectorElem;
    /* Number of elements for field Mixer1Vector */
    static const quint32 MIXER1VECTOR_NUMELEM = 5;
    // Field Mixer2Type information
    /* Enumeration options for field Mixer2Type */
    typedef enum { MIXER2TYPE_DISABLED=0, MIXER2TYPE_MOTOR=1, MIXER2TYPE_SERVO=2 } Mixer2TypeOptions;
    // Field Mixer2Vector information
    /* Array element names for field Mixer2Vector */
    typedef enum { MIXER2VECTOR_THROTTLECURVE1=0, MIXER2VECTOR_THROTTLECURVE2=1, MIXER2VECTOR_ROLL=2, MIXER2VECTOR_PITCH=3, MIXER2VECTOR_YAW=4 } Mixer2VectorElem;
    /* Number of elements for field Mixer2Vector */
    static const quint32 MIXER2VECTOR_NUMELEM = 5;
    // Field Mixer3Type information
    /* Enumeration options for field Mixer3Type */
    typedef enum { MIXER3TYPE_DISABLED=0, MIXER3TYPE_MOTOR=1, MIXER3TYPE_SERVO=2 } Mixer3TypeOptions;
    // Field Mixer3Vector information
    /* Array element names for field Mixer3Vector */
    typedef enum { MIXER3VECTOR_THROTTLECURVE1=0, MIXER3VECTOR_THROTTLECURVE2=1, MIXER3VECTOR_ROLL=2, MIXER3VECTOR_PITCH=3, MIXER3VECTOR_YAW=4 } Mixer3VectorElem;
    /* Number of elements for field Mixer3Vector */
    static const quint32 MIXER3VECTOR_NUMELEM = 5;
    // Field Mixer4Type information
    /* Enumeration options for field Mixer4Type */
    typedef enum { MIXER4TYPE_DISABLED=0, MIXER4TYPE_MOTOR=1, MIXER4TYPE_SERVO=2 } Mixer4TypeOptions;
    // Field Mixer4Vector information
    /* Array element names for field Mixer4Vector */
    typedef enum { MIXER4VECTOR_THROTTLECURVE1=0, MIXER4VECTOR_THROTTLECURVE2=1, MIXER4VECTOR_ROLL=2, MIXER4VECTOR_PITCH=3, MIXER4VECTOR_YAW=4 } Mixer4VectorElem;
    /* Number of elements for field Mixer4Vector */
    static const quint32 MIXER4VECTOR_NUMELEM = 5;
    // Field Mixer5Type information
    /* Enumeration options for field Mixer5Type */
    typedef enum { MIXER5TYPE_DISABLED=0, MIXER5TYPE_MOTOR=1, MIXER5TYPE_SERVO=2 } Mixer5TypeOptions;
    // Field Mixer5Vector information
    /* Array element names for field Mixer5Vector */
    typedef enum { MIXER5VECTOR_THROTTLECURVE1=0, MIXER5VECTOR_THROTTLECURVE2=1, MIXER5VECTOR_ROLL=2, MIXER5VECTOR_PITCH=3, MIXER5VECTOR_YAW=4 } Mixer5VectorElem;
    /* Number of elements for field Mixer5Vector */
    static const quint32 MIXER5VECTOR_NUMELEM = 5;
    // Field Mixer6Type information
    /* Enumeration options for field Mixer6Type */
    typedef enum { MIXER6TYPE_DISABLED=0, MIXER6TYPE_MOTOR=1, MIXER6TYPE_SERVO=2 } Mixer6TypeOptions;
    // Field Mixer6Vector information
    /* Array element names for field Mixer6Vector */
    typedef enum { MIXER6VECTOR_THROTTLECURVE1=0, MIXER6VECTOR_THROTTLECURVE2=1, MIXER6VECTOR_ROLL=2, MIXER6VECTOR_PITCH=3, MIXER6VECTOR_YAW=4 } Mixer6VectorElem;
    /* Number of elements for field Mixer6Vector */
    static const quint32 MIXER6VECTOR_NUMELEM = 5;
    // Field Mixer7Type information
    /* Enumeration options for field Mixer7Type */
    typedef enum { MIXER7TYPE_DISABLED=0, MIXER7TYPE_MOTOR=1, MIXER7TYPE_SERVO=2 } Mixer7TypeOptions;
    // Field Mixer7Vector information
    /* Array element names for field Mixer7Vector */
    typedef enum { MIXER7VECTOR_THROTTLECURVE1=0, MIXER7VECTOR_THROTTLECURVE2=1, MIXER7VECTOR_ROLL=2, MIXER7VECTOR_PITCH=3, MIXER7VECTOR_YAW=4 } Mixer7VectorElem;
    /* Number of elements for field Mixer7Vector */
    static const quint32 MIXER7VECTOR_NUMELEM = 5;

  
    // Constants
    static const quint32 OBJID = 1945801048U;
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
