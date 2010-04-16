/**
 ******************************************************************************
 *
 * @file       uavobjectbrowserconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   uavobjectbrowser
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

#include "uavobjectbrowserconfiguration.h"
#include <QtCore/QDataStream>

UAVObjectBrowserConfiguration::UAVObjectBrowserConfiguration(QString classId, const QByteArray &state, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_recentlyUpdatedColor(QColor(255, 230, 230)),
    m_manuallyChangedColor(QColor(230, 230, 255)),
    m_recentlyUpdatedTimeout(500)
{
    if (state.count() > 0) {
        QDataStream stream(state);
        QColor recent;
        QColor manual;
        int timeout;
        stream >> recent;
        stream >> manual;
        stream >> timeout;
        m_recentlyUpdatedColor = recent;
        m_manuallyChangedColor = manual;
        m_recentlyUpdatedTimeout = timeout;
    }
}

IUAVGadgetConfiguration *UAVObjectBrowserConfiguration::clone()
{
    UAVObjectBrowserConfiguration *m = new UAVObjectBrowserConfiguration(this->classId());
    m->m_recentlyUpdatedColor = m_recentlyUpdatedColor;
    m->m_manuallyChangedColor = m_manuallyChangedColor;
    m->m_recentlyUpdatedTimeout = m_recentlyUpdatedTimeout;
    return m;
}

QByteArray UAVObjectBrowserConfiguration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << m_recentlyUpdatedColor;
    stream << m_manuallyChangedColor;
    stream << m_recentlyUpdatedTimeout;
    return bytes;
}

