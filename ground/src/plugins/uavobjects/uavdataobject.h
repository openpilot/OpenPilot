/**
 ******************************************************************************
 *
 * @file       uavdataobject.h
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
#ifndef UAVDATAOBJECT_H
#define UAVDATAOBJECT_H

#include "uavobject.h"
#include "uavobjectfield.h"
#include "uavmetaobject.h"
#include <QList>

class UAVDataObject: public UAVObject
{
    Q_OBJECT

public:
    UAVDataObject(quint32 objID, quint32 instID, bool isSingleInst, QString& name, quint32 numBytes);
    void initialize(QList<UAVObjectField*> fields, Metadata& mdata);
    void initialize(QList<UAVObjectField*> fields, UAVMetaObject* mobj);

    qint32 getNumFields();
    QList<UAVObjectField*> getFields();
    UAVObjectField* getField(QString& name);
    qint32 pack(quint8* dataOut);
    qint32 unpack(const quint8* dataIn);
    void setMetadata(const Metadata& mdata);
    Metadata getMetadata();
    UAVMetaObject* getMetaObject();

private slots:
    void fieldUpdated(UAVObjectField* field);

private:
    quint8* data;
    QList<UAVObjectField*> fields;
    UAVMetaObject* mobj;

    void initialize(QList<UAVObjectField*> fields);


};

#endif // UAVDATAOBJECT_H
