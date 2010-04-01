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

#include "uavobjectfieldstring.h"

UAVObjectFieldString::UAVObjectFieldString(const QString& name, quint32 maxSize):
        UAVObjectFieldPrimitives<quint8>(name, QString(""), maxSize)
{
}

QString UAVObjectFieldString::getString()
{
    QMutexLocker locker(obj->getMutex());
    // Null terminate last element
    setValue('\0', numElements - 1);
    // Read characters into string until a null is received
    QString str;
    quint8 ch;
    for (quint32 index = 0; index < numElements; ++index)
    {
        // Get character and check if end of string is received
        ch = getValue(index);
        if ( ch != '\0' )
        {
            str.append(ch);
        }
        else
        {
            break;
        }
    }
    // Done
    return str;
}

void UAVObjectFieldString::setString(QString& str)
{
    QMutexLocker locker(obj->getMutex());
    // Copy string to data
    QByteArray barray = str.toAscii();
    quint32 index;
    for (index = 0; index < (quint32)barray.length() && index < (numElements-1); ++index)
    {
        setValue(barray[index], index);
    }
    // Null terminate
    setValue('\0', index);
}


