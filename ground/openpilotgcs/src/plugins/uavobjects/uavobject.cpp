/**
 ******************************************************************************
 *
 * @file       uavobject.cpp
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
#include "uavobject.h"

#include <utils/crc.h>

#include <QtEndian>
#include <QDebug>
#include <QtWidgets>

using namespace Utils;

// Constants
#define UAVOBJ_ACCESS_SHIFT                    0
#define UAVOBJ_GCS_ACCESS_SHIFT                1
#define UAVOBJ_TELEMETRY_ACKED_SHIFT           2
#define UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT       3
#define UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT     4
#define UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT 6
#define UAVOBJ_LOGGING_UPDATE_MODE_SHIFT       8
#define UAVOBJ_UPDATE_MODE_MASK                0x3

// Macros
#define SET_BITS(var, shift, value, mask) var = (var & ~(mask << shift)) | (value << shift);

/**
 * Constructor
 * @param objID The object ID
 * @param isSingleInst True if this object can only have a single instance
 * @param name Object name
 */
UAVObject::UAVObject(quint32 objID, bool isSingleInst, const QString & name)
{
    this->objID        = objID;
    this->instID       = 0;
    this->isSingleInst = isSingleInst;
    this->name         = name;
    this->data         = 0;
    this->numBytes     = 0;
    this->mutex        = new QMutex(QMutex::Recursive);
    m_isKnown = false;
}

/**
 * Initialize object with its instance ID
 */
void UAVObject::initialize(quint32 instID)
{
    QMutexLocker locker(mutex);

    this->instID = instID;
}

/**
 * Initialize objects' data fields
 * @param fields List of fields held by the object
 * @param data Pointer to that actual object data, this is needed by the fields to access the data
 * @param numBytes Number of bytes in the object (total, including all fields)
 */
void UAVObject::initializeFields(QList<UAVObjectField *> & fields, quint8 *data, quint32 numBytes)
{
    QMutexLocker locker(mutex);

    this->numBytes = numBytes;
    this->data     = data;
    this->fields   = fields;
    // Initialize fields
    quint32 offset = 0;
    for (int n = 0; n < fields.length(); ++n) {
        fields[n]->initialize(data, offset, this);
        offset += fields[n]->getNumBytes();
        connect(fields[n], SIGNAL(fieldUpdated(UAVObjectField *)), this, SLOT(fieldUpdated(UAVObjectField *)));
    }
}

/**
 * Called from the fields each time they are updated
 */
void UAVObject::fieldUpdated(UAVObjectField *field)
{
    Q_UNUSED(field);
// emit objectUpdatedAuto(this); // trigger object updated event
// emit objectUpdated(this);
}

/**
 * Get the object ID
 */
quint32 UAVObject::getObjID()
{
    return objID;
}

/**
 * Get the instance ID
 */
quint32 UAVObject::getInstID()
{
    return instID;
}

/**
 * Returns true if this is a single instance object
 */
bool UAVObject::isSingleInstance()
{
    return isSingleInst;
}

/**
 * Get the name of the object
 */
QString UAVObject::getName()
{
    return name;
}

/**
 * Get the description of the object
 */
QString UAVObject::getDescription()
{
    return description;
}

/**
 * Set the description of the object
 */
void UAVObject::setDescription(const QString & description)
{
    this->description = description;
}

/**
 * Get the category of the object
 */
QString UAVObject::getCategory()
{
    return category;
}

/**
 * Set the category of the object
 */
void UAVObject::setCategory(const QString & category)
{
    this->category = category;
}

/**
 * Get the total number of bytes of the object's data
 */
quint32 UAVObject::getNumBytes()
{
    return numBytes;
}

/**
 * Request that this object is updated with the latest values from the autopilot
 */
void UAVObject::requestUpdate()
{
    emit updateRequested(this);
}

/**
 * Request that all instances of this object are updated with the latest values from the autopilot
 * Must be called on instance zero
 */
void UAVObject::requestUpdateAll()
{
    if (instID == 0) {
        emit updateRequested(this, true);
    }
}

/**
 * Signal that the object has been updated
 */
void UAVObject::updated()
{
    emit objectUpdatedManual(this);
    emit objectUpdated(this);
}

