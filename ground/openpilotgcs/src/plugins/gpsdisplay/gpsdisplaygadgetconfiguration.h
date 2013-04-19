/**
 ******************************************************************************
 *
 * @file       gpsdisplaygadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GPSGadgetPlugin GPS Gadget Plugin
 * @{
 * @brief A gadget that displays GPS status and enables basic configuration 
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

#ifndef GPSDISPLAYGADGETCONFIGURATION_H
#define GPSDISPLAYGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include <qextserialport/src/qextserialport.h>

using namespace Core;

class GpsDisplayGadgetConfiguration : public IUAVGadgetConfiguration
{
    Q_OBJECT
    public:
        explicit GpsDisplayGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

        void setConnectionMode(QString mode) { m_connectionMode = mode; }
        QString connectionMode() { return m_connectionMode; }

        //set port configuration functions
        void setSpeed(BaudRateType speed) {m_defaultSpeed=speed;}
        void setDataBits(DataBitsType databits) {m_defaultDataBits=databits;}
        void setFlow(FlowType flow) {m_defaultFlow=flow;}
        void setParity(ParityType parity) {m_defaultParity=parity;}
        void setStopBits(StopBitsType stopbits) {m_defaultStopBits=stopbits;}
        void setPort(QString port){m_defaultPort=port;}
        void setTimeOut(long timeout){m_defaultTimeOut=timeout;}

        //get port configuration functions
        QString port(){return m_defaultPort;}
        BaudRateType speed() {return m_defaultSpeed;}
        FlowType flow() {return m_defaultFlow;}
        DataBitsType dataBits() {return m_defaultDataBits;}
        StopBitsType stopBits() {return m_defaultStopBits;}
        ParityType parity() {return m_defaultParity;}
        long timeOut(){return m_defaultTimeOut;}

        void saveConfig(QSettings* settings) const;
        IUAVGadgetConfiguration *clone();

    private:
        QString m_connectionMode;
        QString m_defaultPort;
        BaudRateType m_defaultSpeed;
        DataBitsType m_defaultDataBits;
        FlowType m_defaultFlow;
        ParityType m_defaultParity;
        StopBitsType m_defaultStopBits;
        long m_defaultTimeOut;

};

#endif // GPSDISPLAYGADGETCONFIGURATION_H
