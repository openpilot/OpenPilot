/**
 ******************************************************************************
 *
 * @file       uavobjectfield.h
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
#ifndef UAVOBJECTFIELD_H
#define UAVOBJECTFIELD_H

#include "uavobjects_global.h"
#include "uavobject.h"

class UAVObject;

class UAVOBJECTS_EXPORT UAVObjectField: public QObject
{
    Q_OBJECT

public:
    UAVObjectField(const QString& name, const QString& units, quint32 numElements);
    void initialize(quint8* data, quint32 dataOffset, UAVObject* obj);
    virtual void initializeValues() = 0;
    UAVObject* getObject();
    QString getName();
    QString getUnits();
    quint32 getNumElements();
    virtual qint32 pack(quint8* dataOut) = 0;
    virtual qint32 unpack(const quint8* dataIn) = 0;
    virtual double getDouble(quint32 index = 0) = 0;
    virtual void setDouble(double value, quint32 index = 0) = 0;
    quint32 getDataOffset();
    quint32 getNumBytes();
    virtual quint32 getNumBytesElement() = 0;
    QString toString();

signals:
    void fieldUpdated(UAVObjectField* field);

protected:
    QString name;
    QString units;
    quint32 numElements;
    quint32 offset;
    quint8* data;
    UAVObject* obj;

    void clear();


};

#endif // UAVOBJECTFIELD_H
