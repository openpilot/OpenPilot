/**
 ******************************************************************************
 *
 * @file       modeluavproxy.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin OpenPilot Map Plugin
 * @{
 * @brief The OpenPilot Map plugin
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
#include "modeluavoproxy.h"
#include "extensionsystem/pluginmanager.h"
#include <math.h>

ModelUavoProxy::ModelUavoProxy(QObject *parent, flightDataModel *model) : QObject(parent), myModel(model)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();

    Q_ASSERT(pm != NULL);
    objManager    = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager != NULL);
    waypointObj   = Waypoint::GetInstance(objManager);
    Q_ASSERT(waypointObj != NULL);
    pathactionObj = PathAction::GetInstance(objManager);
    Q_ASSERT(pathactionObj != NULL);
}

void ModelUavoProxy::modelToObjects()
{
    qDebug() << "ModelUAVProxy::modelToObjects";
    // update waypoint and path actions UAV objects
    // waypoints are unique and each waypoint has an entry in the UAV waypoint list
    // a path action can be referenced by multiple waypoints
    // waypoints reference path action by their index in the UAV path action list
    // the factoring of path actions (to remove duplicates) happens here...

    // the UAV waypoint list and path action list are probably not empty
    // so we try to reuse existing instances

    // track number of path actions
    int actionCount = 0;

    // iterate over waypoints
    for (int i = 0; i < myModel->rowCount(); ++i) {

        // Path Actions

        // create action to use as a search criteria
        // this object does not need to be deleted as it will either be added to the managed list or deleted later
        PathAction *action = new PathAction;

        // get model data
        PathAction::DataFields actionData = action->getData();
        modelToPathAction(i, actionData);

        // see if that path action has already been added in this run
        PathAction *foundAction = findPathAction(actionData, actionCount);
        // TODO this test needs a consistency check as it is unsafe.
        // if the find method is buggy and returns false positives then the flight plan sent to the uav is broken!
        // the find method should do a "binary" compare instead of a field by field compare
        // if a field is added everywhere and not in the compare method then you can start having false positives
        if (!foundAction) {
            // create or reuse an action instance
            action = createPathAction(actionCount, action);
            actionCount++;

            // send action update to UAV
            action->setData(actionData);
            action->updated();
            qDebug() << "ModelUAVProxy::modelToObjects - sent action instance :" << action->getInstID();
        }
        else {
            action->deleteLater();
            action = foundAction;
            qDebug() << "ModelUAVProxy::modelToObjects - found action instance :" << action->getInstID();
        }

        // Waypoints

        // create or reuse a waypoint instance
        Waypoint *waypoint = createWaypoint(i, NULL);
        Q_ASSERT(waypoint);

        // get model data
        Waypoint::DataFields waypointData = waypoint->getData();
        modelToWaypoint(i, waypointData);

        // connect waypoint to path action
        waypointData.Action = action->getInstID();

        // send waypoint update to UAV
        waypoint->setData(waypointData);
        waypoint->updated();
        qDebug() << "ModelUAVProxy::modelToObjects - sent waypoint instance :" << waypoint->getInstID();
    }
}

Waypoint *ModelUavoProxy::createWaypoint(int index, Waypoint *newWaypoint) {
    Waypoint *waypoint = NULL;
    int count = objManager->getNumInstances(waypointObj->getObjID());
    if (index < count) {
        // reuse object
        qDebug() << "ModelUAVProxy::createWaypoint - reused waypoint instance :" << index << "/" << count;
        waypoint = Waypoint::GetInstance(objManager, index);
        if (newWaypoint) {
            newWaypoint->deleteLater();
        }
    } else if (index < count + 1) {
        // create "next" object
        qDebug() << "ModelUAVProxy::createWaypoint - created waypoint instance :" << index;
        // TODO is there a limit to the number of wp?
        waypoint = newWaypoint ? newWaypoint : new Waypoint;
        waypoint->initialize(index, waypoint->getMetaObject());
        objManager->registerObject(waypoint);
    }
    else {
        // we can only create the "next" object
        // TODO fail in a clean way :(
    }
    return waypoint;
}

PathAction *ModelUavoProxy::createPathAction(int index, PathAction *newAction) {
    PathAction *action = NULL;
    int count = objManager->getNumInstances(pathactionObj->getObjID());
    if (index < count) {
        // reuse object
        qDebug() << "ModelUAVProxy::createPathAction - reused action instance :" << index << "/" << count;
        action = PathAction::GetInstance(objManager, index);
        if (newAction) {
            newAction->deleteLater();
        }
    } else if (index < count + 1) {
        // create "next" object
        qDebug() << "ModelUAVProxy::createPathAction - created action instance :" << index;
        // TODO is there a limit to the number of path actions?
        action = newAction ? newAction : new PathAction;
        action->initialize(index, action->getMetaObject());
        objManager->registerObject(action);
    }
    else {
        // we can only create the "next" object
        // TODO fail in a clean way :(
    }
    return action;
}

PathAction *ModelUavoProxy::findPathAction(const PathAction::DataFields &actionData, int actionCount) {
    int instancesCount = objManager->getNumInstances(pathactionObj->getObjID());
    int count = actionCount <= instancesCount ? actionCount : instancesCount;
    for (int i = 0; i < count; ++i) {
        PathAction *action = PathAction::GetInstance(objManager, i);
        Q_ASSERT(action);
        if (!action) {
            continue;
        }
        PathAction::DataFields fields = action->getData();
        if (fields.Command == actionData.Command
                && fields.ConditionParameters[0] == actionData.ConditionParameters[0]
                && fields.ConditionParameters[1] == actionData.ConditionParameters[1]
                && fields.ConditionParameters[2] == actionData.ConditionParameters[2]
                && fields.EndCondition == actionData.EndCondition
                && fields.ErrorDestination == actionData.ErrorDestination
                && fields.JumpDestination == actionData.JumpDestination && fields.Mode == actionData.Mode
                && fields.ModeParameters[0] == actionData.ModeParameters[0]
                && fields.ModeParameters[1] == actionData.ModeParameters[1]
                && fields.ModeParameters[2] == actionData.ModeParameters[2]) {
            return action;
        }
    }
    return NULL;
}

void ModelUavoProxy::objectsToModel()
{
    myModel->removeRows(0, myModel->rowCount());

    int instanceCount = objManager->getNumInstances(waypointObj->getObjID());
    for (int i = 0; i < instanceCount; ++i) {
        Waypoint *waypoint = Waypoint::GetInstance(objManager, i);
        Q_ASSERT(waypoint);
        if (!waypoint) {
            continue;
        }
        myModel->insertRow(i);

        Waypoint::DataFields waypointData = waypoint->getData();
        waypointToModel(i, waypointData);

        PathAction *action = PathAction::GetInstance(objManager, waypointData.Action);
        Q_ASSERT(action);
        if (!action) {
            continue;
        }

        PathAction::DataFields actionData = action->getData();
        pathActionToModel(i, actionData);
    }
}

void ModelUavoProxy::modelToWaypoint(int i, Waypoint::DataFields &data) {
    double distance, bearing, altitude, velocity;

    QModelIndex index = myModel->index(i, flightDataModel::DISRELATIVE);
    distance = myModel->data(index).toDouble();
    index    = myModel->index(i, flightDataModel::BEARELATIVE);
    bearing  = myModel->data(index).toDouble();
    index    = myModel->index(i, flightDataModel::ALTITUDERELATIVE);
    altitude = myModel->data(index).toFloat();
    index    = myModel->index(i, flightDataModel::VELOCITY);
    velocity = myModel->data(index).toFloat();

    data.Position[Waypoint::POSITION_NORTH] = distance * cos(bearing / (180 * M_PI));
    data.Position[Waypoint::POSITION_EAST]  = distance * sin(bearing / (180 * M_PI));
    data.Position[Waypoint::POSITION_DOWN]  = -altitude;
    data.Velocity = velocity;
}

void ModelUavoProxy::waypointToModel(int i, Waypoint::DataFields &data) {

    double distance = sqrt(data.Position[Waypoint::POSITION_NORTH] * data.Position[Waypoint::POSITION_NORTH] +
                    data.Position[Waypoint::POSITION_EAST] * data.Position[Waypoint::POSITION_EAST]);

    double bearing  = atan2(data.Position[Waypoint::POSITION_EAST], data.Position[Waypoint::POSITION_NORTH]) * (180 / M_PI);
    if (bearing != bearing) {
        bearing = 0;
    }

    double altitude = -data.Position[Waypoint::POSITION_DOWN];

    QModelIndex index = myModel->index(i, flightDataModel::VELOCITY);
    myModel->setData(index, data.Velocity);
    index  = myModel->index(i, flightDataModel::DISRELATIVE);
    myModel->setData(index, distance);
    index  = myModel->index(i, flightDataModel::BEARELATIVE);
    myModel->setData(index, bearing);
    index  = myModel->index(i, flightDataModel::ALTITUDERELATIVE);
    myModel->setData(index, altitude);
}

void ModelUavoProxy::modelToPathAction(int i, PathAction::DataFields &data) {
    QModelIndex index = myModel->index(i, flightDataModel::MODE);
    data.Mode = myModel->data(index).toInt();
    index = myModel->index(i, flightDataModel::MODE_PARAMS0);
    data.ModeParameters[0] = myModel->data(index).toFloat();
    index = myModel->index(i, flightDataModel::MODE_PARAMS1);
    data.ModeParameters[1] = myModel->data(index).toFloat();
    index = myModel->index(i, flightDataModel::MODE_PARAMS2);
    data.ModeParameters[2] = myModel->data(index).toFloat();
    index = myModel->index(i, flightDataModel::MODE_PARAMS3);
    data.ModeParameters[3] = myModel->data(index).toFloat();
    index = myModel->index(i, flightDataModel::CONDITION);
    data.EndCondition = myModel->data(index).toInt();
    index = myModel->index(i, flightDataModel::CONDITION_PARAMS0);
    data.ConditionParameters[0] = myModel->data(index).toFloat();
    index = myModel->index(i, flightDataModel::CONDITION_PARAMS1);
    data.ConditionParameters[1] = myModel->data(index).toFloat();
    index = myModel->index(i, flightDataModel::CONDITION_PARAMS2);
    data.ConditionParameters[2] = myModel->data(index).toFloat();
    index = myModel->index(i, flightDataModel::CONDITION_PARAMS3);
    data.ConditionParameters[3] = myModel->data(index).toFloat();
    index = myModel->index(i, flightDataModel::COMMAND);
    data.Command = myModel->data(index).toInt();
    index = myModel->index(i, flightDataModel::JUMPDESTINATION);
    data.JumpDestination = myModel->data(index).toInt() - 1;
    index = myModel->index(i, flightDataModel::ERRORDESTINATION);
    data.ErrorDestination = myModel->data(index).toInt() - 1;
}

void ModelUavoProxy::pathActionToModel(int i, PathAction::DataFields &data) {
    QModelIndex index = myModel->index(i, flightDataModel::ISRELATIVE);
    myModel->setData(index, true);

    index = myModel->index(i, flightDataModel::COMMAND);
    myModel->setData(index, data.Command);

    index = myModel->index(i, flightDataModel::CONDITION_PARAMS0);
    myModel->setData(index, data.ConditionParameters[0]);
    index = myModel->index(i, flightDataModel::CONDITION_PARAMS1);
    myModel->setData(index, data.ConditionParameters[1]);
    index = myModel->index(i, flightDataModel::CONDITION_PARAMS2);
    myModel->setData(index, data.ConditionParameters[2]);
    index = myModel->index(i, flightDataModel::CONDITION_PARAMS3);
    myModel->setData(index, data.ConditionParameters[3]);

    index = myModel->index(i, flightDataModel::CONDITION);
    myModel->setData(index, data.EndCondition);

    index = myModel->index(i, flightDataModel::ERRORDESTINATION);
    myModel->setData(index, data.ErrorDestination + 1);

    index = myModel->index(i, flightDataModel::JUMPDESTINATION);
    myModel->setData(index, data.JumpDestination + 1);

    index = myModel->index(i, flightDataModel::MODE);
    myModel->setData(index, data.Mode);

    index = myModel->index(i, flightDataModel::MODE_PARAMS0);
    myModel->setData(index, data.ModeParameters[0]);
    index = myModel->index(i, flightDataModel::MODE_PARAMS1);
    myModel->setData(index, data.ModeParameters[1]);
    index = myModel->index(i, flightDataModel::MODE_PARAMS2);
    myModel->setData(index, data.ModeParameters[2]);
    index = myModel->index(i, flightDataModel::MODE_PARAMS3);
    myModel->setData(index, data.ModeParameters[3]);
}
