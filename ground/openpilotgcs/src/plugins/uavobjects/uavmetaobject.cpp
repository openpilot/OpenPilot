/**
 ******************************************************************************
 *
 * @file       uavmetaobject.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
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
#include "uavmetaobject.h"
#include "uavobjectfield.h"

/**
 * Constructor
 */
UAVMetaObject::UAVMetaObject(quint32 objID, const QString& name, UAVObject *parent):
        UAVObject(objID, true, name)
{
    this->parent = parent;
    // Setup default metadata of metaobject (can not be changed)
    UAVObject::MetadataInitialize(ownMetadata);
    // Setup fields
    QStringList modesBitField;
    modesBitField << tr("FlightReadOnly") << tr("GCSReadOnly") << tr("FlightTelemetryAcked") << tr("GCSTelemetryAcked") << tr("FlightUpdatePeriodic") << tr("FlightUpdateOnChange") << tr("GCSUpdatePeriodic") << tr("GCSUpdateOnChange");
    QList<UAVObjectField*> fields;    
    fields.append( new UAVObjectField(tr("Modes"), tr("boolean"), UAVObjectField::BITFIELD, modesBitField, QStringList()) );
    fields.append( new UAVObjectField(tr("Flight Telemetry Update Period"), tr("ms"), UAVObjectField::UINT16, 1, QStringList()) );
    fields.append( new UAVObjectField(tr("GCS Telemetry Update Period"), tr("ms"), UAVObjectField::UINT16, 1, QStringList()) );
    fields.append( new UAVObjectField(tr("Logging Update Period"), tr("ms"), UAVObjectField::UINT16, 1, QStringList()) );
    // Initialize parent
    UAVObject::initialize(0);
    UAVObject::initializeFields(fields, (quint8*)&parentMetadata, sizeof(Metadata));
    // Setup metadata of parent
    parentMetadata = parent->getDefaultMetadata();
}

/**
 * Get the parent object
 */
UAVObject* UAVMetaObject::getParentObject()
{
    return parent;
}

/**
 * Set the metadata of the metaobject, this function will
 * do nothing since metaobjects have read-only metadata.
 */
void UAVMetaObject::setMetadata(const Metadata& mdata)
{
    Q_UNUSED(mdata);
    return; // can not update metaobject's metadata
}

/**
 * Get the metadata of the metaobject
 */
UAVObject::Metadata UAVMetaObject::getMetadata()
{
    return ownMetadata;
}

/**
 * Get the default metadata
 */
UAVObject::Metadata UAVMetaObject::getDefaultMetadata()
{
    return ownMetadata;
}

/**
 * Set the metadata held by the metaobject
 */
void UAVMetaObject::setData(const Metadata& mdata)
{
    QMutexLocker locker(mutex);
    parentMetadata = mdata;
    emit objectUpdatedAuto(this); // trigger object updated event
    emit objectUpdated(this);
}

/**
 * Get the metadata held by the metaobject
 */
UAVObject::Metadata UAVMetaObject::getData()
{
    QMutexLocker locker(mutex);
    return parentMetadata;
}


