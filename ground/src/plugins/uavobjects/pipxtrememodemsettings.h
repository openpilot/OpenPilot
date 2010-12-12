/**
 ******************************************************************************
 *
 * @file       pipxtrememodemsettings.h
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
#ifndef PIPXTREMEMODEMSETTINGS_H
#define PIPXTREMEMODEMSETTINGS_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT PipXtremeModemSettings: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint8 Frequency calibration;
        quint32 Frequency min;
        quint32 Frequency max;
        quint32 Frequency;
        quint8 RF bandwidth;
        quint8 Max Tx power;
        quint8 AESEncryption;
        quint8 AESEncryptionKey[16];
        quint32 PairedSerialNumberCRC;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field Frequency calibration information
    // Field Frequency min information
    // Field Frequency max information
    // Field Frequency information
    // Field RF bandwidth information
    /* Enumeration options for field RF bandwidth */
    typedef enum { RF BANDWIDTH_500=0, RF BANDWIDTH_1000=1, RF BANDWIDTH_2000=2, RF BANDWIDTH_4000=3, RF BANDWIDTH_8000=4, RF BANDWIDTH_9600=5, RF BANDWIDTH_16000=6, RF BANDWIDTH_19200=7, RF BANDWIDTH_24000=8, RF BANDWIDTH_32000=9, RF BANDWIDTH_64000=10, RF BANDWIDTH_128000=11, RF BANDWIDTH_192000=12 } RF bandwidthOptions;
    // Field Max Tx power information
    /* Enumeration options for field Max Tx power */
    typedef enum { MAX TX POWER_1=0, MAX TX POWER_2=1, MAX TX POWER_5=2, MAX TX POWER_8=3, MAX TX POWER_11=4, MAX TX POWER_14=5, MAX TX POWER_17=6, MAX TX POWER_20=7 } Max Tx powerOptions;
    // Field AESEncryption information
    /* Enumeration options for field AESEncryption */
    typedef enum { AESENCRYPTION_FALSE=0, AESENCRYPTION_TRUE=1 } AESEncryptionOptions;
    // Field AESEncryptionKey information
    /* Number of elements for field AESEncryptionKey */
    static const quint32 AESENCRYPTIONKEY_NUMELEM = 16;
    // Field PairedSerialNumberCRC information

  
    // Constants
    static const quint32 OBJID = 3841416984U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 1;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    PipXtremeModemSettings();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static PipXtremeModemSettings* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // PIPXTREMEMODEMSETTINGS_H
