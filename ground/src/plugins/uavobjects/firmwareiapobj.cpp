/**
 ******************************************************************************
 *
 * @file       firmwareiapobj.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: firmwareiap.xml. 
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
#include "firmwareiapobj.h"
#include "uavobjectfield.h"

const QString FirmwareIAPObj::NAME = QString("FirmwareIAPObj");

/**
 * Constructor
 */
FirmwareIAPObj::FirmwareIAPObj(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList CommandElemNames;
    CommandElemNames.append("0");
    fields.append( new UAVObjectField(QString("Command"), QString("na"), UAVObjectField::UINT16, CommandElemNames, QStringList()) );
    QStringList DescriptionElemNames;
    DescriptionElemNames.append("0");
    DescriptionElemNames.append("1");
    DescriptionElemNames.append("2");
    DescriptionElemNames.append("3");
    DescriptionElemNames.append("4");
    DescriptionElemNames.append("5");
    DescriptionElemNames.append("6");
    DescriptionElemNames.append("7");
    DescriptionElemNames.append("8");
    DescriptionElemNames.append("9");
    DescriptionElemNames.append("10");
    DescriptionElemNames.append("11");
    DescriptionElemNames.append("12");
    DescriptionElemNames.append("13");
    DescriptionElemNames.append("14");
    DescriptionElemNames.append("15");
    DescriptionElemNames.append("16");
    DescriptionElemNames.append("17");
    DescriptionElemNames.append("18");
    DescriptionElemNames.append("19");
    DescriptionElemNames.append("20");
    DescriptionElemNames.append("21");
    DescriptionElemNames.append("22");
    DescriptionElemNames.append("23");
    DescriptionElemNames.append("24");
    DescriptionElemNames.append("25");
    DescriptionElemNames.append("26");
    DescriptionElemNames.append("27");
    DescriptionElemNames.append("28");
    DescriptionElemNames.append("29");
    DescriptionElemNames.append("30");
    DescriptionElemNames.append("31");
    DescriptionElemNames.append("32");
    DescriptionElemNames.append("33");
    DescriptionElemNames.append("34");
    DescriptionElemNames.append("35");
    DescriptionElemNames.append("36");
    DescriptionElemNames.append("37");
    DescriptionElemNames.append("38");
    DescriptionElemNames.append("39");
    DescriptionElemNames.append("40");
    DescriptionElemNames.append("41");
    DescriptionElemNames.append("42");
    DescriptionElemNames.append("43");
    DescriptionElemNames.append("44");
    DescriptionElemNames.append("45");
    DescriptionElemNames.append("46");
    DescriptionElemNames.append("47");
    DescriptionElemNames.append("48");
    DescriptionElemNames.append("49");
    DescriptionElemNames.append("50");
    DescriptionElemNames.append("51");
    DescriptionElemNames.append("52");
    DescriptionElemNames.append("53");
    DescriptionElemNames.append("54");
    DescriptionElemNames.append("55");
    DescriptionElemNames.append("56");
    DescriptionElemNames.append("57");
    DescriptionElemNames.append("58");
    DescriptionElemNames.append("59");
    DescriptionElemNames.append("60");
    DescriptionElemNames.append("61");
    DescriptionElemNames.append("62");
    DescriptionElemNames.append("63");
    DescriptionElemNames.append("64");
    DescriptionElemNames.append("65");
    DescriptionElemNames.append("66");
    DescriptionElemNames.append("67");
    DescriptionElemNames.append("68");
    DescriptionElemNames.append("69");
    DescriptionElemNames.append("70");
    DescriptionElemNames.append("71");
    DescriptionElemNames.append("72");
    DescriptionElemNames.append("73");
    DescriptionElemNames.append("74");
    DescriptionElemNames.append("75");
    DescriptionElemNames.append("76");
    DescriptionElemNames.append("77");
    DescriptionElemNames.append("78");
    DescriptionElemNames.append("79");
    DescriptionElemNames.append("80");
    DescriptionElemNames.append("81");
    DescriptionElemNames.append("82");
    DescriptionElemNames.append("83");
    DescriptionElemNames.append("84");
    DescriptionElemNames.append("85");
    DescriptionElemNames.append("86");
    DescriptionElemNames.append("87");
    DescriptionElemNames.append("88");
    DescriptionElemNames.append("89");
    DescriptionElemNames.append("90");
    DescriptionElemNames.append("91");
    DescriptionElemNames.append("92");
    DescriptionElemNames.append("93");
    DescriptionElemNames.append("94");
    DescriptionElemNames.append("95");
    DescriptionElemNames.append("96");
    DescriptionElemNames.append("97");
    DescriptionElemNames.append("98");
    DescriptionElemNames.append("99");
    fields.append( new UAVObjectField(QString("Description"), QString("na"), UAVObjectField::UINT8, DescriptionElemNames, QStringList()) );
    QStringList HWVersionElemNames;
    HWVersionElemNames.append("0");
    fields.append( new UAVObjectField(QString("HWVersion"), QString("na"), UAVObjectField::UINT8, HWVersionElemNames, QStringList()) );
    QStringList TargetElemNames;
    TargetElemNames.append("0");
    fields.append( new UAVObjectField(QString("Target"), QString("na"), UAVObjectField::UINT8, TargetElemNames, QStringList()) );
    QStringList ArmResetElemNames;
    ArmResetElemNames.append("0");
    fields.append( new UAVObjectField(QString("ArmReset"), QString("na"), UAVObjectField::UINT8, ArmResetElemNames, QStringList()) );
    QStringList crcElemNames;
    crcElemNames.append("0");
    fields.append( new UAVObjectField(QString("crc"), QString("na"), UAVObjectField::UINT32, crcElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata FirmwareIAPObj::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 1;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 1;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
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
void FirmwareIAPObj::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
FirmwareIAPObj::DataFields FirmwareIAPObj::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void FirmwareIAPObj::setData(const DataFields& data)
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
UAVDataObject* FirmwareIAPObj::clone(quint32 instID)
{
    FirmwareIAPObj* obj = new FirmwareIAPObj();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
FirmwareIAPObj* FirmwareIAPObj::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<FirmwareIAPObj*>(objMngr->getObject(FirmwareIAPObj::OBJID, instID));
}
