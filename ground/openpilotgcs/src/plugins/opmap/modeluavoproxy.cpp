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
#include "uavobjecthelper.h"

#include <QProgressDialog>
#include <math.h>

ModelUavoProxy::ModelUavoProxy(QObject *parent, flightDataModel *model) : QObject(parent), myModel(model)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();

    Q_ASSERT(pm != NULL);

    objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr != NULL);
}

void ModelUavoProxy::sendPathPlan()
{
    modelToObjects();

    PathPlan *pathPlan = PathPlan::GetInstance(objMngr);

    const int waypointCount = pathPlan->getWaypointCount();
    const int actionCount   = pathPlan->getPathActionCount();

    QProgressDialog progress(tr("Sending the path plan to the board... "), "", 0, 1 + waypointCount + actionCount);
    progress.setWindowModality(Qt::WindowModal);
    progress.setCancelButton(NULL);
    progress.show();

    UAVObjectUpdaterHelper updateHelper;

    // send PathPlan
    bool success = (updateHelper.doObjectAndWait(pathPlan) == UAVObjectUpdaterHelper::SUCCESS);
    progress.setValue(1);

    if (success) {
        // send Waypoint instances
        qDebug() << "sending" << waypointCount << "waypoints";
        for (int i = 0; i < waypointCount; ++i) {
            Waypoint *waypoint = Waypoint::GetInstance(objMngr, i);
            success = (updateHelper.doObjectAndWait(waypoint) == UAVObjectUpdaterHelper::SUCCESS);
            if (!success) {
                break;
            }
            progress.setValue(progress.value() + 1);
        }
    }

    if (success) {
        // send PathAction instances
        qDebug() << "sending" << actionCount << "path actions";
        for (int i = 0; i < actionCount; ++i) {
            PathAction *action = PathAction::GetInstance(objMngr, i);
            success = (updateHelper.doObjectAndWait(action) == UAVObjectUpdaterHelper::SUCCESS);
            if (!success) {
                break;
            }
            progress.setValue(progress.value() + 1);
        }
    }

    qDebug() << "ModelUavoProxy::pathPlanSent - completed" << success;
    if (!success) {
        QMessageBox::critical(NULL, tr("Sending Path Plan Failed!"), tr("Failed to send the path plan to the board."));
    }

    progress.close();
}

void ModelUavoProxy::receivePathPlan()
{
    QProgressDialog progress(tr("Receiving the path plan from the board... "), "", 0, 0);
    progress.setWindowModality(Qt::WindowModal);
    progress.setCancelButton(NULL);
    progress.show();

    UAVObjectRequestHelper requestHelper;

    PathPlan *pathPlan = PathPlan::GetInstance(objMngr);
    bool success = (requestHelper.doObjectAndWait(pathPlan) == UAVObjectUpdaterHelper::SUCCESS);

    const int waypointCount = pathPlan->getWaypointCount();
    const int actionCount   = pathPlan->getPathActionCount();

    progress.setMaximum(1 + waypointCount + actionCount);
    progress.setValue(1);

    if (success && (waypointCount > objMngr->getNumInstances(Waypoint::OBJID))) {
        // allocate needed Waypoint instances
        Waypoint *waypoint = new Waypoint;
        waypoint->initialize(waypointCount - 1, waypoint->getMetaObject());
        success = objMngr->registerObject(waypoint);
    }
    if (success) {
        // request Waypoint instances
        qDebug() << "requesting" << waypointCount << "waypoints";
        for (int i = 0; i < waypointCount; ++i) {
            Waypoint *waypoint = Waypoint::GetInstance(objMngr, i);
            success = (requestHelper.doObjectAndWait(waypoint) == UAVObjectRequestHelper::SUCCESS);
            if (!success) {
                break;
            }
            progress.setValue(progress.value() + 1);
        }
    }

    if (success && (actionCount > objMngr->getNumInstances(PathAction::OBJID))) {
        // allocate needed PathAction instances
        PathAction *action = new PathAction;
        action->initialize(actionCount - 1, action->getMetaObject());
        success = objMngr->registerObject(action);
    }
    if (success) {
        // request PathAction isntances
        qDebug() << "requesting" << actionCount << "path actions";
        for (int i = 0; i < actionCount; ++i) {
            PathAction *action = PathAction::GetInstance(objMngr, i);
            success = (requestHelper.doObjectAndWait(action) == UAVObjectRequestHelper::SUCCESS);
            if (!success) {
                break;
            }
            progress.setValue(progress.value() + 1);
        }
    }

    qDebug() << "ModelUavoProxy::pathPlanReceived - completed" << success;
    if (success) {
        objectsToModel();
    } else {
        QMessageBox::critical(NULL, tr("Receiving Path Plan Failed!"), tr("Failed to receive the path plan from the board."));
    }

    progress.close();
}

