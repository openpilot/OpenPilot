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
#include "uavobjecttemplate.h"

$(NAME)::$(NAME)(): UAVDataObject(OBJID, 0, SINGLEINST, NAME, NUMBYTES)
{
    // Create fields
    QList<UAVObjectField*> fields;

    $(FIELDS)
    // fields.append(new UAVObjectField($(FIELD_NAME), $(FIELD_UNITS), $(FIELD_TYPE), $(FIELD_NUMELEM));

    // Create metadata
    UAVObject::Metadata metadata;
    metadata.ackRequired = $(ACK);
    metadata.gcsTelemetryUpdateMode = $(GCSTELEM_UPDATEMODE);
    metadata.gcsTelemetryUpdatePeriod = $(GCSTELEM_UPDATEPERIOD);
    metadata.flightTelemetryUpdateMode = $(FLIGHTTELEM_UPDATEMODE);
    metadata.flightTelemetryUpdatePeriod = $(FLIGHTTELEM_UPDATEPERIOD);
    metadata.loggingUpdateMode = $(LOGGING_UPDATEMODE);
    metadata.loggingUpdatePeriod = $(LOGGING_UPDATEPERIOD);

    // Initialize object
    initialize(fields, metadata);
}

$(NAME)Data $(NAME)::getData()
{
    return data;
}

void $(NAME)::setData($(NAME)Data& data)
{
    this->data = data;
}
