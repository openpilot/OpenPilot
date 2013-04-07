/**
 ******************************************************************************
 *
 * @file       uavobjectmanager.h
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
#ifndef UAVOBJECTMANAGER_H
#define UAVOBJECTMANAGER_H

#include "uavobjects_global.h"
#include "uavobject.h"
#include "uavdataobject.h"
#include "uavmetaobject.h"
#include <QList>
#include <QMutex>
#include <QMutexLocker>

class UAVOBJECTS_EXPORT UAVObjectManager: public QObject
{
    Q_OBJECT

public:
    UAVObjectManager();
    ~UAVObjectManager();

    bool registerObject(UAVDataObject* obj);
    QList< QList<UAVObject*> > getObjects();
    QList< QList<UAVDataObject*> > getDataObjects();
    QList< QList<UAVMetaObject*> > getMetaObjects();
    UAVObject* getObject(const QString& name, quint32 instId = 0);
    UAVObject* getObject(quint32 objId, quint32 instId = 0);
    QList<UAVObject*> getObjectInstances(const QString& name);
    QList<UAVObject*> getObjectInstances(quint32 objId);
    qint32 getNumInstances(const QString& name);
    qint32 getNumInstances(quint32 objId);

signals:
    void newObject(UAVObject* obj);
    void newInstance(UAVObject* obj);

private:
    static const quint32 MAX_INSTANCES = 1000;

    QList< QList<UAVObject*> > objects;
    QMutex* mutex;

    void addObject(UAVObject* obj);
    UAVObject* getObject(const QString* name, quint32 objId, quint32 instId);
    QList<UAVObject*> getObjectInstances(const QString* name, quint32 objId);
    qint32 getNumInstances(const QString* name, quint32 objId);
};


#endif // UAVOBJECTMANAGER_H
