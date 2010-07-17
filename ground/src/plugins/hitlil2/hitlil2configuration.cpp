/**
 ******************************************************************************
 *
 * @file       hitlil2configuration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlil2plugin
 * @{
 * 
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

#include "hitlil2configuration.h"
#include <QtCore/QDataStream>

HITLIL2Configuration::HITLIL2Configuration(QString classId, const QByteArray &state, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
     m_il2HostName(""), m_il2Latitude(""), m_il2Longitude(""), m_il2Port(0), m_il2ManualControl(false)
{
    if (state.count() > 0) {
        QDataStream stream(state);
        QString il2HostName;
        QString il2Latitude;
        QString il2Longitude;
        int il2Port;
        bool il2ManualControl;
        stream >> il2HostName;
        m_il2HostName = il2HostName;
        stream >> il2Latitude;
        m_il2Latitude = il2Latitude;
        stream >> il2Longitude;
        m_il2Longitude = il2Longitude;
        stream >> il2Port;
        m_il2Port = il2Port;
        stream >> il2ManualControl;
        m_il2ManualControl = il2ManualControl;
    }
}

IUAVGadgetConfiguration *HITLIL2Configuration::clone()
{
    HITLIL2Configuration *m = new HITLIL2Configuration(this->classId());
    m->m_il2HostName = m_il2HostName;
    m->m_il2Latitude = m_il2Latitude;
    m->m_il2Longitude = m_il2Longitude;
    m->m_il2Port = m_il2Port;
    m->m_il2ManualControl = m_il2ManualControl;
    return m;
}

QByteArray HITLIL2Configuration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << m_il2HostName;
    stream << m_il2Latitude;
    stream << m_il2Longitude;
    stream << m_il2Port;
    stream << m_il2ManualControl;
    return bytes;
}