// update waypoint and path actions UAV objects
//
// waypoints are unique and each waypoint has an entry in the UAV waypoint list
//
// a path action can be referenced by multiple waypoints
// waypoints reference path action by their index in the UAV path action list
// the compression of path actions happens here.
// (compression consists in keeping only one instance of similar path actions)
//
// the UAV waypoint list and path action list are probably not empty, so we try to reuse existing instances
bool ModelUavoProxy::modelToObjects()
{
    qDebug() << "ModelUAVProxy::modelToObjects";

    // track number of path actions
    int actionCount   = 0;

    // iterate over waypoints
    int waypointCount = myModel->rowCount();
    for (int i = 0; i < waypointCount; ++i) {
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

            // update UAVObject
            action->setData(actionData);
        } else {
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

        // update UAVObject
        waypoint->setData(waypointData);
    }

    // Put "safe" values in unused waypoint and path action objects
    if (waypointCount < objMngr->getNumInstances(Waypoint::OBJID)) {
        for (int i = waypointCount; i < objMngr->getNumInstances(Waypoint::OBJID); ++i) {
            // TODO
        }
    }
    if (actionCount < objMngr->getNumInstances(PathAction::OBJID)) {
        for (int i = actionCount; i < objMngr->getNumInstances(PathAction::OBJID); ++i) {
            // TODO
        }
    }

    // Update PathPlan
    PathPlan *pathPlan = PathPlan::GetInstance(objMngr);
    PathPlan::DataFields pathPlanData = pathPlan->getData();

    pathPlanData.WaypointCount   = waypointCount;
    pathPlanData.PathActionCount = actionCount;
    pathPlanData.Crc = computePathPlanCrc(waypointCount, actionCount);

    pathPlan->setData(pathPlanData);

    return true;
}

Waypoint *ModelUavoProxy::createWaypoint(int index, Waypoint *newWaypoint)
{
    Waypoint *waypoint = NULL;
    int count = objMngr->getNumInstances(Waypoint::OBJID);

    if (index < count) {
        // reuse object
        qDebug() << "ModelUAVProxy::createWaypoint - reused waypoint instance :" << index << "/" << count;
        waypoint = Waypoint::GetInstance(objMngr, index);
        if (newWaypoint) {
            newWaypoint->deleteLater();
        }
    } else if (index < count + 1) {
        // create "next" object
        qDebug() << "ModelUAVProxy::createWaypoint - created waypoint instance :" << index;
        // TODO is there a limit to the number of wp?
        waypoint = newWaypoint ? newWaypoint : new Waypoint;
        waypoint->initialize(index, waypoint->getMetaObject());
        objMngr->registerObject(waypoint);
    } else {
        // we can only create the "next" object
        // TODO fail in a clean way :(
    }
    return waypoint;
}

