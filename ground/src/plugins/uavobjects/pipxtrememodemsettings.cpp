/**
 ******************************************************************************
 *
 * @file       pipxtrememodemsettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: pipxtrememodemsettings.xml. 
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
#include "pipxtrememodemsettings.h"
#include "uavobjectfield.h"

const QString PipXtremeModemSettings::NAME = QString("PipXtremeModemSettings");

/**
 * Constructor
 */
PipXtremeModemSettings::PipXtremeModemSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList FrequencyElemNames;
    FrequencyElemNames.append("0");
    fields.append( new UAVObjectField(QString("Frequency"), QString("Hz"), UAVObjectField::UINT32, FrequencyElemNames, QStringList()) );
    QStringList RFBandwidthElemNames;
    RFBandwidthElemNames.append("0");
    QStringList RFBandwidthEnumOptions;
    RFBandwidthEnumOptions.append("500");
    RFBandwidthEnumOptions.append("1000");
    RFBandwidthEnumOptions.append("2000");
    RFBandwidthEnumOptions.append("4000");
    RFBandwidthEnumOptions.append("8000");
    RFBandwidthEnumOptions.append("9600");
    RFBandwidthEnumOptions.append("16000");
    RFBandwidthEnumOptions.append("19200");
    RFBandwidthEnumOptions.append("24000");
    RFBandwidthEnumOptions.append("32000");
    RFBandwidthEnumOptions.append("64000");
    RFBandwidthEnumOptions.append("128000");
    RFBandwidthEnumOptions.append("192000");
    fields.append( new UAVObjectField(QString("RFBandwidth"), QString("Hz"), UAVObjectField::ENUM, RFBandwidthElemNames, RFBandwidthEnumOptions) );
    QStringList MaxTxPowerElemNames;
    MaxTxPowerElemNames.append("0");
    QStringList MaxTxPowerEnumOptions;
    MaxTxPowerEnumOptions.append("1");
    MaxTxPowerEnumOptions.append("2");
    MaxTxPowerEnumOptions.append("5");
    MaxTxPowerEnumOptions.append("8");
    MaxTxPowerEnumOptions.append("11");
    MaxTxPowerEnumOptions.append("14");
    MaxTxPowerEnumOptions.append("17");
    MaxTxPowerEnumOptions.append("20");
    fields.append( new UAVObjectField(QString("MaxTxPower"), QString("dBm"), UAVObjectField::ENUM, MaxTxPowerElemNames, MaxTxPowerEnumOptions) );
    QStringList AESEncryptionElemNames;
    AESEncryptionElemNames.append("0");
    QStringList AESEncryptionEnumOptions;
    AESEncryptionEnumOptions.append("False");
    AESEncryptionEnumOptions.append("True");
    fields.append( new UAVObjectField(QString("AESEncryption"), QString(""), UAVObjectField::ENUM, AESEncryptionElemNames, AESEncryptionEnumOptions) );
    QStringList AESEncryptionKeyElemNames;
    AESEncryptionKeyElemNames.append("0");
    AESEncryptionKeyElemNames.append("1");
    AESEncryptionKeyElemNames.append("2");
    AESEncryptionKeyElemNames.append("3");
    AESEncryptionKeyElemNames.append("4");
    AESEncryptionKeyElemNames.append("5");
    AESEncryptionKeyElemNames.append("6");
    AESEncryptionKeyElemNames.append("7");
    AESEncryptionKeyElemNames.append("8");
    AESEncryptionKeyElemNames.append("9");
    AESEncryptionKeyElemNames.append("10");
    AESEncryptionKeyElemNames.append("11");
    AESEncryptionKeyElemNames.append("12");
    AESEncryptionKeyElemNames.append("13");
    AESEncryptionKeyElemNames.append("14");
    AESEncryptionKeyElemNames.append("15");
    fields.append( new UAVObjectField(QString("AESEncryptionKey"), QString(""), UAVObjectField::UINT8, AESEncryptionKeyElemNames, QStringList()) );
    QStringList PairedSerialNumberCRCElemNames;
    PairedSerialNumberCRCElemNames.append("0");
    fields.append( new UAVObjectField(QString("PairedSerialNumberCRC"), QString(""), UAVObjectField::UINT32, PairedSerialNumberCRCElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata PipXtremeModemSettings::getDefaultMetadata()
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
void PipXtremeModemSettings::setDefaultFieldValues()
{
    data.Frequency = 433920000;
    data.RFBandwidth = 11;
    data.MaxTxPower = 4;
    data.AESEncryption = 0;
    data.AESEncryptionKey[0] = 0;
    data.AESEncryptionKey[1] = 0;
    data.AESEncryptionKey[2] = 0;
    data.AESEncryptionKey[3] = 0;
    data.AESEncryptionKey[4] = 0;
    data.AESEncryptionKey[5] = 0;
    data.AESEncryptionKey[6] = 0;
    data.AESEncryptionKey[7] = 0;
    data.AESEncryptionKey[8] = 0;
    data.AESEncryptionKey[9] = 0;
    data.AESEncryptionKey[10] = 0;
    data.AESEncryptionKey[11] = 0;
    data.AESEncryptionKey[12] = 0;
    data.AESEncryptionKey[13] = 0;
    data.AESEncryptionKey[14] = 0;
    data.AESEncryptionKey[15] = 0;
    data.PairedSerialNumberCRC = 0;

}

/**
 * Get the object data fields
 */
PipXtremeModemSettings::DataFields PipXtremeModemSettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void PipXtremeModemSettings::setData(const DataFields& data)
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
UAVDataObject* PipXtremeModemSettings::clone(quint32 instID)
{
    PipXtremeModemSettings* obj = new PipXtremeModemSettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
PipXtremeModemSettings* PipXtremeModemSettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<PipXtremeModemSettings*>(objMngr->getObject(PipXtremeModemSettings::OBJID, instID));
}