/**
 * Signal that all instance of the object have been updated
 * Must be called on instance zero
 */
void UAVObject::updatedAll()
{
    if (instID == 0) {
        emit objectUpdatedManual(this, true);
        // TODO call objectUpdated() for all instances?
        // emit objectUpdated(this);
    }
}

/**
 * Lock mutex of this object
 */
void UAVObject::lock()
{
    mutex->lock();
}

/**
 * Lock mutex of this object
 */
void UAVObject::lock(int timeoutMs)
{
    mutex->tryLock(timeoutMs);
}

/**
 * Unlock mutex of this object
 */
void UAVObject::unlock()
{
    mutex->unlock();
}

/**
 * Get object's mutex
 */
QMutex *UAVObject::getMutex()
{
    return mutex;
}

/**
 * Get the number of fields held by this object
 */
qint32 UAVObject::getNumFields()
{
    QMutexLocker locker(mutex);

    return fields.count();
}

/**
 * Get the object's fields
 */
QList<UAVObjectField *> UAVObject::getFields()
{
    QMutexLocker locker(mutex);

    return fields;
}

/**
 * Get a specific field
 * @returns The field or NULL if not found
 */
UAVObjectField *UAVObject::getField(const QString & name)
{
    QMutexLocker locker(mutex);

    // Look for field
    for (int n = 0; n < fields.length(); ++n) {
        if (name.compare(fields[n]->getName()) == 0) {
            return fields[n];
        }
    }
    // If this point is reached then the field was not found
    qWarning() << "UAVObject::getField Non existant field" << name << "requested."
               << "This indicates a bug. Make sure you also have null checking for non-debug code.";
    return NULL;
}

/**
 * Pack the object data into a byte array
 * @returns The number of bytes copied
 */
qint32 UAVObject::pack(quint8 *dataOut)
{
    QMutexLocker locker(mutex);
    qint32 offset = 0;

    for (int n = 0; n < fields.length(); ++n) {
        fields[n]->pack(&dataOut[offset]);
        offset += fields[n]->getNumBytes();
    }
    return numBytes;
}

/**
 * Unpack the object data from a byte array
 * @returns The number of bytes copied
 */
qint32 UAVObject::unpack(const quint8 *dataIn)
{
    QMutexLocker locker(mutex);
    qint32 offset = 0;

    for (int n = 0; n < fields.length(); ++n) {
        fields[n]->unpack(&dataIn[offset]);
        offset += fields[n]->getNumBytes();
    }
    emit objectUnpacked(this); // trigger object updated event
    emit objectUpdated(this);

    return numBytes;
}

/**
 * Update a CRC with the object data
 * @returns The updated CRC
 */
quint8 UAVObject::updateCRC(quint8 crc)
{
    QMutexLocker locker(mutex);

    // crc = Crc::updateCRC(crc, (quint8 *) &objID, sizeof(objID));
    // crc = Crc::updateCRC(crc, (quint8 *) &instID, sizeof(instID));
    crc = Crc::updateCRC(crc, data, numBytes);

    return crc;
}

/**
 * Save the object data to the file.
 * The file will be created in the current directory
 * and its name will be the same as the object with
 * the .uavobj extension.
 * @returns True on success, false on failure
 */
bool UAVObject::save()
{
    QMutexLocker locker(mutex);

    // Open file
    QFile file(name + ".uavobj");

    if (!file.open(QFile::WriteOnly)) {
        return false;
    }

    // Write object
    if (!save(file)) {
        return false;
    }

    // Close file
    file.close();
    return true;
}

/**
 * Save the object data to the file.
 * The file is expected to be already open for writting.
 * The data will be appended and the file will not be closed.
 * @returns True on success, false on failure
 */
bool UAVObject::save(QFile & file)
{
    QMutexLocker locker(mutex);
    quint8 buffer[numBytes];
    quint8 tmpId[4];

    // Write the object ID
    qToLittleEndian<quint32>(objID, tmpId);
    if (file.write((const char *)tmpId, 4) == -1) {
        return false;
    }

    // Write the instance ID
    if (!isSingleInst) {
        qToLittleEndian<quint16>(instID, tmpId);
        if (file.write((const char *)tmpId, 2) == -1) {
            return false;
        }
    }

    // Write the data
    pack(buffer);
    if (file.write((const char *)buffer, numBytes) == -1) {
        return false;
    }

    // Done
    return true;
}

