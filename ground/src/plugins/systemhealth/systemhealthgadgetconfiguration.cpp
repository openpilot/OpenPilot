/**
 ******************************************************************************
 *
 * @file       systemhealthgadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      System Health Plugin Gadget configuration
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   systemhealthplugin
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

#include "systemhealthgadgetconfiguration.h"
#include <QtCore/QDataStream>

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
SystemHealthGadgetConfiguration::SystemHealthGadgetConfiguration(QString classId, const QByteArray &state, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    systemFile("Unknown")
{
    //if a saved configuration exists load it
    if (state.count() > 0) {
        QDataStream stream(state);
        stream >> systemFile;
    }
}
/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *SystemHealthGadgetConfiguration::clone()
{
    SystemHealthGadgetConfiguration *m = new SystemHealthGadgetConfiguration(this->classId());
    m->systemFile=systemFile;
    return m;
}
/**
 * Saves a configuration.
 *
 */
QByteArray SystemHealthGadgetConfiguration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << systemFile;

    return bytes;
}
