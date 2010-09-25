/**
 ******************************************************************************
 *
 * @file       ahrscalibration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: ahrscalibration.xml. 
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
#ifndef AHRSCALIBRATION_H
#define AHRSCALIBRATION_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT AHRSCalibration: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint8 measure_var;
        float accel_bias[3];
        float accel_scale[3];
        float accel_var[3];
        float gyro_bias[3];
        float gyro_scale[3];
        float gyro_var[3];
        float mag_bias[3];
        float mag_scale[3];
        float mag_var[3];

    } __attribute__((packed)) DataFields;

    // Field information
    // Field measure_var information
    /* Enumeration options for field measure_var */
    typedef enum { MEASURE_VAR_SET=0, MEASURE_VAR_MEASURE=1, MEASURE_VAR_ECHO=2 } measure_varOptions;
    // Field accel_bias information
    /* Array element names for field accel_bias */
    typedef enum { ACCEL_BIAS_X=0, ACCEL_BIAS_Y=1, ACCEL_BIAS_Z=2 } accel_biasElem;
    /* Number of elements for field accel_bias */
    static const quint32 ACCEL_BIAS_NUMELEM = 3;
    // Field accel_scale information
    /* Array element names for field accel_scale */
    typedef enum { ACCEL_SCALE_X=0, ACCEL_SCALE_Y=1, ACCEL_SCALE_Z=2 } accel_scaleElem;
    /* Number of elements for field accel_scale */
    static const quint32 ACCEL_SCALE_NUMELEM = 3;
    // Field accel_var information
    /* Array element names for field accel_var */
    typedef enum { ACCEL_VAR_X=0, ACCEL_VAR_Y=1, ACCEL_VAR_Z=2 } accel_varElem;
    /* Number of elements for field accel_var */
    static const quint32 ACCEL_VAR_NUMELEM = 3;
    // Field gyro_bias information
    /* Array element names for field gyro_bias */
    typedef enum { GYRO_BIAS_X=0, GYRO_BIAS_Y=1, GYRO_BIAS_Z=2 } gyro_biasElem;
    /* Number of elements for field gyro_bias */
    static const quint32 GYRO_BIAS_NUMELEM = 3;
    // Field gyro_scale information
    /* Array element names for field gyro_scale */
    typedef enum { GYRO_SCALE_X=0, GYRO_SCALE_Y=1, GYRO_SCALE_Z=2 } gyro_scaleElem;
    /* Number of elements for field gyro_scale */
    static const quint32 GYRO_SCALE_NUMELEM = 3;
    // Field gyro_var information
    /* Array element names for field gyro_var */
    typedef enum { GYRO_VAR_X=0, GYRO_VAR_Y=1, GYRO_VAR_Z=2 } gyro_varElem;
    /* Number of elements for field gyro_var */
    static const quint32 GYRO_VAR_NUMELEM = 3;
    // Field mag_bias information
    /* Array element names for field mag_bias */
    typedef enum { MAG_BIAS_X=0, MAG_BIAS_Y=1, MAG_BIAS_Z=2 } mag_biasElem;
    /* Number of elements for field mag_bias */
    static const quint32 MAG_BIAS_NUMELEM = 3;
    // Field mag_scale information
    /* Array element names for field mag_scale */
    typedef enum { MAG_SCALE_X=0, MAG_SCALE_Y=1, MAG_SCALE_Z=2 } mag_scaleElem;
    /* Number of elements for field mag_scale */
    static const quint32 MAG_SCALE_NUMELEM = 3;
    // Field mag_var information
    /* Array element names for field mag_var */
    typedef enum { MAG_VAR_X=0, MAG_VAR_Y=1, MAG_VAR_Z=2 } mag_varElem;
    /* Number of elements for field mag_var */
    static const quint32 MAG_VAR_NUMELEM = 3;

  
    // Constants
    static const quint32 OBJID = 1408636690U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 1;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    AHRSCalibration();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static AHRSCalibration* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // AHRSCALIBRATION_H
