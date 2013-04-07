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

UAVObjectField::UAVObjectField(const QString& name, const QString& units, FieldType type, quint32 numElements, const QStringList& options, const QString &limits)
{
    QStringList elementNames;
    // Set element names
    for (quint32 n = 0; n < numElements; ++n)
    {
        elementNames.append(QString("%1").arg(n));
    }
    // Initialize
    constructorInitialize(name, units, type, elementNames, options,limits);

}

UAVObjectField::UAVObjectField(const QString& name, const QString& units, FieldType type, const QStringList& elementNames, const QStringList& options, const QString &limits)
{
    constructorInitialize(name, units, type, elementNames, options,limits);
}

void UAVObjectField::constructorInitialize(const QString& name, const QString& units, FieldType type, const QStringList& elementNames, const QStringList& options,const QString &limits)
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
    case BITFIELD:
        numBytesPerElement = sizeof(quint8);
        this->options = QStringList()<<tr("0")<<tr("1");
        break;
    case STRING:
        numBytesPerElement = sizeof(quint8);
        break;
    default:
        numBytesPerElement = 0;
    }
    limitsInitialize(limits);
}

void UAVObjectField::limitsInitialize(const QString &limits)
{
    /// format
    /// (TY)->type (EQ-equal;NE-not equal;BE-between;BI-bigger;SM-smaller)
    /// (VALX)->value
    /// %TY:VAL1:VAL2:VAL3,%TY,VAL1,VAL2,VAL3
    /// example: first element bigger than 3 and second element inside [2.3,5]
    /// "%BI:3,%BE:2.3:5"
    if(limits.isEmpty())
        return;
    QStringList stringPerElement=limits.split(",");
    quint32 index=0;
    foreach (QString str, stringPerElement) {
        QStringList ruleList=str.split(";");
        QList<LimitStruct> limitList;
        foreach(QString rule,ruleList)
        {
            QString _str=rule.trimmed();
            if(_str.isEmpty())
                continue;
            QStringList valuesPerElement=_str.split(":");
            LimitStruct lstruc;
            bool startFlag=valuesPerElement.at(0).startsWith("%");
            bool maxIndexFlag=(int)(index)<(int)numElements;
            bool elemNumberSizeFlag=valuesPerElement.at(0).size()==3;
            bool aux;
            valuesPerElement.at(0).mid(1,4).toInt(&aux,16);
            bool b4=((valuesPerElement.at(0).size())==7 && aux);
            if(startFlag && maxIndexFlag && (elemNumberSizeFlag || b4))
            {
                if(b4)
                    lstruc.board=valuesPerElement.at(0).mid(1,4).toInt(&aux,16);
                else
                    lstruc.board=0;
                if(valuesPerElement.at(0).right(2)=="EQ")
                    lstruc.type=EQUAL;
                else if(valuesPerElement.at(0).right(2)=="NE")
                    lstruc.type=NOT_EQUAL;
                else if(valuesPerElement.at(0).right(2)=="BE")
                    lstruc.type=BETWEEN;
                else if(valuesPerElement.at(0).right(2)=="BI")
                    lstruc.type=BIGGER;
                else if(valuesPerElement.at(0).right(2)=="SM")
                    lstruc.type=SMALLER;
                else
                    qDebug()<<"limits parsing failed (invalid property) on UAVObjectField"<<name;
                valuesPerElement.removeAt(0);
                foreach(QString _value,valuesPerElement)
                {
                    QString value=_value.trimmed();
                    switch (type)
                    {
                    case UINT8:
                    case UINT16:
                    case UINT32:
                    case BITFIELD:
                        lstruc.values.append((quint32)value.toULong());
                        break;
                    case INT8:
                    case INT16:
                    case INT32:
                        lstruc.values.append((qint32)value.toLong());
                        break;
                    case FLOAT32:
                        lstruc.values.append((float)value.toFloat());
                        break;
                    case ENUM:
                        lstruc.values.append((QString)value);
                        break;
                    case STRING:
                        lstruc.values.append((QString)value);
                        break;
                    default:
                        lstruc.values.append(QVariant());
                    }
                }
                limitList.append(lstruc);
            }
            else
            {
                if(!valuesPerElement.at(0).isEmpty() && !startFlag)
                    qDebug()<<"limits parsing failed (property doesn't start with %) on UAVObjectField"<<name;
                else if(!maxIndexFlag)
                    qDebug()<<"limits parsing failed (index>numelements) on UAVObjectField"<<name<<"index"<<index<<"numElements"<<numElements;
                else if(!elemNumberSizeFlag || !b4 )
                    qDebug()<<"limits parsing failed limit not starting with %XX or %YYYYXX where XX is the limit type and YYYY is the board type on UAVObjectField"<<name;
            }
        }
        elementLimits.insert(index,limitList);
        ++index;

    }
    foreach(QList<LimitStruct> limitList,elementLimits)
    {
        foreach(LimitStruct limit,limitList)
        {
            qDebug()<<"Limit type"<<limit.type<<"for board"<<limit.board<<"for field"<<getName();
            foreach(QVariant var,limit.values)
            {
                qDebug()<<"value"<<var;
            }
        }
    }
}
bool UAVObjectField::isWithinLimits(QVariant var,quint32 index, int board)
{
    if(!elementLimits.keys().contains(index))
        return true;

    foreach(LimitStruct struc,elementLimits.value(index))
    {
        if((struc.board!=board) && board!=0 && struc.board!=0)
            continue;
        switch(struc.type)
        {
        case EQUAL:
            switch (type)
            {
            case INT8:
            case INT16:
            case INT32:
                foreach (QVariant vars, struc.values) {
                    if(var.toInt()==vars.toInt())
                        return true;
                }
                return false;
                break;
            case UINT8:
            case UINT16:
            case UINT32:
            case BITFIELD:
                foreach (QVariant vars, struc.values) {
                    if(var.toUInt()==vars.toUInt())
                        return true;
                }
                return false;
                break;
            case ENUM:
            case STRING:
                foreach (QVariant vars, struc.values) {
                    if(var.toString()==vars.toString())
                        return true;
                }
                return false;
                break;
            case FLOAT32:
                foreach (QVariant vars, struc.values) {
                    if(var.toFloat()==vars.toFloat())
                        return true;
                }
                return false;
                break;
            default:
                return true;
            }
            break;
        case NOT_EQUAL:
            switch (type)
            {
            case INT8:
            case INT16:
            case INT32:
                foreach (QVariant vars, struc.values) {
                    if(var.toInt()==vars.toInt())
                        return false;
                }
                return true;
                break;
            case UINT8:
            case UINT16:
            case UINT32:
            case BITFIELD:
                foreach (QVariant vars, struc.values) {
                    if(var.toUInt()==vars.toUInt())
                        return false;
                }
                return true;
                break;
            case ENUM:
            case STRING:
                foreach (QVariant vars, struc.values) {
                    if(var.toString()==vars.toString())
                        return false;
                }
                return true;
                break;
            case FLOAT32:
                foreach (QVariant vars, struc.values) {
                    if(var.toFloat()==vars.toFloat())
                        return false;
                }
                return true;
                break;
            default:
                return true;
            }
            break;
        case BETWEEN:
            if(struc.values.length()<2)
            {
                qDebug()<<__FUNCTION__<<"between limit with less than 1 pair, aborting; field:"<<name;
                return true;
            }
            if(struc.values.length()>2)
                qDebug()<<__FUNCTION__<<"between limit with more than 1 pair, using first; field"<<name;
            switch (type)
            {
            case INT8:
            case INT16:
            case INT32:
                    if(!(var.toInt()>=struc.values.at(0).toInt() && var.toInt()<=struc.values.at(1).toInt()))
                        return false;
                return true;
                break;
            case UINT8:
            case UINT16:
            case UINT32:
            case BITFIELD:
                    if(!(var.toUInt()>=struc.values.at(0).toUInt() && var.toUInt()<=struc.values.at(1).toUInt()))
                        return false;
                return true;
                break;
            case ENUM:
                    if(!(options.indexOf(var.toString())>=options.indexOf(struc.values.at(0).toString()) && options.indexOf(var.toString())<=options.indexOf(struc.values.at(1).toString())))
                        return false;
                return true;
                break;
            case STRING:
                return true;
                break;
            case FLOAT32:
                    if(!(var.toFloat()>=struc.values.at(0).toFloat() && var.toFloat()<=struc.values.at(1).toFloat()))
                        return false;
                return true;
                break;
            default:
                return true;
            }
            break;
        case BIGGER:
            if(struc.values.length()<1)
            {
                qDebug()<<__FUNCTION__<<"BIGGER limit with less than 1 value, aborting; field:"<<name;
                return true;
            }
            if(struc.values.length()>1)
                qDebug()<<__FUNCTION__<<"BIGGER limit with more than 1 value, using first; field"<<name;
            switch (type)
            {
            case INT8:
            case INT16:
            case INT32:
                    if(!(var.toInt()>=struc.values.at(0).toInt()))
                        return false;
                return true;
                break;
            case UINT8:
            case UINT16:
            case UINT32:
            case BITFIELD:
                    if(!(var.toUInt()>=struc.values.at(0).toUInt()))
                        return false;
                return true;
                break;
            case ENUM:
                    if(!(options.indexOf(var.toString())>=options.indexOf(struc.values.at(0).toString())))
                        return false;
                return true;
                break;
            case STRING:
                return true;
                break;
            case FLOAT32:
                    if(!(var.toFloat()>=struc.values.at(0).toFloat()))
                        return false;
                return true;
                break;
            default:
                return true;
            }
            break;
        case SMALLER:
            switch (type)
            {
            case INT8:
            case INT16:
            case INT32:
                    if(!(var.toInt()<=struc.values.at(0).toInt()))
                        return false;
                return true;
                break;
            case UINT8:
            case UINT16:
            case UINT32:
            case BITFIELD:
                    if(!(var.toUInt()<=struc.values.at(0).toUInt()))
                        return false;
                return true;
                break;
            case ENUM:
                    if(!(options.indexOf(var.toString())<=options.indexOf(struc.values.at(0).toString())))
                        return false;
                return true;
                break;
            case STRING:
                return true;
                break;
            case FLOAT32:
                    if(!(var.toFloat()<=struc.values.at(0).toFloat()))
                        return false;
                return true;
                break;
            default:
                return true;
            }
        }
    }
    return true;
}

