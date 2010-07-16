/**
 ******************************************************************************
 *
 * @file       uploadergadgetconfiguration.cpp
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

#include "uploadergadgetconfiguration.h"
#include <QtCore/QDataStream>
#include <qextserialport/src/qextserialport.h>

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
UploaderGadgetConfiguration::UploaderGadgetConfiguration(QString classId, const QByteArray &state, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_defaultPort("Unknown"),
    m_defaultSpeed(BAUD19200),
    m_defaultDataBits(DATA_8),
    m_defaultFlow(FLOW_OFF),
    m_defaultParity(PAR_NONE),
    m_defaultStopBits(STOP_1),
    m_defaultTimeOut(5000)

{
    //if a saved configuration exists load it
    if (state.count() > 0) {
        QDataStream stream(state);
        BaudRateType speed;
        DataBitsType databits;
        FlowType flow;
        ParityType parity;
        StopBitsType stopbits;
        int ispeed;
        int idatabits;
        int iflow;
        int iparity;
        int istopbits;
        QString port;
        stream >> ispeed;
        stream >> idatabits;
        stream >>iflow;
        stream >>iparity;
        stream >> istopbits;
        stream >> port;

        databits=(DataBitsType) idatabits;
        flow=(FlowType)iflow;
        parity=(ParityType)iparity;
        stopbits=(StopBitsType)istopbits;
        speed=(BaudRateType)ispeed;
        m_defaultPort=port;
        m_defaultSpeed=speed;
        m_defaultDataBits=databits;
        m_defaultFlow=flow;
        m_defaultParity=parity;
        m_defaultStopBits=stopbits;

    }

}
/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *UploaderGadgetConfiguration::clone()
{
    UploaderGadgetConfiguration *m = new UploaderGadgetConfiguration(this->classId());

    m->m_defaultSpeed=m_defaultSpeed;
    m->m_defaultDataBits=m_defaultDataBits;
    m->m_defaultFlow=m_defaultFlow;
    m->m_defaultParity=m_defaultParity;
    m->m_defaultStopBits=m_defaultStopBits;
    m->m_defaultPort=m_defaultPort;
    return m;
}
/**
 * Saves a configuration.
 *
 */
QByteArray UploaderGadgetConfiguration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << (int)m_defaultSpeed;
    stream << (int)m_defaultDataBits;
    stream << (int)m_defaultFlow;
    stream << (int)m_defaultParity;
    stream << (int)m_defaultStopBits;
    stream << m_defaultPort;
    return bytes;
}

