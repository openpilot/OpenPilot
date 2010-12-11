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
        quint32 Frequency;
        quint8 RFBandwidth;
        quint8 MaxTxPower;
        quint8 AESEncryption;
        quint8 AESEncryptionKey[16];
        quint32 PairedSerialNumberCRC;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field Frequency information
    // Field RFBandwidth information
    /* Enumeration options for field RFBandwidth */
    typedef enum { RFBANDWIDTH_500=0, RFBANDWIDTH_1000=1, RFBANDWIDTH_2000=2, RFBANDWIDTH_4000=3, RFBANDWIDTH_8000=4, RFBANDWIDTH_9600=5, RFBANDWIDTH_16000=6, RFBANDWIDTH_19200=7, RFBANDWIDTH_24000=8, RFBANDWIDTH_32000=9, RFBANDWIDTH_64000=10, RFBANDWIDTH_128000=11, RFBANDWIDTH_192000=12 } RFBandwidthOptions;
    // Field MaxTxPower information
    /* Enumeration options for field MaxTxPower */
    typedef enum { MAXTXPOWER_1=0, MAXTXPOWER_2=1, MAXTXPOWER_5=2, MAXTXPOWER_8=3, MAXTXPOWER_11=4, MAXTXPOWER_14=5, MAXTXPOWER_17=6, MAXTXPOWER_20=7 } MaxTxPowerOptions;
    // Field AESEncryption information
    /* Enumeration options for field AESEncryption */
    typedef enum { AESENCRYPTION_FALSE=0, AESENCRYPTION_TRUE=1 } AESEncryptionOptions;
    // Field AESEncryptionKey information
    /* Number of elements for field AESEncryptionKey */
    static const quint32 AESENCRYPTIONKEY_NUMELEM = 16;
    // Field PairedSerialNumberCRC information

  
    // Constants
    static const quint32 OBJID = 1441129524U;
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
