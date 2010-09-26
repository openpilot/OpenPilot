/**
 ******************************************************************************
 *
 * @file       guidancesettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: guidancesettings.xml. 
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
#include "guidancesettings.h"
#include "uavobjectfield.h"

const QString GuidanceSettings::NAME = QString("GuidanceSettings");

/**
 * Constructor
 */
GuidanceSettings::GuidanceSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList MaxGroundspeedElemNames;
    MaxGroundspeedElemNames.append("0");
    fields.append( new UAVObjectField(QString("MaxGroundspeed"), QString("cm/s"), UAVObjectField::INT32, MaxGroundspeedElemNames, QStringList()) );
    QStringList GroundVelocityPElemNames;
    GroundVelocityPElemNames.append("0");
    fields.append( new UAVObjectField(QString("GroundVelocityP"), QString(""), UAVObjectField::FLOAT32, GroundVelocityPElemNames, QStringList()) );
    QStringList MaxVerticalSpeedElemNames;
    MaxVerticalSpeedElemNames.append("0");
    fields.append( new UAVObjectField(QString("MaxVerticalSpeed"), QString("cm/s"), UAVObjectField::INT32, MaxVerticalSpeedElemNames, QStringList()) );
    QStringList VertVelocityPElemNames;
    VertVelocityPElemNames.append("0");
    fields.append( new UAVObjectField(QString("VertVelocityP"), QString(""), UAVObjectField::FLOAT32, VertVelocityPElemNames, QStringList()) );
    QStringList VelPElemNames;
    VelPElemNames.append("0");
    fields.append( new UAVObjectField(QString("VelP"), QString(""), UAVObjectField::FLOAT32, VelPElemNames, QStringList()) );
    QStringList VelIElemNames;
    VelIElemNames.append("0");
    fields.append( new UAVObjectField(QString("VelI"), QString(""), UAVObjectField::FLOAT32, VelIElemNames, QStringList()) );
    QStringList VelDElemNames;
    VelDElemNames.append("0");
    fields.append( new UAVObjectField(QString("VelD"), QString(""), UAVObjectField::FLOAT32, VelDElemNames, QStringList()) );
    QStringList DownPElemNames;
    DownPElemNames.append("0");
    fields.append( new UAVObjectField(QString("DownP"), QString(""), UAVObjectField::FLOAT32, DownPElemNames, QStringList()) );
    QStringList DownIElemNames;
    DownIElemNames.append("0");
    fields.append( new UAVObjectField(QString("DownI"), QString(""), UAVObjectField::FLOAT32, DownIElemNames, QStringList()) );
    QStringList DownDElemNames;
    DownDElemNames.append("0");
    fields.append( new UAVObjectField(QString("DownD"), QString(""), UAVObjectField::FLOAT32, DownDElemNames, QStringList()) );
    QStringList MaxVelIntegralElemNames;
    MaxVelIntegralElemNames.append("0");
    fields.append( new UAVObjectField(QString("MaxVelIntegral"), QString("deg"), UAVObjectField::FLOAT32, MaxVelIntegralElemNames, QStringList()) );
    QStringList MaxThrottleIntegralElemNames;
    MaxThrottleIntegralElemNames.append("0");
    fields.append( new UAVObjectField(QString("MaxThrottleIntegral"), QString("deg"), UAVObjectField::FLOAT32, MaxThrottleIntegralElemNames, QStringList()) );
    QStringList VelUpdatePeriodElemNames;
    VelUpdatePeriodElemNames.append("0");
    fields.append( new UAVObjectField(QString("VelUpdatePeriod"), QString(""), UAVObjectField::INT32, VelUpdatePeriodElemNames, QStringList()) );
    QStringList VelPIDUpdatePeriodElemNames;
    VelPIDUpdatePeriodElemNames.append("0");
    fields.append( new UAVObjectField(QString("VelPIDUpdatePeriod"), QString(""), UAVObjectField::INT32, VelPIDUpdatePeriodElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata GuidanceSettings::getDefaultMetadata()
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
void GuidanceSettings::setDefaultFieldValues()
{
    data.MaxGroundspeed = 100;
    data.GroundVelocityP = 0.1;
    data.MaxVerticalSpeed = 100;
    data.VertVelocityP = 0.1;
    data.VelP = 0.1;
    data.VelI = 0.1;
    data.VelD = 0;
    data.DownP = 0;
    data.DownI = 0;
    data.DownD = 0;
    data.MaxVelIntegral = 2;
    data.MaxThrottleIntegral = 1;
    data.VelUpdatePeriod = 100;
    data.VelPIDUpdatePeriod = 20;

}

/**
 * Get the object data fields
 */
GuidanceSettings::DataFields GuidanceSettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void GuidanceSettings::setData(const DataFields& data)
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
UAVDataObject* GuidanceSettings::clone(quint32 instID)
{
    GuidanceSettings* obj = new GuidanceSettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
GuidanceSettings* GuidanceSettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<GuidanceSettings*>(objMngr->getObject(GuidanceSettings::OBJID, instID));
}
