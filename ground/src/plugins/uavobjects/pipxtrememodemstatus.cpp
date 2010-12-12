/**
 ******************************************************************************
 *
 * @file       pipxtrememodemstatus.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: pipxtrememodemstatus.xml. 
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
#include "pipxtrememodemstatus.h"
#include "uavobjectfield.h"

const QString PipXtremeModemStatus::NAME = QString("PipXtremeModemStatus");

/**
 * Constructor
 */
PipXtremeModemStatus::PipXtremeModemStatus(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList SerialNumberElemNames;
    SerialNumberElemNames.append("0");
    SerialNumberElemNames.append("1");
    SerialNumberElemNames.append("2");
    SerialNumberElemNames.append("3");
    SerialNumberElemNames.append("4");
    SerialNumberElemNames.append("5");
    SerialNumberElemNames.append("6");
    SerialNumberElemNames.append("7");
    SerialNumberElemNames.append("8");
    SerialNumberElemNames.append("9");
    SerialNumberElemNames.append("10");
    SerialNumberElemNames.append("11");
    SerialNumberElemNames.append("12");
    SerialNumberElemNames.append("13");
    SerialNumberElemNames.append("14");
    SerialNumberElemNames.append("15");
    SerialNumberElemNames.append("16");
    SerialNumberElemNames.append("17");
    SerialNumberElemNames.append("18");
    SerialNumberElemNames.append("19");
    SerialNumberElemNames.append("20");
    SerialNumberElemNames.append("21");
    SerialNumberElemNames.append("22");
    SerialNumberElemNames.append("23");
    fields.append( new UAVObjectField(QString("SerialNumber"), QString(""), UAVObjectField::UINT8, SerialNumberElemNames, QStringList()) );
    QStringList SerialNumberCRCElemNames;
    SerialNumberCRCElemNames.append("0");
    fields.append( new UAVObjectField(QString("SerialNumberCRC"), QString(""), UAVObjectField::UINT32, SerialNumberCRCElemNames, QStringList()) );
    QStringList UpTimeElemNames;
    UpTimeElemNames.append("0");
    fields.append( new UAVObjectField(QString("UpTime"), QString("ms"), UAVObjectField::UINT32, UpTimeElemNames, QStringList()) );
    QStringList FrequencyElemNames;
    FrequencyElemNames.append("0");
    fields.append( new UAVObjectField(QString("Frequency"), QString("Hz"), UAVObjectField::UINT32, FrequencyElemNames, QStringList()) );
    QStringList RFBandwidthElemNames;
    RFBandwidthElemNames.append("0");
    fields.append( new UAVObjectField(QString("RFBandwidth"), QString("Hz"), UAVObjectField::UINT32, RFBandwidthElemNames, QStringList()) );
    QStringList TxPowerElemNames;
    TxPowerElemNames.append("0");
    fields.append( new UAVObjectField(QString("TxPower"), QString("dBm"), UAVObjectField::INT8, TxPowerElemNames, QStringList()) );
    QStringList StateElemNames;
    StateElemNames.append("0");
    QStringList StateEnumOptions;
    StateEnumOptions.append("Disconnected");
    StateEnumOptions.append("Connecting");
    StateEnumOptions.append("Connected");
    StateEnumOptions.append("NotReady");
    fields.append( new UAVObjectField(QString("State"), QString(""), UAVObjectField::ENUM, StateElemNames, StateEnumOptions) );
    QStringList TxRetryElemNames;
    TxRetryElemNames.append("0");
    fields.append( new UAVObjectField(QString("TxRetry"), QString(""), UAVObjectField::UINT16, TxRetryElemNames, QStringList()) );
    QStringList TxDataRateElemNames;
    TxDataRateElemNames.append("0");
    fields.append( new UAVObjectField(QString("TxDataRate"), QString("bytes/sec"), UAVObjectField::UINT32, TxDataRateElemNames, QStringList()) );
    QStringList RxDataRateElemNames;
    RxDataRateElemNames.append("0");
    fields.append( new UAVObjectField(QString("RxDataRate"), QString("bytes/sec"), UAVObjectField::UINT32, RxDataRateElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata PipXtremeModemStatus::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 0;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 0;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.flightTelemetryUpdatePeriod = 5000;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_NEVER;
    metadata.loggingUpdatePeriod = 0;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void PipXtremeModemStatus::setDefaultFieldValues()
{
    data.SerialNumber[0] = 0;
    data.SerialNumber[1] = 0;
    data.SerialNumber[2] = 0;
    data.SerialNumber[3] = 0;
    data.SerialNumber[4] = 0;
    data.SerialNumber[5] = 0;
    data.SerialNumber[6] = 0;
    data.SerialNumber[7] = 0;
    data.SerialNumber[8] = 0;
    data.SerialNumber[9] = 0;
    data.SerialNumber[10] = 0;
    data.SerialNumber[11] = 0;
    data.SerialNumber[12] = 0;
    data.SerialNumber[13] = 0;
    data.SerialNumber[14] = 0;
    data.SerialNumber[15] = 0;
    data.SerialNumber[16] = 0;
    data.SerialNumber[17] = 0;
    data.SerialNumber[18] = 0;
    data.SerialNumber[19] = 0;
    data.SerialNumber[20] = 0;
    data.SerialNumber[21] = 0;
    data.SerialNumber[22] = 0;
    data.SerialNumber[23] = 0;
    data.SerialNumberCRC = 0;

}

/**
 * Get the object data fields
 */
PipXtremeModemStatus::DataFields PipXtremeModemStatus::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void PipXtremeModemStatus::setData(const DataFields& data)
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
UAVDataObject* PipXtremeModemStatus::clone(quint32 instID)
{
    PipXtremeModemStatus* obj = new PipXtremeModemStatus();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
PipXtremeModemStatus* PipXtremeModemStatus::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<PipXtremeModemStatus*>(objMngr->getObject(PipXtremeModemStatus::OBJID, instID));
}
