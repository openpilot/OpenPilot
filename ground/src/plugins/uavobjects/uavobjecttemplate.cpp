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
