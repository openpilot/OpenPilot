/**
 ******************************************************************************
 *
 * @file       uavobjectfield.cpp
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
#include "uavobjectfield.h"
#include <QtEndian>

UAVObjectField::UAVObjectField(const QString& name, const QString& units, quint32 numElements)
{
    // Copy params
    this->name = name;
    this->units = units;
    this->numElements = numElements;
    this->offset = 0;
    this->data = NULL;
    this->obj = NULL;
}

void UAVObjectField::initialize(quint8* data, quint32 dataOffset, UAVObject* obj)
{
    this->data = data;
    this->offset = dataOffset;
    this->obj = obj;
    clear();
    initializeValues();
}

UAVObject* UAVObjectField::getObject()
{
    return obj;
}

void UAVObjectField::clear()
{
    if (data != NULL)
    {
        QMutexLocker locker(obj->getMutex());
        for (unsigned int n = 0; n < getNumBytesElement()*numElements; ++n)
        {
            data[offset + n] = 0;
        }
    }
}

QString UAVObjectField::getName()
{
    return name;
}

QString UAVObjectField::getUnits()
{
    return units;
}

quint32 UAVObjectField::getNumElements()
{
    return numElements;
}

quint32 UAVObjectField::getDataOffset()
{
    return offset;
}

quint32 UAVObjectField::getNumBytes()
{
    return getNumBytesElement() * numElements;
}

QString UAVObjectField::toString()
{
    QString sout;
    sout.append ( QString("%1: [ ").arg(name) );
    for (unsigned int n = 0; n < numElements; ++n)
    {
        sout.append( QString("%1 ").arg(getDouble(n)) );
    }
    sout.append( QString("] %1\n").arg(units) );
    return sout;
}



