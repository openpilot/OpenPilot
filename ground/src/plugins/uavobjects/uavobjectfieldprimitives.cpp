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
#include "uavobjectfieldprimitives.h"

/**
 * Constructor
 */
template <typename FType>
UAVObjectFieldPrimitives<FType>::UAVObjectFieldPrimitives(const QString& name, const QString& units, quint32 numElements):
        UAVObjectField(name, units, numElements)
{
    numBytesPerElement = sizeof(FType);
}

template <typename FType>
void UAVObjectFieldPrimitives<FType>::initializeValues()
{
    for (quint32 n = 0; n < numElements; ++n)
    {
        setValue(0, n);
    }
}

/**
 * Pack the field in to an array of bytes.
 */
template <typename FType>
qint32 UAVObjectFieldPrimitives<FType>::pack(quint8* dataOut)
{
    QMutexLocker locker(obj->getMutex());
    // Pack each element in output buffer
    for (quint32 index = 0; index < numElements; ++index)
    {
        FType value;
        memcpy(&value, &data[offset + numBytesPerElement*index], numBytesPerElement);
        qToBigEndian<FType>(value, &dataOut[numBytesPerElement*index]);
    }
    // Done
    return getNumBytes();
}

/**
 * Unpack the field from an array of bytes.
 */
template <typename FType>
qint32 UAVObjectFieldPrimitives<FType>::unpack(const quint8* dataIn)
{
    QMutexLocker locker(obj->getMutex());
    // Pack each element in output buffer
    for (quint32 index = 0; index < numElements; ++index)
    {
        FType value;
        value = qFromBigEndian<FType>(&dataIn[numBytesPerElement*index]);
        memcpy(&data[offset + numBytesPerElement*index], &value, numBytesPerElement);
    }
    // Done
    return getNumBytes();
}

/**
 * Get the field value as a double.
 */
template <typename FType>
double UAVObjectFieldPrimitives<FType>::getDouble(quint32 index)
{
    QMutexLocker locker(obj->getMutex());
    double ret = 0.0;
    // Check if index is out of bounds or no data available
    if (index < numElements && data != NULL)
    {
        FType value;
        memcpy(&value, &data[offset + numBytesPerElement*index], numBytesPerElement);
        ret = (double)value;
    }
    // Done
    return ret;
}

/**
 * Set the field value from a double.
 */
template <typename FType>
void UAVObjectFieldPrimitives<FType>::setDouble(double value, quint32 index)
{
    QMutexLocker locker(obj->getMutex());
    // Check if index is out of bounds or no data available
    if (index < numElements && data != NULL)
    {
        FType tmpValue;
        tmpValue = (FType)value;
        memcpy(&data[offset + numBytesPerElement*index], &tmpValue, numBytesPerElement);
    }

    // Emit updated event
    emit fieldUpdated(this);
}

/**
 * Get the number of bytes per field element.
 */
template <typename FType>
quint32 UAVObjectFieldPrimitives<FType>::getNumBytesElement()
{
    return numBytesPerElement;
}

/**
 * Get the field value.
 */
template <typename FType>
FType UAVObjectFieldPrimitives<FType>::getValue(quint32 index)
{
    QMutexLocker locker(obj->getMutex());
    // Check if index is out of bounds or no data available
    if (index < numElements && data != NULL)
    {
        FType value;
        memcpy(&value, &data[offset + numBytesPerElement*index], numBytesPerElement);
        return value;
    }
    else
    {
        return 0;
    }
}

/**
 * Set the field value.
 */
template <typename FType>
void UAVObjectFieldPrimitives<FType>::setValue(FType value, quint32 index)
{
    QMutexLocker locker(obj->getMutex());
    // Check if index is out of bounds or no data available
    if (index < numElements && data != NULL)
    {
        memcpy(&data[offset + numBytesPerElement*index], &value, numBytesPerElement);
    }

    // Emit updated event
    emit fieldUpdated(this);
}

/**
 * Pre-define valid templates
 */
template class UAVObjectFieldPrimitives<qint8>;
template class UAVObjectFieldPrimitives<qint16>;
template class UAVObjectFieldPrimitives<qint32>;
template class UAVObjectFieldPrimitives<quint8>;
template class UAVObjectFieldPrimitives<quint16>;
template class UAVObjectFieldPrimitives<quint32>;
template class UAVObjectFieldPrimitives<float>;