PathAction *ModelUavoProxy::createPathAction(int index, PathAction *newAction)
{
    PathAction *action = NULL;
    int count = objMngr->getNumInstances(PathAction::OBJID);

    if (index < count) {
        // reuse object
        qDebug() << "ModelUAVProxy::createPathAction - reused action instance :" << index << "/" << count;
        action = PathAction::GetInstance(objMngr, index);
        if (newAction) {
            newAction->deleteLater();
        }
    } else if (index < count + 1) {
        // create "next" object
        qDebug() << "ModelUAVProxy::createPathAction - created action instance :" << index;
        // TODO is there a limit to the number of path actions?
        action = newAction ? newAction : new PathAction;
        action->initialize(index, action->getMetaObject());
        objMngr->registerObject(action);
    } else {
        // we can only create the "next" object
        // TODO fail in a clean way :(
    }
    return action;
}

PathAction *ModelUavoProxy::findPathAction(const PathAction::DataFields &actionData, int actionCount)
{
    int instancesCount = objMngr->getNumInstances(PathAction::OBJID);
    int count = actionCount <= instancesCount ? actionCount : instancesCount;

    for (int i = 0; i < count; ++i) {
        PathAction *action = PathAction::GetInstance(objMngr, i);
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

bool ModelUavoProxy::objectsToModel()
{
    // build model from uav objects
    // the list of objects can end with "garbage" instances due to previous flightpath
    // they need to be ignored

    PathPlan *pathPlan = PathPlan::GetInstance(objMngr);
    PathPlan::DataFields pathPlanData = pathPlan->getData();

    int waypointCount  = pathPlanData.WaypointCount;
    int actionCount    = pathPlanData.PathActionCount;

    // consistency check
    if (waypointCount > objMngr->getNumInstances(Waypoint::OBJID)) {
        QMessageBox::critical(NULL, tr("Path Plan Download Failed"), tr("Path plan way point count error !"));
        return false;
    }
    if (actionCount > objMngr->getNumInstances(PathAction::OBJID)) {
        QMessageBox::critical(NULL, tr("Path Plan Download Failed"), tr("Path plan path action count error !"));
        return false;
    }
    if (pathPlanData.Crc != computePathPlanCrc(waypointCount, actionCount)) {
        QMessageBox::critical(NULL, tr("Path Plan Upload Failed"), tr("Path plan CRC error !"));
        return false;
    }

    int rowCount = myModel->rowCount();
    if (waypointCount < rowCount) {
        myModel->removeRows(waypointCount, rowCount - waypointCount);
    } else if (waypointCount > rowCount) {
        myModel->insertRows(rowCount, waypointCount - rowCount);
    }

    for (int i = 0; i < waypointCount; ++i) {
        Waypoint *waypoint = Waypoint::GetInstance(objMngr, i);
        Q_ASSERT(waypoint);
        if (!waypoint) {
            continue;
        }

        Waypoint::DataFields waypointData = waypoint->getData();
        waypointToModel(i, waypointData);

        PathAction *action = PathAction::GetInstance(objMngr, waypoint->getAction());
        Q_ASSERT(action);
        if (!action) {
            continue;
        }

        PathAction::DataFields actionData = action->getData();
        pathActionToModel(i, actionData);
    }
    return true;
}

void ModelUavoProxy::modelToWaypoint(int i, Waypoint::DataFields &data)
{
    double distance, bearing, altitude, velocity;

    QModelIndex index = myModel->index(i, flightDataModel::DISRELATIVE);

    distance = myModel->data(index).toDouble();
    index    = myModel->index(i, flightDataModel::BEARELATIVE);
    bearing  = myModel->data(index).toDouble();
    index    = myModel->index(i, flightDataModel::ALTITUDERELATIVE);
    altitude = myModel->data(index).toFloat();
    index    = myModel->index(i, flightDataModel::VELOCITY);
    velocity = myModel->data(index).toFloat();

    data.Position[Waypoint::POSITION_NORTH] = distance * cos(bearing / 180 * M_PI);
    data.Position[Waypoint::POSITION_EAST]  = distance * sin(bearing / 180 * M_PI);
    data.Position[Waypoint::POSITION_DOWN]  = -altitude;
    data.Velocity = velocity;
}

void ModelUavoProxy::waypointToModel(int i, Waypoint::DataFields &data)
{
    double distance = sqrt(data.Position[Waypoint::POSITION_NORTH] * data.Position[Waypoint::POSITION_NORTH] +
                           data.Position[Waypoint::POSITION_EAST] * data.Position[Waypoint::POSITION_EAST]);

    double bearing  = atan2(data.Position[Waypoint::POSITION_EAST], data.Position[Waypoint::POSITION_NORTH]) * 180 / M_PI;

    if (bearing != bearing) {
        bearing = 0;
    }

    double altitude   = -data.Position[Waypoint::POSITION_DOWN];

    QModelIndex index = myModel->index(i, flightDataModel::VELOCITY);
    myModel->setData(index, data.Velocity);
    index = myModel->index(i, flightDataModel::DISRELATIVE);
    myModel->setData(index, distance);
    index = myModel->index(i, flightDataModel::BEARELATIVE);
    myModel->setData(index, bearing);
    index = myModel->index(i, flightDataModel::ALTITUDERELATIVE);
    myModel->setData(index, altitude);
}

void ModelUavoProxy::modelToPathAction(int i, PathAction::DataFields &data)
{
    QModelIndex index = myModel->index(i, flightDataModel::MODE);

    data.Mode    = myModel->data(index).toInt();
    index        = myModel->index(i, flightDataModel::MODE_PARAMS0);
    data.ModeParameters[0] = myModel->data(index).toFloat();
    index        = myModel->index(i, flightDataModel::MODE_PARAMS1);
    data.ModeParameters[1] = myModel->data(index).toFloat();
    index        = myModel->index(i, flightDataModel::MODE_PARAMS2);
    data.ModeParameters[2] = myModel->data(index).toFloat();
    index        = myModel->index(i, flightDataModel::MODE_PARAMS3);
    data.ModeParameters[3] = myModel->data(index).toFloat();
    index        = myModel->index(i, flightDataModel::CONDITION);
    data.EndCondition = myModel->data(index).toInt();
    index        = myModel->index(i, flightDataModel::CONDITION_PARAMS0);
    data.ConditionParameters[0] = myModel->data(index).toFloat();
    index        = myModel->index(i, flightDataModel::CONDITION_PARAMS1);
    data.ConditionParameters[1] = myModel->data(index).toFloat();
    index        = myModel->index(i, flightDataModel::CONDITION_PARAMS2);
    data.ConditionParameters[2] = myModel->data(index).toFloat();
    index        = myModel->index(i, flightDataModel::CONDITION_PARAMS3);
    data.ConditionParameters[3] = myModel->data(index).toFloat();
    index        = myModel->index(i, flightDataModel::COMMAND);
    data.Command = myModel->data(index).toInt();
    index        = myModel->index(i, flightDataModel::JUMPDESTINATION);
    data.JumpDestination = myModel->data(index).toInt() - 1;
    index        = myModel->index(i, flightDataModel::ERRORDESTINATION);
    data.ErrorDestination = myModel->data(index).toInt() - 1;
}

void ModelUavoProxy::pathActionToModel(int i, PathAction::DataFields &data)
{
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

quint8 ModelUavoProxy::computePathPlanCrc(int waypointCount, int actionCount)
{
    quint8 crc = 0;

    for (int i = 0; i < waypointCount; ++i) {
        Waypoint *waypoint = Waypoint::GetInstance(objMngr, i);
        crc = waypoint->updateCRC(crc);
    }
    for (int i = 0; i < actionCount; ++i) {
        PathAction *action = PathAction::GetInstance(objMngr, i);
        crc = action->updateCRC(crc);
    }
    return crc;
}
