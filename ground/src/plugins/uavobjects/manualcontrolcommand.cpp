/**
 ******************************************************************************
 *
 * @file       manualcontrolcommand.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: manualcontrolcommand.xml. 
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
#include "manualcontrolcommand.h"
#include "uavobjectfield.h"

const QString ManualControlCommand::NAME = QString("ManualControlCommand");

/**
 * Constructor
 */
ManualControlCommand::ManualControlCommand(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList ConnectedElemNames;
    ConnectedElemNames.append("0");
    QStringList ConnectedEnumOptions;
    ConnectedEnumOptions.append("False");
    ConnectedEnumOptions.append("True");
    fields.append( new UAVObjectField(QString("Connected"), QString(""), UAVObjectField::ENUM, ConnectedElemNames, ConnectedEnumOptions) );
    QStringList ArmedElemNames;
    ArmedElemNames.append("0");
    QStringList ArmedEnumOptions;
    ArmedEnumOptions.append("False");
    ArmedEnumOptions.append("True");
    fields.append( new UAVObjectField(QString("Armed"), QString(""), UAVObjectField::ENUM, ArmedElemNames, ArmedEnumOptions) );
    QStringList RollElemNames;
    RollElemNames.append("0");
    fields.append( new UAVObjectField(QString("Roll"), QString("%"), UAVObjectField::FLOAT32, RollElemNames, QStringList()) );
    QStringList PitchElemNames;
    PitchElemNames.append("0");
    fields.append( new UAVObjectField(QString("Pitch"), QString("%"), UAVObjectField::FLOAT32, PitchElemNames, QStringList()) );
    QStringList YawElemNames;
    YawElemNames.append("0");
    fields.append( new UAVObjectField(QString("Yaw"), QString("%"), UAVObjectField::FLOAT32, YawElemNames, QStringList()) );
    QStringList ThrottleElemNames;
    ThrottleElemNames.append("0");
    fields.append( new UAVObjectField(QString("Throttle"), QString("%"), UAVObjectField::FLOAT32, ThrottleElemNames, QStringList()) );
    QStringList FlightModeElemNames;
    FlightModeElemNames.append("0");
    QStringList FlightModeEnumOptions;
    FlightModeEnumOptions.append("Manual");
    FlightModeEnumOptions.append("Stabilized");
    FlightModeEnumOptions.append("Auto");
    fields.append( new UAVObjectField(QString("FlightMode"), QString(""), UAVObjectField::ENUM, FlightModeElemNames, FlightModeEnumOptions) );
    QStringList StabilizationSettingsElemNames;
    StabilizationSettingsElemNames.append("Roll");
    StabilizationSettingsElemNames.append("Pitch");
    StabilizationSettingsElemNames.append("Yaw");
    QStringList StabilizationSettingsEnumOptions;
    StabilizationSettingsEnumOptions.append("None");
    StabilizationSettingsEnumOptions.append("Rate");
    StabilizationSettingsEnumOptions.append("Position");
    fields.append( new UAVObjectField(QString("StabilizationSettings"), QString(""), UAVObjectField::ENUM, StabilizationSettingsElemNames, StabilizationSettingsEnumOptions) );
    QStringList Accessory1ElemNames;
    Accessory1ElemNames.append("0");
    fields.append( new UAVObjectField(QString("Accessory1"), QString("%"), UAVObjectField::FLOAT32, Accessory1ElemNames, QStringList()) );
    QStringList Accessory2ElemNames;
    Accessory2ElemNames.append("0");
    fields.append( new UAVObjectField(QString("Accessory2"), QString("%"), UAVObjectField::FLOAT32, Accessory2ElemNames, QStringList()) );
    QStringList Accessory3ElemNames;
    Accessory3ElemNames.append("0");
    fields.append( new UAVObjectField(QString("Accessory3"), QString("%"), UAVObjectField::FLOAT32, Accessory3ElemNames, QStringList()) );
    QStringList ChannelElemNames;
    ChannelElemNames.append("0");
    ChannelElemNames.append("1");
    ChannelElemNames.append("2");
    ChannelElemNames.append("3");
    ChannelElemNames.append("4");
    ChannelElemNames.append("5");
    ChannelElemNames.append("6");
    ChannelElemNames.append("7");
    fields.append( new UAVObjectField(QString("Channel"), QString("us"), UAVObjectField::INT16, ChannelElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata ManualControlCommand::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 0;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 0;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.flightTelemetryUpdatePeriod = 2000;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_NEVER;
    metadata.loggingUpdatePeriod = 0;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void ManualControlCommand::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
ManualControlCommand::DataFields ManualControlCommand::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void ManualControlCommand::setData(const DataFields& data)
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
UAVDataObject* ManualControlCommand::clone(quint32 instID)
{
    ManualControlCommand* obj = new ManualControlCommand();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
ManualControlCommand* ManualControlCommand::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<ManualControlCommand*>(objMngr->getObject(ManualControlCommand::OBJID, instID));
}
