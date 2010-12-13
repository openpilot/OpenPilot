/**
 ******************************************************************************
 *
 * @file       mixersettings.cpp
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
#include "mixersettings.h"
#include "uavobjectfield.h"

const QString MixerSettings::NAME = QString("MixerSettings");

/**
 * Constructor
 */
MixerSettings::MixerSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList MaxAccelElemNames;
    MaxAccelElemNames.append("0");
    fields.append( new UAVObjectField(QString("MaxAccel"), QString("units/sec"), UAVObjectField::FLOAT32, MaxAccelElemNames, QStringList()) );
    QStringList FeedForwardElemNames;
    FeedForwardElemNames.append("0");
    fields.append( new UAVObjectField(QString("FeedForward"), QString(""), UAVObjectField::FLOAT32, FeedForwardElemNames, QStringList()) );
    QStringList AccelTimeElemNames;
    AccelTimeElemNames.append("0");
    fields.append( new UAVObjectField(QString("AccelTime"), QString("ms"), UAVObjectField::FLOAT32, AccelTimeElemNames, QStringList()) );
    QStringList DecelTimeElemNames;
    DecelTimeElemNames.append("0");
    fields.append( new UAVObjectField(QString("DecelTime"), QString("ms"), UAVObjectField::FLOAT32, DecelTimeElemNames, QStringList()) );
    QStringList ThrottleCurve1ElemNames;
    ThrottleCurve1ElemNames.append("0");
    ThrottleCurve1ElemNames.append("25");
    ThrottleCurve1ElemNames.append("50");
    ThrottleCurve1ElemNames.append("75");
    ThrottleCurve1ElemNames.append("100");
    fields.append( new UAVObjectField(QString("ThrottleCurve1"), QString("percent"), UAVObjectField::FLOAT32, ThrottleCurve1ElemNames, QStringList()) );
    QStringList ThrottleCurve2ElemNames;
    ThrottleCurve2ElemNames.append("0");
    ThrottleCurve2ElemNames.append("25");
    ThrottleCurve2ElemNames.append("50");
    ThrottleCurve2ElemNames.append("75");
    ThrottleCurve2ElemNames.append("100");
    fields.append( new UAVObjectField(QString("ThrottleCurve2"), QString("percent"), UAVObjectField::FLOAT32, ThrottleCurve2ElemNames, QStringList()) );
    QStringList Mixer1TypeElemNames;
    Mixer1TypeElemNames.append("0");
    QStringList Mixer1TypeEnumOptions;
    Mixer1TypeEnumOptions.append("Disabled");
    Mixer1TypeEnumOptions.append("Motor");
    Mixer1TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer1Type"), QString(""), UAVObjectField::ENUM, Mixer1TypeElemNames, Mixer1TypeEnumOptions) );
    QStringList Mixer1VectorElemNames;
    Mixer1VectorElemNames.append("ThrottleCurve1");
    Mixer1VectorElemNames.append("ThrottleCurve2");
    Mixer1VectorElemNames.append("Roll");
    Mixer1VectorElemNames.append("Pitch");
    Mixer1VectorElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer1Vector"), QString(""), UAVObjectField::INT8, Mixer1VectorElemNames, QStringList()) );
    QStringList Mixer2TypeElemNames;
    Mixer2TypeElemNames.append("0");
    QStringList Mixer2TypeEnumOptions;
    Mixer2TypeEnumOptions.append("Disabled");
    Mixer2TypeEnumOptions.append("Motor");
    Mixer2TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer2Type"), QString(""), UAVObjectField::ENUM, Mixer2TypeElemNames, Mixer2TypeEnumOptions) );
    QStringList Mixer2VectorElemNames;
    Mixer2VectorElemNames.append("ThrottleCurve1");
    Mixer2VectorElemNames.append("ThrottleCurve2");
    Mixer2VectorElemNames.append("Roll");
    Mixer2VectorElemNames.append("Pitch");
    Mixer2VectorElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer2Vector"), QString(""), UAVObjectField::INT8, Mixer2VectorElemNames, QStringList()) );
    QStringList Mixer3TypeElemNames;
    Mixer3TypeElemNames.append("0");
    QStringList Mixer3TypeEnumOptions;
    Mixer3TypeEnumOptions.append("Disabled");
    Mixer3TypeEnumOptions.append("Motor");
    Mixer3TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer3Type"), QString(""), UAVObjectField::ENUM, Mixer3TypeElemNames, Mixer3TypeEnumOptions) );
    QStringList Mixer3VectorElemNames;
    Mixer3VectorElemNames.append("ThrottleCurve1");
    Mixer3VectorElemNames.append("ThrottleCurve2");
    Mixer3VectorElemNames.append("Roll");
    Mixer3VectorElemNames.append("Pitch");
    Mixer3VectorElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer3Vector"), QString(""), UAVObjectField::INT8, Mixer3VectorElemNames, QStringList()) );
    QStringList Mixer4TypeElemNames;
    Mixer4TypeElemNames.append("0");
    QStringList Mixer4TypeEnumOptions;
    Mixer4TypeEnumOptions.append("Disabled");
    Mixer4TypeEnumOptions.append("Motor");
    Mixer4TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer4Type"), QString(""), UAVObjectField::ENUM, Mixer4TypeElemNames, Mixer4TypeEnumOptions) );
    QStringList Mixer4VectorElemNames;
    Mixer4VectorElemNames.append("ThrottleCurve1");
    Mixer4VectorElemNames.append("ThrottleCurve2");
    Mixer4VectorElemNames.append("Roll");
    Mixer4VectorElemNames.append("Pitch");
    Mixer4VectorElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer4Vector"), QString(""), UAVObjectField::INT8, Mixer4VectorElemNames, QStringList()) );
    QStringList Mixer5TypeElemNames;
    Mixer5TypeElemNames.append("0");
    QStringList Mixer5TypeEnumOptions;
    Mixer5TypeEnumOptions.append("Disabled");
    Mixer5TypeEnumOptions.append("Motor");
    Mixer5TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer5Type"), QString(""), UAVObjectField::ENUM, Mixer5TypeElemNames, Mixer5TypeEnumOptions) );
    QStringList Mixer5VectorElemNames;
    Mixer5VectorElemNames.append("ThrottleCurve1");
    Mixer5VectorElemNames.append("ThrottleCurve2");
    Mixer5VectorElemNames.append("Roll");
    Mixer5VectorElemNames.append("Pitch");
    Mixer5VectorElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer5Vector"), QString(""), UAVObjectField::INT8, Mixer5VectorElemNames, QStringList()) );
    QStringList Mixer6TypeElemNames;
    Mixer6TypeElemNames.append("0");
    QStringList Mixer6TypeEnumOptions;
    Mixer6TypeEnumOptions.append("Disabled");
    Mixer6TypeEnumOptions.append("Motor");
    Mixer6TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer6Type"), QString(""), UAVObjectField::ENUM, Mixer6TypeElemNames, Mixer6TypeEnumOptions) );
    QStringList Mixer6VectorElemNames;
    Mixer6VectorElemNames.append("ThrottleCurve1");
    Mixer6VectorElemNames.append("ThrottleCurve2");
    Mixer6VectorElemNames.append("Roll");
    Mixer6VectorElemNames.append("Pitch");
    Mixer6VectorElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer6Vector"), QString(""), UAVObjectField::INT8, Mixer6VectorElemNames, QStringList()) );
    QStringList Mixer7TypeElemNames;
    Mixer7TypeElemNames.append("0");
    QStringList Mixer7TypeEnumOptions;
    Mixer7TypeEnumOptions.append("Disabled");
    Mixer7TypeEnumOptions.append("Motor");
    Mixer7TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer7Type"), QString(""), UAVObjectField::ENUM, Mixer7TypeElemNames, Mixer7TypeEnumOptions) );
    QStringList Mixer7VectorElemNames;
    Mixer7VectorElemNames.append("ThrottleCurve1");
    Mixer7VectorElemNames.append("ThrottleCurve2");
    Mixer7VectorElemNames.append("Roll");
    Mixer7VectorElemNames.append("Pitch");
    Mixer7VectorElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer7Vector"), QString(""), UAVObjectField::INT8, Mixer7VectorElemNames, QStringList()) );
    QStringList Mixer8TypeElemNames;
    Mixer8TypeElemNames.append("0");
    QStringList Mixer8TypeEnumOptions;
    Mixer8TypeEnumOptions.append("Disabled");
    Mixer8TypeEnumOptions.append("Motor");
    Mixer8TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer8Type"), QString(""), UAVObjectField::ENUM, Mixer8TypeElemNames, Mixer8TypeEnumOptions) );
    QStringList Mixer8VectorElemNames;
    Mixer8VectorElemNames.append("ThrottleCurve1");
    Mixer8VectorElemNames.append("ThrottleCurve2");
    Mixer8VectorElemNames.append("Roll");
    Mixer8VectorElemNames.append("Pitch");
    Mixer8VectorElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer8Vector"), QString(""), UAVObjectField::INT8, Mixer8VectorElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata MixerSettings::getDefaultMetadata()
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
void MixerSettings::setDefaultFieldValues()
{
    data.MaxAccel = 1000;
    data.FeedForward = 0;
    data.AccelTime = 0;
    data.DecelTime = 0;
    data.ThrottleCurve1[0] = 0;
    data.ThrottleCurve1[1] = 0.25;
    data.ThrottleCurve1[2] = 0.5;
    data.ThrottleCurve1[3] = 0.75;
    data.ThrottleCurve1[4] = 1;
    data.ThrottleCurve2[0] = 0;
    data.ThrottleCurve2[1] = 0.25;
    data.ThrottleCurve2[2] = 0.5;
    data.ThrottleCurve2[3] = 0.75;
    data.ThrottleCurve2[4] = 1;
    data.Mixer1Type = 0;
    data.Mixer1Vector[0] = 0;
    data.Mixer1Vector[1] = 0;
    data.Mixer1Vector[2] = 0;
    data.Mixer1Vector[3] = 0;
    data.Mixer1Vector[4] = 0;
    data.Mixer2Type = 0;
    data.Mixer2Vector[0] = 0;
    data.Mixer2Vector[1] = 0;
    data.Mixer2Vector[2] = 0;
    data.Mixer2Vector[3] = 0;
    data.Mixer2Vector[4] = 0;
    data.Mixer3Type = 0;
    data.Mixer3Vector[0] = 0;
    data.Mixer3Vector[1] = 0;
    data.Mixer3Vector[2] = 0;
    data.Mixer3Vector[3] = 0;
    data.Mixer3Vector[4] = 0;
    data.Mixer4Type = 0;
    data.Mixer4Vector[0] = 0;
    data.Mixer4Vector[1] = 0;
    data.Mixer4Vector[2] = 0;
    data.Mixer4Vector[3] = 0;
    data.Mixer4Vector[4] = 0;
    data.Mixer5Type = 0;
    data.Mixer5Vector[0] = 0;
    data.Mixer5Vector[1] = 0;
    data.Mixer5Vector[2] = 0;
    data.Mixer5Vector[3] = 0;
    data.Mixer5Vector[4] = 0;
    data.Mixer6Type = 0;
    data.Mixer6Vector[0] = 0;
    data.Mixer6Vector[1] = 0;
    data.Mixer6Vector[2] = 0;
    data.Mixer6Vector[3] = 0;
    data.Mixer6Vector[4] = 0;
    data.Mixer7Type = 0;
    data.Mixer7Vector[0] = 0;
    data.Mixer7Vector[1] = 0;
    data.Mixer7Vector[2] = 0;
    data.Mixer7Vector[3] = 0;
    data.Mixer7Vector[4] = 0;
    data.Mixer8Type = 0;
    data.Mixer8Vector[0] = 0;
    data.Mixer8Vector[1] = 0;
    data.Mixer8Vector[2] = 0;
    data.Mixer8Vector[3] = 0;
    data.Mixer8Vector[4] = 0;

}

/**
 * Get the object data fields
 */
MixerSettings::DataFields MixerSettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void MixerSettings::setData(const DataFields& data)
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
UAVDataObject* MixerSettings::clone(quint32 instID)
{
    MixerSettings* obj = new MixerSettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
MixerSettings* MixerSettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<MixerSettings*>(objMngr->getObject(MixerSettings::OBJID, instID));
}
