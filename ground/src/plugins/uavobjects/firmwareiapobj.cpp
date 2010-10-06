/**
 ******************************************************************************
 *
 * @file       firmwareiapobj.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: firmwareiap.xml. 
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
#include "firmwareiapobj.h"
#include "uavobjectfield.h"

const QString FirmwareIAPObj::NAME = QString("FirmwareIAPObj");

/**
 * Constructor
 */
FirmwareIAPObj::FirmwareIAPObj(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList CommandElemNames;
    CommandElemNames.append("0");
    fields.append( new UAVObjectField(QString("Command"), QString("na"), UAVObjectField::UINT16, CommandElemNames, QStringList()) );
    QStringList PortElemNames;
    PortElemNames.append("0");
    fields.append( new UAVObjectField(QString("Port"), QString("na"), UAVObjectField::UINT32, PortElemNames, QStringList()) );
    QStringList VersionElemNames;
    VersionElemNames.append("0");
    VersionElemNames.append("1");
    VersionElemNames.append("2");
    fields.append( new UAVObjectField(QString("Version"), QString("na"), UAVObjectField::UINT8, VersionElemNames, QStringList()) );
    QStringList SVNElemNames;
    SVNElemNames.append("0");
    fields.append( new UAVObjectField(QString("SVN"), QString("na"), UAVObjectField::UINT16, SVNElemNames, QStringList()) );
    QStringList crcElemNames;
    crcElemNames.append("0");
    fields.append( new UAVObjectField(QString("crc"), QString("na"), UAVObjectField::UINT32, crcElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata FirmwareIAPObj::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 1;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 1;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
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
void FirmwareIAPObj::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
FirmwareIAPObj::DataFields FirmwareIAPObj::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void FirmwareIAPObj::setData(const DataFields& data)
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
UAVDataObject* FirmwareIAPObj::clone(quint32 instID)
{
    FirmwareIAPObj* obj = new FirmwareIAPObj();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
FirmwareIAPObj* FirmwareIAPObj::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<FirmwareIAPObj*>(objMngr->getObject(FirmwareIAPObj::OBJID, instID));
}
