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

UAVObjectField::UAVObjectField(const QString& name, const QString& units, FieldType type, quint32 numElements)
{
    // Copy params
    this->name = name;
    this->units = units;
    this->type = type;
    this->numElements = numElements;
    this->offset = 0;
    this->data = NULL;
    this->obj = NULL;
    // Calculate the number of bytes per element based on the type
    switch (type) {
    case FIELDTYPE_CHAR:
    case FIELDTYPE_INT8:
        this->numBytesPerElement = 1;
        break;
    case FIELDTYPE_INT16:
        this->numBytesPerElement = 2;
        break;
    case FIELDTYPE_INT32:
    case FIELDTYPE_FLOAT32:
        this->numBytesPerElement = 4;
        break;
    default:
        this->numBytesPerElement = 0;
    }
}

void UAVObjectField::initialize(quint8* data, quint32 dataOffset, UAVObject* obj)
{
    this->data = data;
    this->offset = dataOffset;
    this->obj = obj;
    clear();
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
        for (unsigned int n = 0; n < numBytesPerElement*numElements; ++n)
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

UAVObjectField::FieldType UAVObjectField::getType()
{
    return type;
}

quint32 UAVObjectField::getNumElements()
{
    return numElements;
}

qint32 UAVObjectField::pack(quint8* dataOut)
{
    QMutexLocker locker(obj->getMutex());
    // Pack each element in output buffer
    for (quint32 index = 0; index < numElements; ++index)
    {
        switch (type) {
        case FIELDTYPE_CHAR:
        case FIELDTYPE_INT8:
            dataOut[numBytesPerElement*index] = data[offset + numBytesPerElement*index];
            break;
        case FIELDTYPE_INT16:
            qint16 value16;
            memcpy(&value16, &data[offset + numBytesPerElement*index], 2);
            qToBigEndian<qint16>(value16, &dataOut[numBytesPerElement*index]);
            break;
        case FIELDTYPE_INT32:
        case FIELDTYPE_FLOAT32:
            qint32 value32;
            memcpy(&value32, &data[offset + numBytesPerElement*index], 4);
            qToBigEndian<qint32>(value32, &dataOut[numBytesPerElement*index]);
            break;
        default:
            return 0;
        }
    }
    // Done
    return getNumBytes();
}

qint32 UAVObjectField::unpack(const quint8* dataIn)
{
    QMutexLocker locker(obj->getMutex());
    // Pack each element in output buffer
    for (quint32 index = 0; index < numElements; ++index)
    {
        switch (type) {
        case FIELDTYPE_CHAR:
        case FIELDTYPE_INT8:
            data[offset + numBytesPerElement*index] = dataIn[numBytesPerElement*index];
            break;
        case FIELDTYPE_INT16:
            qint16 value16;
            value16 = qFromBigEndian<qint16>(&dataIn[numBytesPerElement*index]);
            memcpy(&data[offset + numBytesPerElement*index], &value16, 2);
            break;
        case FIELDTYPE_INT32:
        case FIELDTYPE_FLOAT32:
            qint32 value32;
            value32 = qFromBigEndian<qint32>(&dataIn[numBytesPerElement*index]);
            memcpy(&data[offset + numBytesPerElement*index], &value32, 4);
            break;
        default:
            return 0;
        }
    }
    // Done
    return getNumBytes();
}

double UAVObjectField::getValue(quint32 index)
{
    double ret = 0.0;

    // Check if index is out of bounds or no data available
    if (index < numElements && data != NULL)
    {
        // Get value from data
        QMutexLocker locker(obj->getMutex());
        switch (type) {
        case FIELDTYPE_CHAR:
        case FIELDTYPE_INT8:
            qint8 value8;
            value8 = data[offset + numBytesPerElement*index];
            ret = (double)value8;
            break;
        case FIELDTYPE_INT16:
            qint16 value16;
            memcpy(&value16, &data[offset + numBytesPerElement*index], 2);
            ret = (double)value16;
            break;
        case FIELDTYPE_INT32:
            qint32 value32;
            memcpy(&value32, &data[offset + numBytesPerElement*index], 4);
            ret = (double)value32;
            break;
        case FIELDTYPE_FLOAT32:
            float valuef;
            memcpy(&valuef, &data[offset + numBytesPerElement*index], 4);
            ret = (double)valuef;
            break;
        default:
            ret = 0.0;
        }
    }
    return ret;
}

void UAVObjectField::setValue(double value, quint32 index)
{
    // Check if index is out of bounds or no data available
    if (index < numElements && data != NULL)
    {
        // Set value
        QMutexLocker locker(obj->getMutex());
        switch (type) {
        case FIELDTYPE_CHAR:
        case FIELDTYPE_INT8:
            data[offset + numBytesPerElement*index] = (qint8)value;
            break;
        case FIELDTYPE_INT16:
            qint16 value16;
            value16 = (qint16)value;
            memcpy(&data[offset + numBytesPerElement*index], &value16, 2);
            break;
        case FIELDTYPE_INT32:
            qint32 value32;
            value32 = (qint32)value;
            memcpy(&data[offset + numBytesPerElement*index], &value32, 4);
            break;
        case FIELDTYPE_FLOAT32:
            float valuef;
            valuef = (float)value;
            memcpy(&data[offset + numBytesPerElement*index], &valuef, 4);
            break;            
        }
    }

    // Emit updated event
    emit fieldUpdated(this);
}

double UAVObjectField::getValue()
{
    return getValue(0);
}

void UAVObjectField::setValue(double value)
{
    setValue(value, 0);
}


QString UAVObjectField::getString()
{
    QString str;
    if (data != NULL)
    {
        QMutexLocker locker(obj->getMutex());
        data[offset + numElements - 1] = '\0'; // null terminate
        if (type == FIELDTYPE_CHAR)
        {
            str.append((char*)&data[offset]);
        }
    }
    return str;
}

void UAVObjectField::setString(QString& str)
{
    QByteArray barray = str.toAscii();
    if (data != NULL)
    {
        QMutexLocker locker(obj->getMutex());
        for (int n = 0; n < barray.length() && n < (int)numElements; ++n)
        {
            data[offset+n] = barray[n];
        }
        data[offset + numElements - 1] = '\0'; // null terminate
    }
}

quint32 UAVObjectField::getDataOffset()
{
    return offset;
}

quint32 UAVObjectField::getNumBytes()
{
    return numBytesPerElement * numElements;
}

quint32 UAVObjectField::getNumBytesElement()
{
    return numBytesPerElement;
}

QString UAVObjectField::toString()
{
    QString sout;
    sout.append ( QString("%1: [ ").arg(name) );
    for (unsigned int n = 0; n < numElements; ++n)
    {
        sout.append( QString("%1 ").arg(getValue(n)) );
    }
    sout.append( QString("] %1\n").arg(units) );
    return sout;
}