/**
 * Load the object data from a file.
 * The file will be openned in the current directory
 * and its name will be the same as the object with
 * the .uavobj extension.
 * @returns True on success, false on failure
 */
bool UAVObject::load()
{
    QMutexLocker locker(mutex);

    // Open file
    QFile file(name + ".uavobj");

    if (!file.open(QFile::ReadOnly)) {
        return false;
    }

    // Load object
    if (!load(file)) {
        return false;
    }

    // Close file
    file.close();
    return true;
}

/**
 * Load the object data from file.
 * The file is expected to be already open for reading.
 * The data will be read and the file will not be closed.
 * @returns True on success, false on failure
 */
bool UAVObject::load(QFile & file)
{
    QMutexLocker locker(mutex);
    quint8 buffer[numBytes];
    quint8 tmpId[4];

    // Read the object ID
    if (file.read((char *)tmpId, 4) != 4) {
        return false;
    }

    // Check that the IDs match
    if (qFromLittleEndian<quint32>(tmpId) != objID) {
        return false;
    }

    // Read the instance ID
    if (file.read((char *)tmpId, 2) != 2) {
        return false;
    }

    // Check that the IDs match
    if (qFromLittleEndian<quint16>(tmpId) != instID) {
        return false;
    }

    // Read and unpack the data
    if (file.read((char *)buffer, numBytes) != numBytes) {
        return false;
    }
    unpack(buffer);

    // Done
    return true;
}

/**
 * Return a string with the object information
 */
QString UAVObject::toString()
{
    QString sout;

    sout.append(toStringBrief());
    sout.append('\n');
    sout.append(toStringData());
    return sout;
}

/**
 * Return a string with the object information (only the header)
 */
QString UAVObject::toStringBrief()
{
    QString sout;

    // object Id is converted to uppercase hexadecimal
    sout.append(QString("%1 (ID: %2-%3, %4 bytes, %5)")
                .arg(getName())
                .arg(getObjID(), 1, 16).toUpper()
                .arg(getInstID())
                .arg(getNumBytes())
                .arg(isSingleInstance() ? "single" : "multiple"));
    return sout;
}

/**
 * Return a string with the object information (only the data)
 */
QString UAVObject::toStringData()
{
    QString sout;

    sout.append("Data:\n");
    for (int n = 0; n < fields.length(); ++n) {
        sout.append(QString("\t%1").arg(fields[n]->toString()));
    }
    return sout;
}

void UAVObject::toXML(QXmlStreamWriter *xmlWriter)
{
    xmlWriter->writeStartElement("object");
    xmlWriter->writeAttribute("name", getName());
    xmlWriter->writeAttribute("id", QString("%1").arg(getObjID(), 1, 16).toUpper());
    xmlWriter->writeAttribute("instance", QString("%1").arg(getInstID()));
    xmlWriter->writeStartElement("fields");
    foreach(UAVObjectField * field, fields) {
        field->toXML(xmlWriter);
    }
    xmlWriter->writeEndElement(); // fields
    xmlWriter->writeEndElement(); // object
}

void UAVObject::fromXML(QXmlStreamReader *xmlReader)
{
    // Check that we are the correct object by name, and that we are the same instance id
    // but dont check id since we want to be able to be forward compatible.
    if (xmlReader->name() == "object" &&
        xmlReader->attributes().value("name") == getName() &&
        xmlReader->attributes().value("instance") == QString("%1").arg(getInstID())) {
        QXmlStreamReader::TokenType token = xmlReader->readNext();
        if (token == QXmlStreamReader::StartElement && xmlReader->name() == "fields") {
            while (xmlReader->readNextStartElement()) {
                if (xmlReader->name() == "field") {
                    QStringRef fieldName = xmlReader->attributes().value("name");
                    if (fieldName != "") {
                        getField(*fieldName.string())->fromXML(xmlReader);
                    }
                }
            }
        }
    }
}

