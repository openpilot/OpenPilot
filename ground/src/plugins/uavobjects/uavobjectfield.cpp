#include "uavobjectfield.h"
#include <QtEndian>

UAVObjectField::UAVObjectField(QString& name, QString& units, FieldType type, quint32 numElements)
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
            value16 = (qint16)qFromBigEndian<qint16>(&data[offset + numBytesPerElement*index]);
            ret = (double)value16;
            break;
        case FIELDTYPE_INT32:
            qint32 value32;
            value32 = (qint32)qFromBigEndian<qint32>(&data[offset + numBytesPerElement*index]);
            ret = (double)value32;
            break;
        case FIELDTYPE_FLOAT32:
            qint32 tmp;
            float valuef;
            tmp = (qint32)qFromBigEndian<qint32>(&data[offset + numBytesPerElement*index]);
            memcpy(&valuef, &tmp, 4);
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
            qToBigEndian<qint16>((qint16)value, &data[offset + numBytesPerElement*index]);
            break;
        case FIELDTYPE_INT32:
            qToBigEndian<qint32>((qint32)value, &data[offset + numBytesPerElement*index]);
            break;
        case FIELDTYPE_FLOAT32:
            qint32 tmp;
            memcpy(&tmp, &value, 4);
            qToBigEndian<qint32>((qint32)tmp, &data[offset + numBytesPerElement*index]);
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


