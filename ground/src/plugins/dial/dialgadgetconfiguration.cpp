/**
 ******************************************************************************
 *
 * @file       dialgadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @addtogroup DialPlugin Dial Plugin
 * @{
 * @brief Dial Plugin Gadget configuration
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

#include "dialgadgetconfiguration.h"
#include <QtCore/QDataStream>

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
DialGadgetConfiguration::DialGadgetConfiguration(QString classId, const QByteArray &state, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_defaultDial("Unknown"),
    dialBackgroundID("background"),
    dialForegroundID("foreground"),
    dialNeedleID1("needle"),
    dialNeedleID2("needle2"),
    dialNeedleID3("needle3"),
    needle1MinValue(0),
    needle1MaxValue(100),
    needle2MinValue(0),
    needle2MaxValue(100),
    needle3MinValue(0),
    needle3MaxValue(100),
    needle1Factor(1),
    needle2Factor(1),
    needle3Factor(1),
    needle1Move("Rotate"),
    needle2Move("Rotate"),
    needle3Move("Rotate")
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
        stream >> dialNeedleID3;
        stream >> needle1MinValue;
        stream >> needle1MaxValue;
        stream >> needle2MinValue;
        stream >> needle2MaxValue;
        stream >> needle3MinValue;
        stream >> needle3MaxValue;
        stream >> needle1DataObject;
        stream >> needle1ObjectField;
        stream >> needle2DataObject;
        stream >> needle2ObjectField;
        stream >> needle3DataObject;
        stream >> needle3ObjectField;
        stream >> needle1Factor;
        stream >> needle2Factor;
        stream >> needle3Factor;
        stream >> needle1Move;
        stream >> needle2Move;
        stream >> needle3Move;
        stream >> font;

    }
}
/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *DialGadgetConfiguration::clone()
{
    DialGadgetConfiguration *m = new DialGadgetConfiguration(this->classId());
    m->m_defaultDial=m_defaultDial;
    return m;
}
/**
 * Saves a configuration.
 *
 */
QByteArray DialGadgetConfiguration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << m_defaultDial;
    stream << dialBackgroundID;
    stream << dialForegroundID;
    stream << dialNeedleID1;
    stream << dialNeedleID2;
    stream << dialNeedleID3;
    stream << needle1MinValue;
    stream << needle1MaxValue;
    stream << needle2MinValue;
    stream << needle2MaxValue;
    stream << needle3MinValue;
    stream << needle3MaxValue;
    stream << needle1DataObject;
    stream << needle1ObjectField;
    stream << needle2DataObject;
    stream << needle2ObjectField;
    stream << needle3DataObject;
    stream << needle3ObjectField;
    stream << needle1Factor;
    stream << needle2Factor;
    stream << needle3Factor;
    stream << needle1Move;
    stream << needle2Move;
    stream << needle3Move;
    stream << font;

    return bytes;
}
