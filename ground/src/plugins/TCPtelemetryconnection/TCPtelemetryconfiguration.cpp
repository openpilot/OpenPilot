/**
 ******************************************************************************
 *
 * @file       TCPtelemetryconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   map
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

#include "TCPtelemetryconfiguration.h"
#include <QtCore/QDataStream>

TCPtelemetryConfiguration::TCPtelemetryConfiguration(QString classId, const QByteArray &state, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_HostName("127.0.0.1"),
    m_Port(1000)
{
    if (state.count() > 0) {
        QDataStream stream(state);
        int Port;
        QString HostName;
        stream >> Port;
        stream >> HostName;
        m_Port = Port;
        if (HostName != "")
            m_HostName = HostName;

    }
}

IUAVGadgetConfiguration *TCPtelemetryConfiguration::clone()
{
    TCPtelemetryConfiguration *m = new TCPtelemetryConfiguration(this->classId());
    m->m_Port = m_Port;
    m->m_HostName = m_HostName;
    return m;
}

QByteArray TCPtelemetryConfiguration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << m_Port;
    stream << m_HostName;
    return bytes;
}

