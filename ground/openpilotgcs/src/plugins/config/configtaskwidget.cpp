/**
 ******************************************************************************
 *
 * @file       configtaskwidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#include "configtaskwidget.h"

#include <QtGui/QWidget>


ConfigTaskWidget::ConfigTaskWidget(QWidget *parent) : QWidget(parent)
{

}

ConfigTaskWidget::~ConfigTaskWidget()
{
   // Do nothing
}

void ConfigTaskWidget::saveObjectToSD(UAVObject *obj)
{
    // Add to queue
    queue.enqueue(obj);
    // If queue length is one, then start sending (call sendNextObject)
    // Otherwise, do nothing, it's sending anyway
    if (queue.length()==1)
        saveNextObject();

}

void ConfigTaskWidget::saveNextObject()
{
    if ( queue.isEmpty() )
    {
        return;
    }
    // Get next object from the queue
    UAVObject* obj = queue.head();
    ObjectPersistence* objper = dynamic_cast<ObjectPersistence*>( getObjectManager()->getObject(ObjectPersistence::NAME) );
    connect(objper, SIGNAL(transactionCompleted(UAVObject*,bool)), this, SLOT(transactionCompleted(UAVObject*,bool)));
    if (obj != NULL)
    {
        ObjectPersistence::DataFields data;
        data.Operation = ObjectPersistence::OPERATION_SAVE;
        data.Selection = ObjectPersistence::SELECTION_SINGLEOBJECT;
        data.ObjectID = obj->getObjID();
        data.InstanceID = obj->getInstID();
        objper->setData(data);
        objper->updated();
    }
}

void ConfigTaskWidget::transactionCompleted(UAVObject* obj, bool success)
{
    if(success &&
       obj->getField("Operation")->getValue().toString().compare(QString("Completed")) == 0 ) {
        // Disconnect from sending object
        obj->disconnect(this);
        queue.dequeue(); // We can now remove the object, it's done.
        saveNextObject();
    }
}

void ConfigTaskWidget::updateObjectPersistance(ObjectPersistence::OperationOptions op, UAVObject *obj)
{
    ObjectPersistence* objper = dynamic_cast<ObjectPersistence*>( getObjectManager()->getObject(ObjectPersistence::NAME) );
    if (obj != NULL)
    {
        ObjectPersistence::DataFields data;
        data.Operation = op;
        data.Selection = ObjectPersistence::SELECTION_SINGLEOBJECT;
        data.ObjectID = obj->getObjID();
        data.InstanceID = obj->getInstID();
        objper->setData(data);
        objper->updated();
    }
}

UAVObjectManager* ConfigTaskWidget::getObjectManager() {
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr);
    return objMngr;
}

double ConfigTaskWidget::listMean(QList<double> list)
{
    double accum = 0;
    for(int i = 0; i < list.size(); i++)
        accum += list[i];
    return accum / list.size();
}


/**
  @}
  @}
  */
