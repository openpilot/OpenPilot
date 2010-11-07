/**
 ******************************************************************************
 *
 * @file       gcscontrolgadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A gadget to control the UAV, either from the keyboard or a joystick
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

#include "gcscontrolgadgetconfiguration.h"

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
GCSControlGadgetConfiguration::GCSControlGadgetConfiguration(QString classId, QSettings* qSettings, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent)
{
    //if a saved configuration exists load it
    if(qSettings != 0) {
        /*
        BaudRateType speed;
        DataBitsType databits;
        FlowType flow;
        ParityType parity;
        StopBitsType stopbits;

        int ispeed = qSettings->value("defaultSpeed").toInt();
        int idatabits = qSettings->value("defaultDataBits").toInt();
        int iflow = qSettings->value("defaultFlow").toInt();
        int iparity = qSettings->value("defaultParity").toInt();
        int istopbits = qSettings->value("defaultStopBits").toInt();
        QString port = qSettings->value("defaultPort").toString();
        QString conMode = qSettings->value("connectionMode").toString();

        databits = (DataBitsType) idatabits;
        flow = (FlowType)iflow;
        parity = (ParityType)iparity;
        stopbits = (StopBitsType)istopbits;
        speed = (BaudRateType)ispeed;
        m_defaultPort = port;
        m_defaultSpeed = speed;
        m_defaultDataBits = databits;
        m_defaultFlow = flow;
        m_defaultParity = parity;
        m_defaultStopBits = stopbits;
        m_connectionMode = conMode;
        */
    }

}

/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *GCSControlGadgetConfiguration::clone()
{
    GCSControlGadgetConfiguration *m = new GCSControlGadgetConfiguration(this->classId());

    /*

    m->m_defaultSpeed = m_defaultSpeed;
    m->m_defaultDataBits = m_defaultDataBits;
    m->m_defaultFlow = m_defaultFlow;
    m->m_defaultParity = m_defaultParity;
    m->m_defaultStopBits = m_defaultStopBits;
    m->m_defaultPort = m_defaultPort;
    m->m_connectionMode = m_connectionMode;
    */
    return m;
}

/**
 * Saves a configuration.
 *
 */
void GCSControlGadgetConfiguration::saveConfig(QSettings* settings) const {
    /*
 settings->setValue("defaultSpeed", m_defaultSpeed);
 settings->setValue("defaultDataBits", m_defaultDataBits);
 settings->setValue("defaultFlow", m_defaultFlow);
 settings->setValue("defaultParity", m_defaultParity);
 settings->setValue("defaultStopBits", m_defaultStopBits);
 settings->setValue("defaultPort", m_defaultPort);
 settings->setValue("connectionMode", m_connectionMode);
*/
}
