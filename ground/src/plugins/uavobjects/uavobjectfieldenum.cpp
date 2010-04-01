/**
 ******************************************************************************
 *
 * @file       uavobjectfieldenum.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   uavobjects_plugin
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

#include "uavobjectfieldenum.h"

UAVObjectFieldEnum::UAVObjectFieldEnum(const QString& name, const QString& units, quint32 numElements, QStringList& options):
        UAVObjectFieldPrimitives<quint8>(name, units, numElements)
{
    this->options = options;
}

QStringList UAVObjectFieldEnum::getOptions()
{
    return options;
}

QString UAVObjectFieldEnum::getSelected()
{
    return options[getValue()];
}

void UAVObjectFieldEnum::setSelected(QString& val)
{
    // Find index of selected value
    int index = options.indexOf(val);
    if (index >= 0)
    {
        setValue(index);
    }
}

quint8 UAVObjectFieldEnum::getSelectedIndex()
{
    return getValue();
}

void UAVObjectFieldEnum::setSelectedIndex(quint8 index)
{
    // Check that the index is valid
    if (index < options.length())
    {
        setValue(index);
    }
}