void UAVObject::toJson(QJsonObject &jsonObject)
{
    jsonObject["name"]     = getName();
    jsonObject["setting"]  = isSettingsObject();
    jsonObject["id"] = QString("%1").arg(getObjID(), 1, 16).toUpper();
    jsonObject["instance"] = (int)getInstID();
    QJsonArray jSonFields;
    foreach(UAVObjectField * field, fields) {
        QJsonObject jSonField;

        field->toJson(jSonField);
        jSonFields.append(jSonField);
    }
    jsonObject["fields"] = jSonFields;
}

void UAVObject::fromJson(const QJsonObject &jsonObject)
{
    if (jsonObject["name"].toString() == getName() &&
        jsonObject["instance"].toInt() == (int)getInstID()) {
        QJsonArray jsonFields = jsonObject["fields"].toArray();
        for (int i = 0; i < jsonFields.size(); i++) {
            QJsonObject jsonField = jsonFields.at(i).toObject();
            UAVObjectField *field = getField(jsonField["name"].toString());
            if (field != NULL) {
                field->fromJson(jsonField);
            }
        }
        updated();
    }
}

/**
 * Emit the transactionCompleted event (used by the UAVTalk plugin)
 */
void UAVObject::emitTransactionCompleted(bool success)
{
    emit transactionCompleted(this, success);
}

/**
 * Emit the newInstance event
 */
void UAVObject::emitNewInstance(UAVObject *obj)
{
    emit newInstance(obj);
}

bool UAVObject::isKnown() const
{
    QMutexLocker locker(mutex);

    return m_isKnown;
}

void UAVObject::setIsKnown(bool isKnown)
{
    lock();
    bool changed = m_isKnown != isKnown;
    m_isKnown = isKnown;
    unlock();

    if (changed) {
        emit isKnownChanged(this, isKnown);
    }
}

bool UAVObject::isSettingsObject()
{
    return false;
}

bool UAVObject::isDataObject()
{
    return false;
}

bool UAVObject::isMetaDataObject()
{
    return false;
}

/**
 * Initialize a UAVObjMetadata object.
 * \param[in] metadata The metadata object
 */
void UAVObject::MetadataInitialize(UAVObject::Metadata & metadata)
{
    metadata.flags =
        ACCESS_READWRITE << UAVOBJ_ACCESS_SHIFT |
        ACCESS_READWRITE << UAVOBJ_GCS_ACCESS_SHIFT |
        1 << UAVOBJ_TELEMETRY_ACKED_SHIFT |
        1 << UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT |
        UPDATEMODE_ONCHANGE << UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT |
        UPDATEMODE_ONCHANGE << UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT |
        UPDATEMODE_ONCHANGE << UAVOBJ_LOGGING_UPDATE_MODE_SHIFT;
    metadata.flightTelemetryUpdatePeriod = 0;
    metadata.gcsTelemetryUpdatePeriod    = 0;
    metadata.loggingUpdatePeriod = 0;
}

/**
 * Get the UAVObject metadata access member
 * \param[in] metadata The metadata object
 * \return the access type
 */
UAVObject::AccessMode UAVObject::GetFlightAccess(const UAVObject::Metadata & metadata)
{
    return UAVObject::AccessMode((metadata.flags >> UAVOBJ_ACCESS_SHIFT) & 1);
}

/**
 * Set the UAVObject metadata access member
 * \param[in] metadata The metadata object
 * \param[in] mode The access mode
 */
void UAVObject::SetFlightAccess(UAVObject::Metadata & metadata, UAVObject::AccessMode mode)
{
    SET_BITS(metadata.flags, UAVOBJ_ACCESS_SHIFT, mode, 1);
}

/**
 * Get the UAVObject metadata GCS access member
 * \param[in] metadata The metadata object
 * \return the GCS access type
 */
UAVObject::AccessMode UAVObject::GetGcsAccess(const UAVObject::Metadata & metadata)
{
    return UAVObject::AccessMode((metadata.flags >> UAVOBJ_GCS_ACCESS_SHIFT) & 1);
}

/**
 * Set the UAVObject metadata GCS access member
 * \param[in] metadata The metadata object
 * \param[in] mode The access mode
 */
