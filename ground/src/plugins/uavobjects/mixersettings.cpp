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
    QStringList Mixer0TypeElemNames;
    Mixer0TypeElemNames.append("0");
    QStringList Mixer0TypeEnumOptions;
    Mixer0TypeEnumOptions.append("Disabled");
    Mixer0TypeEnumOptions.append("Motor");
    Mixer0TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer0Type"), QString(""), UAVObjectField::ENUM, Mixer0TypeElemNames, Mixer0TypeEnumOptions) );
    QStringList Mixer0MatrixElemNames;
    Mixer0MatrixElemNames.append("ThrottleCurve1");
    Mixer0MatrixElemNames.append("ThrottleCurve2");
    Mixer0MatrixElemNames.append("Roll");
    Mixer0MatrixElemNames.append("Pitch");
    Mixer0MatrixElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer0Matrix"), QString(""), UAVObjectField::FLOAT32, Mixer0MatrixElemNames, QStringList()) );
    QStringList Mixer1TypeElemNames;
    Mixer1TypeElemNames.append("0");
    QStringList Mixer1TypeEnumOptions;
    Mixer1TypeEnumOptions.append("Disabled");
    Mixer1TypeEnumOptions.append("Motor");
    Mixer1TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer1Type"), QString(""), UAVObjectField::ENUM, Mixer1TypeElemNames, Mixer1TypeEnumOptions) );
    QStringList Mixer1MatrixElemNames;
    Mixer1MatrixElemNames.append("ThrottleCurve1");
    Mixer1MatrixElemNames.append("ThrottleCurve2");
    Mixer1MatrixElemNames.append("Roll");
    Mixer1MatrixElemNames.append("Pitch");
    Mixer1MatrixElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer1Matrix"), QString(""), UAVObjectField::FLOAT32, Mixer1MatrixElemNames, QStringList()) );
    QStringList Mixer2TypeElemNames;
    Mixer2TypeElemNames.append("0");
    QStringList Mixer2TypeEnumOptions;
    Mixer2TypeEnumOptions.append("Disabled");
    Mixer2TypeEnumOptions.append("Motor");
    Mixer2TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer2Type"), QString(""), UAVObjectField::ENUM, Mixer2TypeElemNames, Mixer2TypeEnumOptions) );
    QStringList Mixer2MatrixElemNames;
    Mixer2MatrixElemNames.append("ThrottleCurve1");
    Mixer2MatrixElemNames.append("ThrottleCurve2");
    Mixer2MatrixElemNames.append("Roll");
    Mixer2MatrixElemNames.append("Pitch");
    Mixer2MatrixElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer2Matrix"), QString(""), UAVObjectField::FLOAT32, Mixer2MatrixElemNames, QStringList()) );
    QStringList Mixer3TypeElemNames;
    Mixer3TypeElemNames.append("0");
    QStringList Mixer3TypeEnumOptions;
    Mixer3TypeEnumOptions.append("Disabled");
    Mixer3TypeEnumOptions.append("Motor");
    Mixer3TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer3Type"), QString(""), UAVObjectField::ENUM, Mixer3TypeElemNames, Mixer3TypeEnumOptions) );
    QStringList Mixer3MatrixElemNames;
    Mixer3MatrixElemNames.append("ThrottleCurve1");
    Mixer3MatrixElemNames.append("ThrottleCurve2");
    Mixer3MatrixElemNames.append("Roll");
    Mixer3MatrixElemNames.append("Pitch");
    Mixer3MatrixElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer3Matrix"), QString(""), UAVObjectField::FLOAT32, Mixer3MatrixElemNames, QStringList()) );
    QStringList Mixer4TypeElemNames;
    Mixer4TypeElemNames.append("0");
    QStringList Mixer4TypeEnumOptions;
    Mixer4TypeEnumOptions.append("Disabled");
    Mixer4TypeEnumOptions.append("Motor");
    Mixer4TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer4Type"), QString(""), UAVObjectField::ENUM, Mixer4TypeElemNames, Mixer4TypeEnumOptions) );
    QStringList Mixer4MatrixElemNames;
    Mixer4MatrixElemNames.append("ThrottleCurve1");
    Mixer4MatrixElemNames.append("ThrottleCurve2");
    Mixer4MatrixElemNames.append("Roll");
    Mixer4MatrixElemNames.append("Pitch");
    Mixer4MatrixElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer4Matrix"), QString(""), UAVObjectField::FLOAT32, Mixer4MatrixElemNames, QStringList()) );
    QStringList Mixer5TypeElemNames;
    Mixer5TypeElemNames.append("0");
    QStringList Mixer5TypeEnumOptions;
    Mixer5TypeEnumOptions.append("Disabled");
    Mixer5TypeEnumOptions.append("Motor");
    Mixer5TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer5Type"), QString(""), UAVObjectField::ENUM, Mixer5TypeElemNames, Mixer5TypeEnumOptions) );
    QStringList Mixer5MatrixElemNames;
    Mixer5MatrixElemNames.append("ThrottleCurve1");
    Mixer5MatrixElemNames.append("ThrottleCurve2");
    Mixer5MatrixElemNames.append("Roll");
    Mixer5MatrixElemNames.append("Pitch");
    Mixer5MatrixElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer5Matrix"), QString(""), UAVObjectField::FLOAT32, Mixer5MatrixElemNames, QStringList()) );
    QStringList Mixer6TypeElemNames;
    Mixer6TypeElemNames.append("0");
    QStringList Mixer6TypeEnumOptions;
    Mixer6TypeEnumOptions.append("Disabled");
    Mixer6TypeEnumOptions.append("Motor");
    Mixer6TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer6Type"), QString(""), UAVObjectField::ENUM, Mixer6TypeElemNames, Mixer6TypeEnumOptions) );
    QStringList Mixer6MatrixElemNames;
    Mixer6MatrixElemNames.append("ThrottleCurve1");
    Mixer6MatrixElemNames.append("ThrottleCurve2");
    Mixer6MatrixElemNames.append("Roll");
    Mixer6MatrixElemNames.append("Pitch");
    Mixer6MatrixElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer6Matrix"), QString(""), UAVObjectField::FLOAT32, Mixer6MatrixElemNames, QStringList()) );
    QStringList Mixer7TypeElemNames;
    Mixer7TypeElemNames.append("0");
    QStringList Mixer7TypeEnumOptions;
    Mixer7TypeEnumOptions.append("Disabled");
    Mixer7TypeEnumOptions.append("Motor");
    Mixer7TypeEnumOptions.append("Servo");
    fields.append( new UAVObjectField(QString("Mixer7Type"), QString(""), UAVObjectField::ENUM, Mixer7TypeElemNames, Mixer7TypeEnumOptions) );
    QStringList Mixer7MatrixElemNames;
    Mixer7MatrixElemNames.append("ThrottleCurve1");
    Mixer7MatrixElemNames.append("ThrottleCurve2");
    Mixer7MatrixElemNames.append("Roll");
    Mixer7MatrixElemNames.append("Pitch");
    Mixer7MatrixElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("Mixer7Matrix"), QString(""), UAVObjectField::FLOAT32, Mixer7MatrixElemNames, QStringList()) );

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
    data.ThrottleCurve1[0] = -10;
    data.ThrottleCurve1[1] = -10;
    data.ThrottleCurve1[2] = -10;
    data.ThrottleCurve1[3] = -10;
    data.ThrottleCurve1[4] = -10;
    data.ThrottleCurve2[0] = -10;
    data.ThrottleCurve2[1] = -10;
    data.ThrottleCurve2[2] = -10;
    data.ThrottleCurve2[3] = -10;
    data.ThrottleCurve2[4] = -10;
    data.Mixer0Type = 0;
    data.Mixer0Matrix[0] = 0;
    data.Mixer0Matrix[1] = 0;
    data.Mixer0Matrix[2] = 0;
    data.Mixer0Matrix[3] = 0;
    data.Mixer0Matrix[4] = 0;
    data.Mixer1Type = 0;
    data.Mixer1Matrix[0] = 0;
    data.Mixer1Matrix[1] = 0;
    data.Mixer1Matrix[2] = 0;
    data.Mixer1Matrix[3] = 0;
    data.Mixer1Matrix[4] = 0;
    data.Mixer2Type = 0;
    data.Mixer2Matrix[0] = 0;
    data.Mixer2Matrix[1] = 0;
    data.Mixer2Matrix[2] = 0;
    data.Mixer2Matrix[3] = 0;
    data.Mixer2Matrix[4] = 0;
    data.Mixer3Type = 0;
    data.Mixer3Matrix[0] = 0;
    data.Mixer3Matrix[1] = 0;
    data.Mixer3Matrix[2] = 0;
    data.Mixer3Matrix[3] = 0;
    data.Mixer3Matrix[4] = 0;
    data.Mixer4Type = 0;
    data.Mixer4Matrix[0] = 0;
    data.Mixer4Matrix[1] = 0;
    data.Mixer4Matrix[2] = 0;
    data.Mixer4Matrix[3] = 0;
    data.Mixer4Matrix[4] = 0;
    data.Mixer5Type = 0;
    data.Mixer5Matrix[0] = 0;
    data.Mixer5Matrix[1] = 0;
    data.Mixer5Matrix[2] = 0;
    data.Mixer5Matrix[3] = 0;
    data.Mixer5Matrix[4] = 0;
    data.Mixer6Type = 0;
    data.Mixer6Matrix[0] = 0;
    data.Mixer6Matrix[1] = 0;
    data.Mixer6Matrix[2] = 0;
    data.Mixer6Matrix[3] = 0;
    data.Mixer6Matrix[4] = 0;
    data.Mixer7Type = 0;
    data.Mixer7Matrix[0] = 0;
    data.Mixer7Matrix[1] = 0;
    data.Mixer7Matrix[2] = 0;
    data.Mixer7Matrix[3] = 0;
    data.Mixer7Matrix[4] = 0;

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
