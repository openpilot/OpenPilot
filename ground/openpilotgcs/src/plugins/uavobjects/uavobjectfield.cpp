/**
 ******************************************************************************
 *
 * @file       uavobjectfield.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 * @brief      The UAVUObjects GCS plugin
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
#include <QDebug>

UAVObjectField::UAVObjectField(const QString& name, const QString& units, FieldType type, quint32 numElements, const QStringList& options)
{
    QStringList elementNames;
    // Set element names
    for (quint32 n = 0; n < numElements; ++n)
    {
        elementNames.append(QString("%1").arg(n));
    }
    // Initialize
    constructorInitialize(name, units, type, elementNames, options);

}

UAVObjectField::UAVObjectField(const QString& name, const QString& units, FieldType type, const QStringList& elementNames, const QStringList& options)
{
    constructorInitialize(name, units, type, elementNames, options);
}

void UAVObjectField::constructorInitialize(const QString& name, const QString& units, FieldType type, const QStringList& elementNames, const QStringList& options)
{
    // Copy params
    this->name = name;
    this->units = units;
    this->type = type;
    this->options = options;
    this->numElements = elementNames.length();
    this->offset = 0;
    this->data = NULL;
    this->obj = NULL;
    this->elementNames = elementNames;
    // Set field size
    switch (type)
    {
        case INT8:
            numBytesPerElement = sizeof(qint8);
            break;
        case INT16:
            numBytesPerElement = sizeof(qint16);
            break;
        case INT32:
            numBytesPerElement = sizeof(qint32);
            break;
        case UINT8:
            numBytesPerElement = sizeof(quint8);
            break;
        case UINT16:
            numBytesPerElement = sizeof(quint16);
            break;
        case UINT32:
            numBytesPerElement = sizeof(quint32);
            break;
        case FLOAT32:
            numBytesPerElement = sizeof(quint32);
            break;
        case ENUM:
            numBytesPerElement = sizeof(quint8);
            break;
        case STRING:
            numBytesPerElement = sizeof(quint8);
            break;
        default:
            numBytesPerElement = 0;
    }
}

void UAVObjectField::initialize(quint8* data, quint32 dataOffset, UAVObject* obj)
{
    this->data = data;
    this->offset = dataOffset;
    this->obj = obj;
    clear();
}

UAVObjectField::FieldType UAVObjectField::getType()
{
    return type;
}

QString UAVObjectField::getTypeAsString()
{
    switch (type)
    {
        case UAVObjectField::INT8:
            return "int8";
        case UAVObjectField::INT16:
            return "int16";
        case UAVObjectField::INT32:
            return "int32";
        case UAVObjectField::UINT8:
            return "uint8";
        case UAVObjectField::UINT16:
            return "uint16";
        case UAVObjectField::UINT32:
            return "uint32";
        case UAVObjectField::FLOAT32:
            return "float32";
        case UAVObjectField::ENUM:
            return "enum";
        case UAVObjectField::STRING:
            return "string";
        default:
            return "";
    }
}

QStringList UAVObjectField::getElementNames()
{
    return elementNames;
}

UAVObject* UAVObjectField::getObject()
{
    return obj;
}

void UAVObjectField::clear()
{
    QMutexLocker locker(obj->getMutex());
    memset(&data[offset], 0, numBytesPerElement*numElements);
}

QString UAVObjectField::getName()
{
    return name;
}

QString UAVObjectField::getUnits()
{
    return units;
}

QStringList UAVObjectField::getOptions()
{
    return options;
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
    return numBytesPerElement * numElements;
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


qint32 UAVObjectField::pack(quint8* dataOut)
{
    QMutexLocker locker(obj->getMutex());
    // Pack each element in output buffer
    switch (type)
    {
        case INT8:
            memcpy(dataOut, &data[offset], numElements);
            break;
        case INT16:
            for (quint32 index = 0; index < numElements; ++index)
            {
                qint16 value;
                memcpy(&value, &data[offset + numBytesPerElement*index], numBytesPerElement);
                qToLittleEndian<qint16>(value, &dataOut[numBytesPerElement*index]);
            }
            break;
        case INT32:
            for (quint32 index = 0; index < numElements; ++index)
            {
                qint32 value;
                memcpy(&value, &data[offset + numBytesPerElement*index], numBytesPerElement);
                qToLittleEndian<qint32>(value, &dataOut[numBytesPerElement*index]);
            }
            break;
        case UINT8:
            for (quint32 index = 0; index < numElements; ++index)
            {
                dataOut[numBytesPerElement*index] = data[offset + numBytesPerElement*index];
            }
            break;
        case UINT16:
            for (quint32 index = 0; index < numElements; ++index)
            {
                quint16 value;
                memcpy(&value, &data[offset + numBytesPerElement*index], numBytesPerElement);
                qToLittleEndian<quint16>(value, &dataOut[numBytesPerElement*index]);
            }
            break;
        case UINT32:
            for (quint32 index = 0; index < numElements; ++index)
            {
                quint32 value;
                memcpy(&value, &data[offset + numBytesPerElement*index], numBytesPerElement);
                qToLittleEndian<quint32>(value, &dataOut[numBytesPerElement*index]);
            }
            break;
        case FLOAT32:
            for (quint32 index = 0; index < numElements; ++index)
            {
                quint32 value;
                memcpy(&value, &data[offset + numBytesPerElement*index], numBytesPerElement);
                qToLittleEndian<quint32>(value, &dataOut[numBytesPerElement*index]);
            }
            break;
        case ENUM:
            for (quint32 index = 0; index < numElements; ++index)
            {
                dataOut[numBytesPerElement*index] = data[offset + numBytesPerElement*index];
            }
            break;
        case STRING:
            memcpy(dataOut, &data[offset], numElements);
            break;
    }
    // Done
    return getNumBytes();
}

qint32 UAVObjectField::unpack(const quint8* dataIn)
{
    QMutexLocker locker(obj->getMutex());
    // Unpack each element from input buffer
    switch (type)
    {
        case INT8:
            memcpy(&data[offset], dataIn, numElements);
            break;
        case INT16:
            for (quint32 index = 0; index < numElements; ++index)
            {
                qint16 value;
                value = qFromLittleEndian<qint16>(&dataIn[numBytesPerElement*index]);
                memcpy(&data[offset + numBytesPerElement*index], &value, numBytesPerElement);
            }
            break;
        case INT32:
            for (quint32 index = 0; index < numElements; ++index)
            {
                qint32 value;
                value = qFromLittleEndian<qint32>(&dataIn[numBytesPerElement*index]);
                memcpy(&data[offset + numBytesPerElement*index], &value, numBytesPerElement);
            }
            break;
        case UINT8:
            for (quint32 index = 0; index < numElements; ++index)
            {
                data[offset + numBytesPerElement*index] = dataIn[numBytesPerElement*index];
            }
            break;
        case UINT16:
            for (quint32 index = 0; index < numElements; ++index)
            {
                quint16 value;
                value = qFromLittleEndian<quint16>(&dataIn[numBytesPerElement*index]);
                memcpy(&data[offset + numBytesPerElement*index], &value, numBytesPerElement);
            }
            break;
        case UINT32:
            for (quint32 index = 0; index < numElements; ++index)
            {
                quint32 value;
                value = qFromLittleEndian<quint32>(&dataIn[numBytesPerElement*index]);
                memcpy(&data[offset + numBytesPerElement*index], &value, numBytesPerElement);
            }
            break;
        case FLOAT32:
            for (quint32 index = 0; index < numElements; ++index)
            {
                quint32 value;
                value = qFromLittleEndian<quint32>(&dataIn[numBytesPerElement*index]);
                memcpy(&data[offset + numBytesPerElement*index], &value, numBytesPerElement);
            }
            break;
        case ENUM:
            for (quint32 index = 0; index < numElements; ++index)
            {
                data[offset + numBytesPerElement*index] = dataIn[numBytesPerElement*index];
            }
            break;
        case STRING:
            memcpy(&data[offset], dataIn, numElements);
            break;
    }
    // Done
    return getNumBytes();
}

quint32 UAVObjectField::getNumBytesElement()
{
    return numBytesPerElement;
}

bool UAVObjectField::isNumeric()
{
    switch (type)
    {
        case INT8:
            return true;
            break;
        case INT16:
            return true;
            break;
        case INT32:
            return true;
            break;
        case UINT8:
            return true;
            break;
        case UINT16:
            return true;
            break;
        case UINT32:
            return true;
            break;
        case FLOAT32:
            return true;
            break;
        case ENUM:
            return false;
            break;
        case STRING:
            return false;
            break;
        default:
            return false;
    }
}

bool UAVObjectField::isText()
{
    switch (type)
    {
        case INT8:
            return false;
            break;
        case INT16:
            return false;
            break;
        case INT32:
            return false;
            break;
        case UINT8:
            return false;
            break;
        case UINT16:
            return false;
            break;
        case UINT32:
            return false;
            break;
        case FLOAT32:
            return false;
            break;
        case ENUM:
            return true;
            break;
        case STRING:
            return true;
            break;
        default:
            return false;
    }
}

QVariant UAVObjectField::getValue(quint32 index)
{
    QMutexLocker locker(obj->getMutex());
    // Check that index is not out of bounds
    if ( index >= numElements )
    {
        return QVariant();
    }
    // Get value
    switch (type)
    {
        case INT8:
        {
            qint8 tmpint8;
            memcpy(&tmpint8, &data[offset + numBytesPerElement*index], numBytesPerElement);
            return QVariant(tmpint8);
            break;
        }
        case INT16:
        {
            qint16 tmpint16;
            memcpy(&tmpint16, &data[offset + numBytesPerElement*index], numBytesPerElement);
            return QVariant(tmpint16);
            break;
        }
        case INT32:
        {
            qint32 tmpint32;
            memcpy(&tmpint32, &data[offset + numBytesPerElement*index], numBytesPerElement);
            return QVariant(tmpint32);
            break;
        }
        case UINT8:
        {
            quint8 tmpuint8;
            memcpy(&tmpuint8, &data[offset + numBytesPerElement*index], numBytesPerElement);
            return QVariant(tmpuint8);
            break;
        }
        case UINT16:
        {
            quint16 tmpuint16;
            memcpy(&tmpuint16, &data[offset + numBytesPerElement*index], numBytesPerElement);
            return QVariant(tmpuint16);
            break;
        }
        case UINT32:
        {
            quint32 tmpuint32;
            memcpy(&tmpuint32, &data[offset + numBytesPerElement*index], numBytesPerElement);
            return QVariant(tmpuint32);
            break;
        }
        case FLOAT32:
        {
            float tmpfloat;
            memcpy(&tmpfloat, &data[offset + numBytesPerElement*index], numBytesPerElement);
            return QVariant(tmpfloat);
            break;
        }
        case ENUM:
        {
            quint8 tmpenum;
            memcpy(&tmpenum, &data[offset + numBytesPerElement*index], numBytesPerElement);
//            Q_ASSERT((tmpenum < options.length()) && (tmpenum >= 0)); // catch bad enum settings
            if(tmpenum >= options.length()) {
                qDebug() << "Invalid value for" << name;
                return QVariant( QString("Bad Value") );
            }
            return QVariant( options[tmpenum] );
            break;
        }
        case STRING:
        {
            data[offset + numElements - 1] = '\0';
            QString str((char*)&data[offset]);
            return QVariant( str );
            break;
        }
    }
    // If this point is reached then we got an invalid type
    return QVariant();
}

void UAVObjectField::setValue(const QVariant& value, quint32 index)
{
    QMutexLocker locker(obj->getMutex());
    // Check that index is not out of bounds
    if ( index >= numElements )
    {
        return;
    } 
    // Get metadata
    UAVObject::Metadata mdata = obj->getMetadata();
    // Update value if the access mode permits
    if ( mdata.gcsAccess == UAVObject::ACCESS_READWRITE )
    {
        switch (type)
        {
            case INT8:
            {
                qint8 tmpint8 = value.toInt();
                memcpy(&data[offset + numBytesPerElement*index], &tmpint8, numBytesPerElement);
                break;
            }
            case INT16:
            {
                qint16 tmpint16 = value.toInt();
                memcpy(&data[offset + numBytesPerElement*index], &tmpint16, numBytesPerElement);
                break;
            }
            case INT32:
            {
                qint32 tmpint32 = value.toInt();
                memcpy(&data[offset + numBytesPerElement*index], &tmpint32, numBytesPerElement);
                break;
            }
            case UINT8:
            {
                quint8 tmpuint8 = value.toUInt();
                memcpy(&data[offset + numBytesPerElement*index], &tmpuint8, numBytesPerElement);
                break;
            }
            case UINT16:
            {
                quint16 tmpuint16 = value.toUInt();
                memcpy(&data[offset + numBytesPerElement*index], &tmpuint16, numBytesPerElement);
                break;
            }
            case UINT32:
            {
                quint32 tmpuint32 = value.toUInt();
                memcpy(&data[offset + numBytesPerElement*index], &tmpuint32, numBytesPerElement);
                break;
            }
            case FLOAT32:
            {
                float tmpfloat = value.toFloat();
                memcpy(&data[offset + numBytesPerElement*index], &tmpfloat, numBytesPerElement);
                break;
            }
            case ENUM:
            {
                qint8 tmpenum = options.indexOf( value.toString() );
                Q_ASSERT(tmpenum >= 0); // To catch any programming errors where we set invalid values
                memcpy(&data[offset + numBytesPerElement*index], &tmpenum, numBytesPerElement);
                break;
            }
            case STRING:
            {
                QString str = value.toString();
                QByteArray barray = str.toAscii();
                quint32 index;
                for (index = 0; index < (quint32)barray.length() && index < (numElements-1); ++index)
                {
                    data[offset+index] = barray[index];
                }
                barray[index] = '\0';
                break;
            }
        }
    }
}

double UAVObjectField::getDouble(quint32 index)
{
    return getValue(index).toDouble();
}

void UAVObjectField::setDouble(double value, quint32 index)
{
    setValue(QVariant(value), index);
}

