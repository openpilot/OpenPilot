/**
 ******************************************************************************
 *
 * @file       uavmetaobject.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   
 * @{
 * 
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

UAVMetaObject::UAVMetaObject(quint32 objID, QString& name, Metadata& mdata, UAVObject* parent):
        UAVObject(objID, 0, true, name, sizeof(Metadata))
{
    this->parentMetadata = mdata;
    this->parent = parent;
    ownMetadata.ackRequired = 1;
    ownMetadata.flightTelemetryUpdateMode = UPDATEMODE_ONCHANGE;
    ownMetadata.flightTelemetryUpdatePeriod = 0;
    ownMetadata.gcsTelemetryUpdateMode = UPDATEMODE_ONCHANGE;
    ownMetadata.gcsTelemetryUpdatePeriod = 0;
    ownMetadata.loggingUpdateMode = UPDATEMODE_ONCHANGE;
    ownMetadata.loggingUpdatePeriod = 0;
}

UAVObject* UAVMetaObject::getParentObject()
{
    return parent;
}

qint32 UAVMetaObject::pack(quint8* dataOut)
{
    QMutexLocker locker(mutex);
    memcpy(dataOut, &parentMetadata, sizeof(Metadata));
    return sizeof(Metadata);
}

qint32 UAVMetaObject::unpack(const quint8* dataIn)
{
    QMutexLocker locker(mutex);
    memcpy(&parentMetadata, dataIn, sizeof(Metadata));
    emit objectUpdated(this, true); // trigger object updated event
    return sizeof(Metadata);
}

void UAVMetaObject::setMetadata(const Metadata& mdata)
{
    return; // can not update metaobject's metadata
}

UAVObject::Metadata UAVMetaObject::getMetadata()
{
    return ownMetadata;
}

void UAVMetaObject::setData(const Metadata& mdata)
{
    QMutexLocker locker(mutex);
    parentMetadata = mdata;
    emit objectUpdated(this, false); // trigger object updated event
}

UAVObject::Metadata UAVMetaObject::getData()
{
    QMutexLocker locker(mutex);
    return parentMetadata;
}
