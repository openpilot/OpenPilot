/**
 ******************************************************************************
 *
 * @file       uavobjectfield.h
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
#ifndef UAVOBJECTFIELD_H
#define UAVOBJECTFIELD_H

#include "uavobjects_global.h"
#include "uavobject.h"
#include <QStringList>
#include <QVariant>
#include <QList>
#include <QMap>

class UAVObject;

class UAVOBJECTS_EXPORT UAVObjectField: public QObject
{
    Q_OBJECT

public:
    typedef enum { INT8 = 0, INT16, INT32, UINT8, UINT16, UINT32, FLOAT32, ENUM, BITFIELD, STRING } FieldType;
    typedef enum { EQUAL,NOT_EQUAL,BETWEEN,BIGGER,SMALLER } LimitType;
    typedef struct
    {
        LimitType type;
        QList<QVariant> values;
        int board;
    } LimitStruct;

    UAVObjectField(const QString& name, const QString& units, FieldType type, quint32 numElements, const QStringList& options,const QString& limits=QString());
    UAVObjectField(const QString& name, const QString& units, FieldType type, const QStringList& elementNames, const QStringList& options,const QString& limits=QString());
    void initialize(quint8* data, quint32 dataOffset, UAVObject* obj);
    UAVObject* getObject();
    FieldType getType();
    QString getTypeAsString();
    QString getName();
    QString getUnits();
    quint32 getNumElements();
    QStringList getElementNames();
    QStringList getOptions();
    qint32 pack(quint8* dataOut);
    qint32 unpack(const quint8* dataIn);
    QVariant getValue(quint32 index = 0);
    bool checkValue(const QVariant& data, quint32 index = 0);
    void setValue(const QVariant& data, quint32 index = 0);
    double getDouble(quint32 index = 0);
    void setDouble(double value, quint32 index = 0);
    quint32 getDataOffset();
    quint32 getNumBytes();
    bool isNumeric();
    bool isText();
    QString toString();

    bool isWithinLimits(QVariant var, quint32 index, int board=0);
    QVariant getMaxLimit(quint32 index, int board=0);
    QVariant getMinLimit(quint32 index, int board=0);
signals:
    void fieldUpdated(UAVObjectField* field);

protected:
    QString name;
    QString units;
    FieldType type;
    QStringList elementNames;
    QStringList options;
    quint32 numElements;
    quint32 numBytesPerElement;
    quint32 offset;
    quint8* data;
    UAVObject* obj;
    QMap<quint32, QList<LimitStruct> > elementLimits;
    void clear();
    void constructorInitialize(const QString& name, const QString& units, FieldType type, const QStringList& elementNames, const QStringList& options, const QString &limits);
    void limitsInitialize(const QString &limits);


};

#endif // UAVOBJECTFIELD_H
