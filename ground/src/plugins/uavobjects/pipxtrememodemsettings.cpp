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
    QStringList Frequency_CalibrationElemNames;
    Frequency_CalibrationElemNames.append("0");
    fields.append( new UAVObjectField(QString("Frequency_Calibration"), QString(""), UAVObjectField::UINT8, Frequency_CalibrationElemNames, QStringList()) );
    QStringList Frequency_MinElemNames;
    Frequency_MinElemNames.append("0");
    fields.append( new UAVObjectField(QString("Frequency_Min"), QString("Hz"), UAVObjectField::UINT32, Frequency_MinElemNames, QStringList()) );
    QStringList Frequency_MaxElemNames;
    Frequency_MaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("Frequency_Max"), QString("Hz"), UAVObjectField::UINT32, Frequency_MaxElemNames, QStringList()) );
    QStringList FrequencyElemNames;
    FrequencyElemNames.append("0");
    fields.append( new UAVObjectField(QString("Frequency"), QString("Hz"), UAVObjectField::UINT32, FrequencyElemNames, QStringList()) );
    QStringList RF_BandwidthElemNames;
    RF_BandwidthElemNames.append("0");
    QStringList RF_BandwidthEnumOptions;
    RF_BandwidthEnumOptions.append("500");
    RF_BandwidthEnumOptions.append("1000");
    RF_BandwidthEnumOptions.append("2000");
    RF_BandwidthEnumOptions.append("4000");
    RF_BandwidthEnumOptions.append("8000");
    RF_BandwidthEnumOptions.append("9600");
    RF_BandwidthEnumOptions.append("16000");
    RF_BandwidthEnumOptions.append("19200");
    RF_BandwidthEnumOptions.append("24000");
    RF_BandwidthEnumOptions.append("32000");
    RF_BandwidthEnumOptions.append("64000");
    RF_BandwidthEnumOptions.append("128000");
    RF_BandwidthEnumOptions.append("192000");
    fields.append( new UAVObjectField(QString("RF_Bandwidth"), QString("Hz"), UAVObjectField::ENUM, RF_BandwidthElemNames, RF_BandwidthEnumOptions) );
    QStringList Max_Tx_PowerElemNames;
    Max_Tx_PowerElemNames.append("0");
    QStringList Max_Tx_PowerEnumOptions;
    Max_Tx_PowerEnumOptions.append("1");
    Max_Tx_PowerEnumOptions.append("2");
    Max_Tx_PowerEnumOptions.append("5");
    Max_Tx_PowerEnumOptions.append("8");
    Max_Tx_PowerEnumOptions.append("11");
    Max_Tx_PowerEnumOptions.append("14");
    Max_Tx_PowerEnumOptions.append("17");
    Max_Tx_PowerEnumOptions.append("20");
    fields.append( new UAVObjectField(QString("Max_Tx_Power"), QString("dBm"), UAVObjectField::ENUM, Max_Tx_PowerElemNames, Max_Tx_PowerEnumOptions) );
    QStringList AES_EncryptionElemNames;
    AES_EncryptionElemNames.append("0");
    QStringList AES_EncryptionEnumOptions;
    AES_EncryptionEnumOptions.append("False");
    AES_EncryptionEnumOptions.append("True");
    fields.append( new UAVObjectField(QString("AES_Encryption"), QString(""), UAVObjectField::ENUM, AES_EncryptionElemNames, AES_EncryptionEnumOptions) );
    QStringList AES_EncryptionKeyElemNames;
    AES_EncryptionKeyElemNames.append("0");
    AES_EncryptionKeyElemNames.append("1");
    AES_EncryptionKeyElemNames.append("2");
    AES_EncryptionKeyElemNames.append("3");
    AES_EncryptionKeyElemNames.append("4");
    AES_EncryptionKeyElemNames.append("5");
    AES_EncryptionKeyElemNames.append("6");
    AES_EncryptionKeyElemNames.append("7");
    AES_EncryptionKeyElemNames.append("8");
    AES_EncryptionKeyElemNames.append("9");
    AES_EncryptionKeyElemNames.append("10");
    AES_EncryptionKeyElemNames.append("11");
    AES_EncryptionKeyElemNames.append("12");
    AES_EncryptionKeyElemNames.append("13");
    AES_EncryptionKeyElemNames.append("14");
    AES_EncryptionKeyElemNames.append("15");
    fields.append( new UAVObjectField(QString("AES_EncryptionKey"), QString(""), UAVObjectField::UINT8, AES_EncryptionKeyElemNames, QStringList()) );
    QStringList Paired_Serial_Number_CRCElemNames;
    Paired_Serial_Number_CRCElemNames.append("0");
    fields.append( new UAVObjectField(QString("Paired_Serial_Number_CRC"), QString(""), UAVObjectField::UINT32, Paired_Serial_Number_CRCElemNames, QStringList()) );

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
    data.Frequency_Calibration = 127;
    data.Frequency_Min = 0;
    data.Frequency_Max = 0;
    data.Frequency = 0;
    data.RF_Bandwidth = 11;
    data.Max_Tx_Power = 4;
    data.AES_Encryption = 0;
    data.AES_EncryptionKey[0] = 0;
    data.AES_EncryptionKey[1] = 0;
    data.AES_EncryptionKey[2] = 0;
    data.AES_EncryptionKey[3] = 0;
    data.AES_EncryptionKey[4] = 0;
    data.AES_EncryptionKey[5] = 0;
    data.AES_EncryptionKey[6] = 0;
    data.AES_EncryptionKey[7] = 0;
    data.AES_EncryptionKey[8] = 0;
    data.AES_EncryptionKey[9] = 0;
    data.AES_EncryptionKey[10] = 0;
    data.AES_EncryptionKey[11] = 0;
    data.AES_EncryptionKey[12] = 0;
    data.AES_EncryptionKey[13] = 0;
    data.AES_EncryptionKey[14] = 0;
    data.AES_EncryptionKey[15] = 0;
    data.Paired_Serial_Number_CRC = 0;

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
