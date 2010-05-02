/**
 ******************************************************************************
 *
 * @file       airspeedgadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Airspeed Plugin Gadget configuration
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   Airspeed
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

#include "airspeedgadgetconfiguration.h"
#include <QtCore/QDataStream>

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
AirspeedGadgetConfiguration::AirspeedGadgetConfiguration(QString classId, const QByteArray &state, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_defaultDial("Unknown"),
    dialBackgroundID("background"),
    dialForegroundID("foreground"),
    dialNeedleID1("needle"),
    dialNeedleID2("needle-2"),
    needle1MinValue(0.001),
    needle1MaxValue(100),
    needle2MinValue(0.001),
    needle2MaxValue(100)
{
    //if a saved configuration exists load it
    if (state.count() > 0) {
        QDataStream stream(state);
        QString dialFile;
        stream >> dialFile;
        m_defaultDial=dialFile;
        stream >> dialBackgroundID;
        stream >> dialForegroundID;
        stream >> dialNeedleID1;
        stream >> dialNeedleID2;
        stream >> needle1MinValue;
        stream >> needle1MaxValue;
        stream >> needle2MinValue;
        stream >> needle2MaxValue;

    }
}
/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *AirspeedGadgetConfiguration::clone()
{
    AirspeedGadgetConfiguration *m = new AirspeedGadgetConfiguration(this->classId());
    m->m_defaultDial=m_defaultDial;
    return m;
}
/**
 * Saves a configuration.
 *
 */
QByteArray AirspeedGadgetConfiguration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << m_defaultDial;
    stream << dialBackgroundID;
    stream << dialForegroundID;
    stream << dialNeedleID1;
    stream << dialNeedleID2;
    stream << needle1MinValue;
    stream << needle1MaxValue;
    stream << needle2MinValue;
    stream << needle2MaxValue;
    return bytes;
}
