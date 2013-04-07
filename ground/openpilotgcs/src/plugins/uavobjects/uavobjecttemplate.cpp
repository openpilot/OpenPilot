/**
 ******************************************************************************
 *
 * @file       $(NAMELC).cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *
 * @note       Object definition file: $(XMLFILE).
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
#include "$(NAMELC).h"
#include "uavobjectfield.h"

const QString $(NAME)::NAME = QString("$(NAME)");
const QString $(NAME)::DESCRIPTION = QString("$(DESCRIPTION)");
const QString $(NAME)::CATEGORY = QString("$(CATEGORY)");

/**
 * Constructor
 */
$(NAME)::$(NAME)(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
$(FIELDSINIT)
    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
    // Set the object description
    setDescription(DESCRIPTION);

    // Set the Category of this object type
    setCategory(CATEGORY);

    connect(this, SIGNAL(objectUpdated(UAVObject*)),
            SLOT(emitNotifications()));
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata $(NAME)::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flags =
      $(FLIGHTACCESS) << UAVOBJ_ACCESS_SHIFT |
      $(GCSACCESS) << UAVOBJ_GCS_ACCESS_SHIFT |
      $(FLIGHTTELEM_ACKED) << UAVOBJ_TELEMETRY_ACKED_SHIFT |
      $(GCSTELEM_ACKED) << UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT |
      $(FLIGHTTELEM_UPDATEMODE) << UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT |
      $(GCSTELEM_UPDATEMODE) << UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT;
    metadata.flightTelemetryUpdatePeriod = $(FLIGHTTELEM_UPDATEPERIOD);
    metadata.gcsTelemetryUpdatePeriod = $(GCSTELEM_UPDATEPERIOD);
    metadata.loggingUpdatePeriod = $(LOGGING_UPDATEPERIOD);
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void $(NAME)::setDefaultFieldValues()
{
$(INITFIELDS)
}

/**
 * Get the object data fields
 */
$(NAME)::DataFields $(NAME)::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void $(NAME)::setData(const DataFields& data)
{
    QMutexLocker locker(mutex);
    // Get metadata
    Metadata mdata = getMetadata();
    // Update object if the access mode permits
    if ( UAVObject::GetGcsAccess(mdata) == ACCESS_READWRITE )
    {
        this->data = data;
        emit objectUpdatedAuto(this); // trigger object updated event
        emit objectUpdated(this);
    }
}

void $(NAME)::emitNotifications()
{
    $(NOTIFY_PROPERTIES_CHANGED)
}

/**
 * Create a clone of this object, a new instance ID must be specified.
 * Do not use this function directly to create new instances, the
 * UAVObjectManager should be used instead.
 */
UAVDataObject* $(NAME)::clone(quint32 instID)
{
    $(NAME)* obj = new $(NAME)();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Create a clone of this object only to be used to retrieve defaults
 */
UAVDataObject* $(NAME)::dirtyClone()
{
    $(NAME)* obj = new $(NAME)();
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
$(NAME)* $(NAME)::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<$(NAME)*>(objMngr->getObject($(NAME)::OBJID, instID));
}

$(PROPERTIES_IMPL)
