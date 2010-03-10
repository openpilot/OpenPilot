/**
 ******************************************************************************
 *
 * @file       uavobjectmanager.h
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
#ifndef UAVOBJECTMANAGER_H
#define UAVOBJECTMANAGER_H

#include "uavobject.h"
#include "uavdataobject.h"
#include "uavmetaobject.h"
#include <QList>
#include <QMutex>

class UAVObjectManager: public QObject
{
    Q_OBJECT

public:
    UAVObjectManager();

    bool registerObject(UAVObject* obj);
    UAVDataObject* newObjectInstance(QString& name, quint32 instId = 0);
    UAVDataObject* newObjectInstance(quint32 objId, quint32 instId = 0);
    QList< QList<UAVObject*> > getObjects();
    QList< QList<UAVDataObject*> > getDataObjects();
    QList< QList<UAVMetaObject*> > getMetaObjects();
    UAVObject* getObject(QString& name, quint32 instId = 0);
    UAVObject* getObject(quint32 objId, quint32 instId = 0);
    QList<UAVObject*> getObjectInstances(QString& name);
    QList<UAVObject*> getObjectInstances(quint32 objId);
    qint32 getNumInstances(QString& name);
    qint32 getNumInstances(quint32 objId);

signals:
    void newObject(UAVObject* obj);

private:
    QList< QList<UAVObject*> > objects;
    QMutex* mutex;

    UAVDataObject* newObjectInstance(QString* name, quint32 objId, quint32 instId);
    UAVObject* getObject(QString* name, quint32 objId, quint32 instId);
    QList<UAVObject*> getObjectInstances(QString* name, quint32 objId);
    qint32 getNumInstances(QString* name, quint32 objId);
};


#endif // UAVOBJECTMANAGER_H
