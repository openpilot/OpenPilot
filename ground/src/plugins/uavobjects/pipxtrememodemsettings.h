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
        quint8 Mode;
        quint8 Serial_Baudrate;
        quint8 Frequency_Calibration;
        quint32 Frequency_Min;
        quint32 Frequency_Max;
        quint32 Frequency;
        quint8 Max_RF_Bandwidth;
        quint8 Max_Tx_Power;
        quint8 AES_Encryption;
        quint8 AES_EncryptionKey[16];
        quint32 Paired_Serial_Number;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field Mode information
    /* Enumeration options for field Mode */
    typedef enum { MODE_NORMAL=0, MODE_TEST_CARRIER=1, MODE_TEST_SPECTRUM=2 } ModeOptions;
    // Field Serial_Baudrate information
    /* Enumeration options for field Serial_Baudrate */
    typedef enum { SERIAL_BAUDRATE_1200=0, SERIAL_BAUDRATE_2400=1, SERIAL_BAUDRATE_4800=2, SERIAL_BAUDRATE_9600=3, SERIAL_BAUDRATE_19200=4, SERIAL_BAUDRATE_38400=5, SERIAL_BAUDRATE_57600=6, SERIAL_BAUDRATE_115200=7, SERIAL_BAUDRATE_230400=8 } Serial_BaudrateOptions;
    // Field Frequency_Calibration information
    // Field Frequency_Min information
    // Field Frequency_Max information
    // Field Frequency information
    // Field Max_RF_Bandwidth information
    /* Enumeration options for field Max_RF_Bandwidth */
    typedef enum { MAX_RF_BANDWIDTH_500=0, MAX_RF_BANDWIDTH_1000=1, MAX_RF_BANDWIDTH_2000=2, MAX_RF_BANDWIDTH_4000=3, MAX_RF_BANDWIDTH_8000=4, MAX_RF_BANDWIDTH_9600=5, MAX_RF_BANDWIDTH_16000=6, MAX_RF_BANDWIDTH_19200=7, MAX_RF_BANDWIDTH_24000=8, MAX_RF_BANDWIDTH_32000=9, MAX_RF_BANDWIDTH_64000=10, MAX_RF_BANDWIDTH_128000=11, MAX_RF_BANDWIDTH_192000=12 } Max_RF_BandwidthOptions;
    // Field Max_Tx_Power information
    /* Enumeration options for field Max_Tx_Power */
    typedef enum { MAX_TX_POWER_1=0, MAX_TX_POWER_2=1, MAX_TX_POWER_5=2, MAX_TX_POWER_8=3, MAX_TX_POWER_11=4, MAX_TX_POWER_14=5, MAX_TX_POWER_17=6, MAX_TX_POWER_20=7 } Max_Tx_PowerOptions;
    // Field AES_Encryption information
    /* Enumeration options for field AES_Encryption */
    typedef enum { AES_ENCRYPTION_FALSE=0, AES_ENCRYPTION_TRUE=1 } AES_EncryptionOptions;
    // Field AES_EncryptionKey information
    /* Number of elements for field AES_EncryptionKey */
    static const quint32 AES_ENCRYPTIONKEY_NUMELEM = 16;
    // Field Paired_Serial_Number information

  
    // Constants
    static const quint32 OBJID = 3822692478U;
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
