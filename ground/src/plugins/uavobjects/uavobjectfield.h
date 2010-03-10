/**
 ******************************************************************************
 *
 * @file       uavobjectfield.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   
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
#ifndef UAVOBJECTFIELD_H
#define UAVOBJECTFIELD_H

#include "uavobject.h"

class UAVObjectField: public QObject
{
    Q_OBJECT

public:
    /**
     * Recognized field types
     */
    typedef enum {
            FIELDTYPE_INT8 = 0,
            FIELDTYPE_INT16,
            FIELDTYPE_INT32,
            FIELDTYPE_FLOAT32,
            FIELDTYPE_CHAR
    } FieldType;

    UAVObjectField(QString& name, QString& units, FieldType type, quint32 numElements);
    void initialize(quint8* data, quint32 dataOffset, UAVObject* obj);
    UAVObject* getObject();
    QString getName();
    QString getUnits();
    FieldType getType();
    quint32 getNumElements();
    double getValue();
    void setValue(double value);
    double getValue(quint32 index);
    void setValue(double value, quint32 index);
    QString getString();
    void setString(QString& str);
    quint32 getDataOffset();
    quint32 getNumBytes();
    quint32 getNumBytesElement();

signals:
    void fieldUpdated(UAVObjectField* field);

private:
    QString name;
    QString units;
    FieldType type;
    quint32 numElements;
    quint32 numBytesPerElement;
    quint32 offset;
    quint8* data;
    UAVObject* obj;

    void clear();


};

#endif // UAVOBJECTFIELD_H