QVariant UAVObjectField::getMaxLimit(quint32 index,int board)
{
    if(!elementLimits.keys().contains(index))
        return QVariant();
    foreach(LimitStruct struc,elementLimits.value(index))
    {
        if((struc.board!=board) && board!=0 && struc.board!=0)
            continue;
        switch(struc.type)
        {
        case EQUAL:
        case NOT_EQUAL:
        case BIGGER:
            return QVariant();
            break;
            break;
        case BETWEEN:
            return struc.values.at(1);
            break;
        case SMALLER:
            return struc.values.at(0);
            break;
        default:
            return QVariant();
            break;
        }
    }
    return QVariant();
}
QVariant UAVObjectField::getMinLimit(quint32 index, int board)
{
    if(!elementLimits.keys().contains(index))
        return QVariant();
    foreach(LimitStruct struc,elementLimits.value(index))
    {
        if((struc.board!=board) && board!=0 && struc.board!=0)
            return QVariant();
        switch(struc.type)
        {
        case EQUAL:
        case NOT_EQUAL:
        case SMALLER:
            return QVariant();
            break;
            break;
        case BETWEEN:
            return struc.values.at(0);
            break;
        case BIGGER:
            return struc.values.at(0);
            break;
        default:
            return QVariant();
            break;
        }
    }
    return QVariant();
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
    case UAVObjectField::BITFIELD:
        return "bitfield";
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
    switch (type)
    {
    case BITFIELD:
        memset(&data[offset], 0, numBytesPerElement*((quint32)(1+(numElements-1)/8)));
        break;
    default:
        memset(&data[offset], 0, numBytesPerElement*numElements);
        break;
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
    switch (type)
    {
    case BITFIELD:
        return numBytesPerElement * ((quint32) (1+(numElements-1)/8));
        break;
    default:
        return numBytesPerElement * numElements;
        break;
    }
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
    case BITFIELD:
        for (quint32 index = 0; index < (quint32)(1+(numElements-1)/8); ++index)
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
    case BITFIELD:
        for (quint32 index = 0; index < (quint32)(1+(numElements-1)/8); ++index)
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
    case BITFIELD:
        return true;
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
    case BITFIELD:
        return false;
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
    case BITFIELD:
    {
        quint8 tmpbitfield;
        memcpy(&tmpbitfield, &data[offset + numBytesPerElement*((quint32)(index/8))], numBytesPerElement);
        tmpbitfield = (tmpbitfield >> (index % 8)) & 1;
        return QVariant( tmpbitfield );
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

bool UAVObjectField::checkValue(const QVariant& value, quint32 index)
{
    QMutexLocker locker(obj->getMutex());
    // Check that index is not out of bounds
    if ( index >= numElements )
    {
        return false;
    }
    // Get metadata
    UAVObject::Metadata mdata = obj->getMetadata();
    // Update value if the access mode permits
    if ( UAVObject::GetFlightAccess(mdata) == UAVObject::ACCESS_READWRITE )
    {
        switch (type)
        {
        case INT8:
        case INT16:
        case INT32:
        case UINT8:
        case UINT16:
        case UINT32:
        case FLOAT32:
        case STRING:
        case BITFIELD:
            return true;
            break;
        case ENUM:
        {
            qint8 tmpenum = options.indexOf( value.toString() );
            return ((tmpenum < 0) ? false : true);
            break;
        }
        default:
            qDebug() << "checkValue: other types" << type;
            Q_ASSERT(0); // To catch any programming errors where we tried to test invalid values
            break;
        }
    }
    return true;
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
    if ( UAVObject::GetGcsAccess(mdata) == UAVObject::ACCESS_READWRITE )
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
        case BITFIELD:
        {
            quint8 tmpbitfield;
            memcpy(&tmpbitfield, &data[offset + numBytesPerElement*((quint32)(index/8))], numBytesPerElement);
            tmpbitfield = (tmpbitfield & ~(1 << (index % 8))) | ( (value.toUInt()!=0?1:0) << (index % 8) );
            memcpy(&data[offset + numBytesPerElement*((quint32)(index/8))], &tmpbitfield, numBytesPerElement);
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

