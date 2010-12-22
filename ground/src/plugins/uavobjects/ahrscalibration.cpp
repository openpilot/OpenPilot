/**
 ******************************************************************************
 *
 * @file       ahrscalibration.cpp
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
#include "ahrscalibration.h"
#include "uavobjectfield.h"

const QString AHRSCalibration::NAME = QString("AHRSCalibration");
const QString AHRSCalibration::DESCRIPTION = QString("Contains the calibration settings for the @ref AHRSCommsModule");

/**
 * Constructor
 */
AHRSCalibration::AHRSCalibration(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList measure_varElemNames;
    measure_varElemNames.append("0");
    QStringList measure_varEnumOptions;
    measure_varEnumOptions.append("SET");
    measure_varEnumOptions.append("MEASURE");
    fields.append( new UAVObjectField(QString("measure_var"), QString(""), UAVObjectField::ENUM, measure_varElemNames, measure_varEnumOptions) );
    QStringList accel_biasElemNames;
    accel_biasElemNames.append("X");
    accel_biasElemNames.append("Y");
    accel_biasElemNames.append("Z");
    fields.append( new UAVObjectField(QString("accel_bias"), QString("m/s"), UAVObjectField::FLOAT32, accel_biasElemNames, QStringList()) );
    QStringList accel_scaleElemNames;
    accel_scaleElemNames.append("X");
    accel_scaleElemNames.append("Y");
    accel_scaleElemNames.append("Z");
    fields.append( new UAVObjectField(QString("accel_scale"), QString("m/s"), UAVObjectField::FLOAT32, accel_scaleElemNames, QStringList()) );
    QStringList accel_varElemNames;
    accel_varElemNames.append("X");
    accel_varElemNames.append("Y");
    accel_varElemNames.append("Z");
    fields.append( new UAVObjectField(QString("accel_var"), QString("m^2/s^s"), UAVObjectField::FLOAT32, accel_varElemNames, QStringList()) );
    QStringList gyro_biasElemNames;
    gyro_biasElemNames.append("X");
    gyro_biasElemNames.append("Y");
    gyro_biasElemNames.append("Z");
    fields.append( new UAVObjectField(QString("gyro_bias"), QString("deg/s"), UAVObjectField::FLOAT32, gyro_biasElemNames, QStringList()) );
    QStringList gyro_scaleElemNames;
    gyro_scaleElemNames.append("X");
    gyro_scaleElemNames.append("Y");
    gyro_scaleElemNames.append("Z");
    fields.append( new UAVObjectField(QString("gyro_scale"), QString("deg/s"), UAVObjectField::FLOAT32, gyro_scaleElemNames, QStringList()) );
    QStringList gyro_varElemNames;
    gyro_varElemNames.append("X");
    gyro_varElemNames.append("Y");
    gyro_varElemNames.append("Z");
    fields.append( new UAVObjectField(QString("gyro_var"), QString("deg^s/s^2"), UAVObjectField::FLOAT32, gyro_varElemNames, QStringList()) );
    QStringList mag_biasElemNames;
    mag_biasElemNames.append("X");
    mag_biasElemNames.append("Y");
    mag_biasElemNames.append("Z");
    fields.append( new UAVObjectField(QString("mag_bias"), QString("mGau"), UAVObjectField::FLOAT32, mag_biasElemNames, QStringList()) );
    QStringList mag_scaleElemNames;
    mag_scaleElemNames.append("X");
    mag_scaleElemNames.append("Y");
    mag_scaleElemNames.append("Z");
    fields.append( new UAVObjectField(QString("mag_scale"), QString("mGau"), UAVObjectField::FLOAT32, mag_scaleElemNames, QStringList()) );
    QStringList mag_varElemNames;
    mag_varElemNames.append("X");
    mag_varElemNames.append("Y");
    mag_varElemNames.append("Z");
    fields.append( new UAVObjectField(QString("mag_var"), QString("mGau^s"), UAVObjectField::FLOAT32, mag_varElemNames, QStringList()) );
    QStringList vel_varElemNames;
    vel_varElemNames.append("0");
    fields.append( new UAVObjectField(QString("vel_var"), QString("(m/s)^2"), UAVObjectField::FLOAT32, vel_varElemNames, QStringList()) );
    QStringList pos_varElemNames;
    pos_varElemNames.append("0");
    fields.append( new UAVObjectField(QString("pos_var"), QString("m^2"), UAVObjectField::FLOAT32, pos_varElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
    // Set the object description
    setDescription(DESCRIPTION);
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata AHRSCalibration::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 1;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 1;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
    metadata.flightTelemetryUpdatePeriod = 0;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_NEVER;
    metadata.loggingUpdatePeriod = 0;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void AHRSCalibration::setDefaultFieldValues()
{
    data.measure_var = 0;
    data.accel_bias[0] = -72;
    data.accel_bias[1] = -72;
    data.accel_bias[2] = 72;
    data.accel_scale[0] = 0.003;
    data.accel_scale[1] = 0.003;
    data.accel_scale[2] = -0.003;
    data.accel_var[0] = 5e-05;
    data.accel_var[1] = 5e-05;
    data.accel_var[2] = 5e-05;
    data.gyro_bias[0] = 23;
    data.gyro_bias[1] = -23;
    data.gyro_bias[2] = 23;
    data.gyro_scale[0] = -0.014;
    data.gyro_scale[1] = 0.014;
    data.gyro_scale[2] = -0.014;
    data.gyro_var[0] = 0.0001;
    data.gyro_var[1] = 0.0001;
    data.gyro_var[2] = 0.0001;
    data.mag_bias[0] = 0;
    data.mag_bias[1] = 0;
    data.mag_bias[2] = 0;
    data.mag_scale[0] = 1;
    data.mag_scale[1] = 1;
    data.mag_scale[2] = 1;
    data.mag_var[0] = 5e-05;
    data.mag_var[1] = 5e-05;
    data.mag_var[2] = 5e-05;
    data.vel_var = 0.4;
    data.pos_var = 0.4;

}

/**
 * Get the object data fields
 */
AHRSCalibration::DataFields AHRSCalibration::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void AHRSCalibration::setData(const DataFields& data)
{
    QMutexLocker locker(mutex);
    // Get metadata
    Metadata mdata = getMetadata();
    // Update object if the access mode permits
    if ( mdata.gcsAccess == ACCESS_READWRITE )
    {
        this->data = data;
        emit objectUpdatedAuto(this); // trigger object updated event
        emit objectUpdated(this);
    }
}

/**
 * Create a clone of this object, a new instance ID must be specified.
 * Do not use this function directly to create new instances, the
 * UAVObjectManager should be used instead.
 */
UAVDataObject* AHRSCalibration::clone(quint32 instID)
{
    AHRSCalibration* obj = new AHRSCalibration();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
AHRSCalibration* AHRSCalibration::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<AHRSCalibration*>(objMngr->getObject(AHRSCalibration::OBJID, instID));
}