void UAVObject::SetGcsAccess(UAVObject::Metadata & metadata, UAVObject::AccessMode mode)
{
    SET_BITS(metadata.flags, UAVOBJ_GCS_ACCESS_SHIFT, mode, 1);
}

/**
 * Get the UAVObject metadata telemetry acked member
 * \param[in] metadata The metadata object
 * \return the telemetry acked boolean
 */
quint8 UAVObject::GetFlightTelemetryAcked(const UAVObject::Metadata & metadata)
{
    return (metadata.flags >> UAVOBJ_TELEMETRY_ACKED_SHIFT) & 1;
}

/**
 * Set the UAVObject metadata telemetry acked member
 * \param[in] metadata The metadata object
 * \param[in] val The telemetry acked boolean
 */
void UAVObject::SetFlightTelemetryAcked(UAVObject::Metadata & metadata, quint8 val)
{
    SET_BITS(metadata.flags, UAVOBJ_TELEMETRY_ACKED_SHIFT, val, 1);
}

/**
 * Get the UAVObject metadata GCS telemetry acked member
 * \param[in] metadata The metadata object
 * \return the telemetry acked boolean
 */
quint8 UAVObject::GetGcsTelemetryAcked(const UAVObject::Metadata & metadata)
{
    return (metadata.flags >> UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT) & 1;
}

/**
 * Set the UAVObject metadata GCS telemetry acked member
 * \param[in] metadata The metadata object
 * \param[in] val The GCS telemetry acked boolean
 */
void UAVObject::SetGcsTelemetryAcked(UAVObject::Metadata & metadata, quint8 val)
{
    SET_BITS(metadata.flags, UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT, val, 1);
}

/**
 * Get the UAVObject metadata telemetry update mode
 * \param[in] metadata The metadata object
 * \return the telemetry update mode
 */
UAVObject::UpdateMode UAVObject::GetFlightTelemetryUpdateMode(const UAVObject::Metadata & metadata)
{
    return UAVObject::UpdateMode((metadata.flags >> UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT) & UAVOBJ_UPDATE_MODE_MASK);
}

/**
 * Set the UAVObject metadata telemetry update mode member
 * \param[in] metadata The metadata object
 * \param[in] val The telemetry update mode
 */
void UAVObject::SetFlightTelemetryUpdateMode(UAVObject::Metadata & metadata, UAVObject::UpdateMode val)
{
    SET_BITS(metadata.flags, UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT, val, UAVOBJ_UPDATE_MODE_MASK);
}

/**
 * Get the UAVObject metadata GCS telemetry update mode
 * \param[in] metadata The metadata object
 * \return the GCS telemetry update mode
 */
UAVObject::UpdateMode UAVObject::GetGcsTelemetryUpdateMode(const UAVObject::Metadata & metadata)
{
    return UAVObject::UpdateMode((metadata.flags >> UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT) & UAVOBJ_UPDATE_MODE_MASK);
}

/**
 * Set the UAVObject metadata GCS telemetry update mode member
 * \param[in] metadata The metadata object
 * \param[in] val The GCS telemetry update mode
 */
void UAVObject::SetGcsTelemetryUpdateMode(UAVObject::Metadata & metadata, UAVObject::UpdateMode val)
{
    SET_BITS(metadata.flags, UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT, val, UAVOBJ_UPDATE_MODE_MASK);
}

/**
 * Get the UAVObject metadata logging update mode
 * \param[in] metadata The metadata object
 * \return the logging update mode
 */
UAVObject::UpdateMode UAVObject::GetLoggingUpdateMode(const UAVObject::Metadata & metadata)
{
    return UAVObject::UpdateMode((metadata.flags >> UAVOBJ_LOGGING_UPDATE_MODE_SHIFT) & UAVOBJ_UPDATE_MODE_MASK);
}

/**
 * Set the UAVObject metadata logging update mode member
 * \param[in] metadata The metadata object
 * \param[in] val The logging update mode
 */
void UAVObject::SetLoggingUpdateMode(UAVObject::Metadata & metadata, UAVObject::UpdateMode val)
{
    SET_BITS(metadata.flags, UAVOBJ_LOGGING_UPDATE_MODE_SHIFT, val, UAVOBJ_UPDATE_MODE_MASK);
}
