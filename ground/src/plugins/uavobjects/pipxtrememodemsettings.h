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
        quint8 Frequency_Calibration;
        quint32 Frequency_Min;
        quint32 Frequency_Max;
        quint32 Frequency;
        quint8 RF_Bandwidth;
        quint8 Max_Tx_Power;
        quint8 AES_Encryption;
        quint8 AES_EncryptionKey[16];
        quint32 Paired_Serial_Number_CRC;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field Frequency_Calibration information
    // Field Frequency_Min information
    // Field Frequency_Max information
    // Field Frequency information
    // Field RF_Bandwidth information
    /* Enumeration options for field RF_Bandwidth */
    typedef enum { RF_BANDWIDTH_500=0, RF_BANDWIDTH_1000=1, RF_BANDWIDTH_2000=2, RF_BANDWIDTH_4000=3, RF_BANDWIDTH_8000=4, RF_BANDWIDTH_9600=5, RF_BANDWIDTH_16000=6, RF_BANDWIDTH_19200=7, RF_BANDWIDTH_24000=8, RF_BANDWIDTH_32000=9, RF_BANDWIDTH_64000=10, RF_BANDWIDTH_128000=11, RF_BANDWIDTH_192000=12 } RF_BandwidthOptions;
    // Field Max_Tx_Power information
    /* Enumeration options for field Max_Tx_Power */
    typedef enum { MAX_TX_POWER_1=0, MAX_TX_POWER_2=1, MAX_TX_POWER_5=2, MAX_TX_POWER_8=3, MAX_TX_POWER_11=4, MAX_TX_POWER_14=5, MAX_TX_POWER_17=6, MAX_TX_POWER_20=7 } Max_Tx_PowerOptions;
    // Field AES_Encryption information
    /* Enumeration options for field AES_Encryption */
    typedef enum { AES_ENCRYPTION_FALSE=0, AES_ENCRYPTION_TRUE=1 } AES_EncryptionOptions;
    // Field AES_EncryptionKey information
    /* Number of elements for field AES_EncryptionKey */
    static const quint32 AES_ENCRYPTIONKEY_NUMELEM = 16;
    // Field Paired_Serial_Number_CRC information

  
    // Constants
    static const quint32 OBJID = 2660664364U;
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
