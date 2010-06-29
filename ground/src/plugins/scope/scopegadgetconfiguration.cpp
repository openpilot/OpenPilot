/**
 ******************************************************************************
 *
 * @file       scopegadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Scope Plugin Gadget configuration
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   scopeplugin
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

#include "scopegadgetconfiguration.h"
#include <QtCore/QDataStream>

ScopeGadgetConfiguration::ScopeGadgetConfiguration(QString classId, const QByteArray &state, QObject *parent) :
        IUAVGadgetConfiguration(classId, parent)
{
}

/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *ScopeGadgetConfiguration::clone()
{
    ScopeGadgetConfiguration *m = new ScopeGadgetConfiguration(this->classId());
    //m->m_defaultDial=m_defaultDial;
    return m;
}
/**
 * Saves a configuration.
 *
 */
QByteArray ScopeGadgetConfiguration::saveState() const
{
    QByteArray bytes;
//    QDataStream stream(&bytes, QIODevice::WriteOnly);
//    stream << m_defaultDial;
//    stream << dialBackgroundID;
//    stream << dialForegroundID;
//    stream << dialNeedleID1;
//    stream << dialNeedleID2;
//    stream << needle1MinValue;
//    stream << needle1MaxValue;
//    stream << needle2MinValue;
//    stream << needle2MaxValue;
//    stream << needle1DataObject;
//    stream << needle1ObjectField;
//    stream << needle2DataObject;
//    stream << needle2ObjectField;
//    stream << needle1Factor;
//    stream << needle2Factor;

    return bytes;
}

