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
    saveState = IDLE;
    queue.clear();
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

    Q_ASSERT(saveState == IDLE);

    // Get next object from the queue
    UAVObject* obj = queue.head();
    ObjectPersistence* objper = dynamic_cast<ObjectPersistence*>( getObjectManager()->getObject(ObjectPersistence::NAME) );
    connect(objper, SIGNAL(transactionCompleted(UAVObject*,bool)), this, SLOT(objectPersistenceTransactionCompleted(UAVObject*,bool)));
    connect(objper, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(objectPersistenceUpdated(UAVObject *)));
    saveState = AWAITING_ACK;
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

/**
  * @brief Process the transactionCompleted message from Telemetry indicating request sent successfully
  * @param[in] The object just transsacted.  Must be ObjectPersistance
  * @param[in] success Indicates that the transaction did not time out
  *
  * After a failed transaction (usually timeout) resends the save request.  After a succesful
  * transaction will then wait for a save completed update from the autopilot.
  */
void ConfigTaskWidget::objectPersistenceTransactionCompleted(UAVObject* obj, bool success)
{
    if(success) {
        Q_ASSERT(obj->getName().compare("ObjectPersistence") == 0);
        Q_ASSERT(saveState == AWAITING_ACK);
        saveState = AWAITING_COMPLETED;
        disconnect(obj, SIGNAL(transactionCompleted(UAVObject*,bool)), this, SLOT(objectPersistenceTransactionCompleted(UAVObject*,bool)));
    } else if (!success) {
        // Can be caused by timeout errors on sending.  Send again.
        saveNextObject();
    }
}

/**
  * @brief Process the ObjectPersistence updated message to confirm the right object saved
  * then requests next object be saved.
  * @param[in] The object just received.  Must be ObjectPersistance
  */
void ConfigTaskWidget::objectPersistenceUpdated(UAVObject * obj)
{
    Q_ASSERT(obj->getName().compare("ObjectPersistence") == 0);
    if(saveState == AWAITING_COMPLETED) {
        // Check flight is saying it completed.  This is the only thing flight should do to trigger an update.
        Q_ASSERT( obj->getField("Operation")->getValue().toString().compare(QString("Completed")) == 0 );

        // Check right object saved
        UAVObject* savingObj = queue.head();
        Q_ASSERT( obj->getField("ObjectID")->getValue() == savingObj->getObjID() );

        obj->disconnect(this);
        queue.dequeue(); // We can now remove the object, it's done.
        saveState = IDLE;
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
