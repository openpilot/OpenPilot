/**
 ******************************************************************************
 *
 * @file       uavobjecttemplate.cpp
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
#include "$(NAMELC).h"

const QString $(NAME)::NAME = QString("$(NAME)");

$(NAME)::$(NAME)(): UAVDataObject(OBJID, SINGLEINST, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;

    $(FIELDS)
    // fields.append(new UAVObjectField(QString("$(FIELD_NAME)"), QString("$(FIELD_UNITS)", UAVObjectField::$(FIELD_TYPE), $(FIELD_NUMELEM));

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
}

UAVObject::Metadata $(NAME)::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.ackRequired = $(ACK);
    metadata.gcsTelemetryUpdateMode = UAVObject::$(GCSTELEM_UPDATEMODE);
    metadata.gcsTelemetryUpdatePeriod = $(GCSTELEM_UPDATEPERIOD);
    metadata.flightTelemetryUpdateMode = UAVObject::$(FLIGHTTELEM_UPDATEMODE);
    metadata.flightTelemetryUpdatePeriod = $(FLIGHTTELEM_UPDATEPERIOD);
    metadata.loggingUpdateMode = UAVObject::$(LOGGING_UPDATEMODE);
    metadata.loggingUpdatePeriod = $(LOGGING_UPDATEPERIOD);
	return metadata;
}

$(NAME)::DataFields $(NAME)::getData()
{
	QMutexLocker locker(mutex);
    return data;
}

void $(NAME)::setData(DataFields& data)
{
	QMutexLocker locker(mutex);
    this->data = data;
    emit objectUpdatedAuto(this); // trigger object updated event
    emit objectUpdated(this);
}

UAVDataObject* $(NAME)::clone(quint32 instID)
{
    $(NAME)* obj = new $(NAME)();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}