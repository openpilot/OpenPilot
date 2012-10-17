/**
 ******************************************************************************
 *
 * @file       uploadergadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup YModemUploader YModem Serial Uploader Plugin
 * @{
 * @brief The YModem protocol serial uploader plugin
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

#ifndef UPLOADERGADGETCONFIGURATION_H
#define UPLOADERGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include <qextserialport/src/qextserialport.h>
#include "uploader_global.h"

using namespace Core;

class UPLOADER_EXPORT UploaderGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit UploaderGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    //set port configuration functions
    void setSpeed(BaudRateType speed) {m_defaultSpeed=speed;}
    void setDataBits(DataBitsType databits) {m_defaultDataBits=databits;}
    void setFlow(FlowType flow) {m_defaultFlow=flow;}
    void setParity(ParityType parity) {m_defaultParity=parity;}
    void setStopBits(StopBitsType stopbits) {m_defaultStopBits=stopbits;}
    void setPort(QString port){m_defaultPort=port;}
    void setTimeOut(long timeout){m_defaultTimeOut=timeout;}

    //get port configuration functions
    BaudRateType Speed() {return m_defaultSpeed;}
    DataBitsType DataBits() {return m_defaultDataBits;}
    FlowType Flow() {return m_defaultFlow;}
    ParityType Parity() {return m_defaultParity;}
    StopBitsType StopBits() {return m_defaultStopBits;}
    QString Port(){return m_defaultPort;}
    long TimeOut(){return m_defaultTimeOut;}

    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();

private:
    QString m_defaultPort;
    BaudRateType m_defaultSpeed;
    DataBitsType m_defaultDataBits;
    FlowType m_defaultFlow;
    ParityType m_defaultParity;
    StopBitsType m_defaultStopBits;
    long m_defaultTimeOut;
};

#endif // UPLOADERGADGETCONFIGURATION_H
