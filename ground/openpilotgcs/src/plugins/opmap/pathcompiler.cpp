/**
 ******************************************************************************
 *
 * @file       pathcompiler.h
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
#include <pathcompiler.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/coordinateconversions.h>
#include <uavobjectmanager.h>
#include <waypoint.h>
#include <homelocation.h>

#include <QDebug>

PathCompiler::PathCompiler(QObject *parent) :
    QObject(parent)
{
    Waypoint *waypoint = NULL;
    HomeLocation *homeLocation = NULL;

    /* Connect the object updates */
    waypoint = Waypoint::GetInstance(getObjectManager());
    Q_ASSERT(waypoint);
    if(waypoint)
        connect(waypoint, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(doUpdateFromUAV()));

    homeLocation = HomeLocation::GetInstance(getObjectManager());
    Q_ASSERT(homeLocation);
    if(homeLocation)
        connect(homeLocation, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(doUpdateFromUAV()));
}

/**
  * Helper method to get the uavobjectamanger
  */
UAVObjectManager * PathCompiler::getObjectManager()
{
    ExtensionSystem::PluginManager *pm = NULL;
    UAVObjectManager *objMngr = NULL;

    pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    if(pm)
        objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr);

    return objMngr;
}

/**
 * This method opens a dialog (if filename is null) and saves the path
 * @param filename The file to save the path to
 * @returns -1 for failure, 0 for success
 */
int PathCompiler::savePath(QString filename)
{
    Q_UNUSED(filename);
    return -1;
}

/**
 * This method opens a dialog (if filename is null) and loads the path
 * @param filename The file to load from
 * @returns -1 for failure, 0 for success
 */
int PathCompiler::loadPath(QString filename)
{
    Q_UNUSED(filename);
    return -1;
}

/**
  * add a waypoint
  * @param waypoint the new waypoint to add
  * @param position which position to insert it to, defaults to end
  */
void PathCompiler::doAddWaypoint(struct PathCompiler::waypoint waypoint, int /*position*/)
{
    /* TODO: If a waypoint is inserted not at the end shift them all by one and */
    /* add the data there */

    UAVObjectManager *objManager = getObjectManager();

    // Format the data from the map into a UAVO
    Waypoint::DataFields newWaypoint = InternalToUavo(waypoint);

    // Search for any waypoints set to stop, because if they exist we
    // should add the waypoint immediately after that one
    int numWaypoints = objManager->getNumInstances(Waypoint::OBJID);
    int i;
    for (i = 0; i < numWaypoints; i++) {
        Waypoint *waypoint = Waypoint::GetInstance(objManager, i);
        Q_ASSERT(waypoint);
        if(waypoint == NULL)
            return;

        Waypoint::DataFields waypointData = waypoint->getData();
        if(waypointData.Action == Waypoint::ACTION_STOP) {
            waypointData.Action = Waypoint::ACTION_PATHTONEXT;
            waypoint->setData(waypointData);
            break;
        }
    }

    if (i >= numWaypoints - 1) {
        // We reached end of list so new waypoint needs to be registered

        Waypoint *waypoint = new Waypoint();
        Q_ASSERT(waypoint);
        if (waypoint) {
            // Register a new waypoint instance
            quint32 newInstId = objManager->getNumInstances(waypoint->getObjID());
            waypoint->initialize(newInstId,waypoint->getMetaObject());
            objManager->registerObject(waypoint);

            // Set the data in the new object
            waypoint->setData(newWaypoint);
            waypoint->updated();
        }
    } else {
        Waypoint *waypoint = Waypoint::GetInstance(objManager, i + 1);
        Q_ASSERT(waypoint);
        if (waypoint) {
            waypoint->setData(newWaypoint);
            waypoint->updated();
        }
    }
}

/**
  * Delete a waypoint
  * @param index which waypoint to delete
  */
void PathCompiler::doDelWaypoint(int index)
{
    // This method is awkward because there is no support
    // on the FC for actually deleting a waypoint.  We need
    // to shift them all by one and set the new "last" waypoint
    // to a stop action

    UAVObjectManager *objManager = getObjectManager();
    Waypoint *waypoint = Waypoint::GetInstance(objManager);
    Q_ASSERT(waypoint);
    if (!waypoint)
        return;

    int numWaypoints = objManager->getNumInstances(waypoint->getObjID());
    for (int i = index; i < numWaypoints - 1; i++) {
        qDebug() << "Copying from" << i+1 << "to" << i;
        Waypoint *waypointDest = Waypoint::GetInstance(objManager, i);
        Q_ASSERT(waypointDest);

        Waypoint *waypointSrc = Waypoint::GetInstance(objManager, i + 1);
        Q_ASSERT(waypointSrc);

        if (!waypointDest || !waypointSrc)
            return;

        // Copy the data down an index
        Waypoint::DataFields waypoint = waypointSrc->getData();
        waypointDest->setData(waypoint);
        waypointDest->updated();
    }

    // Set the second to last waypoint to stop (and last for safety)
    // the functional equivalent to deleting
    for (int i = numWaypoints - 2; i < numWaypoints; i++) {
        qDebug() << "Stopping" << i;
        waypoint = Waypoint::GetInstance(objManager, i);
        Q_ASSERT(waypoint);
        if (waypoint) {
            Waypoint::DataFields waypointData = waypoint->getData();
            waypointData.Action = Waypoint::ACTION_STOP;
            waypoint->setData(waypointData);
            waypoint->updated();
        }
    }
}

/**
  * Delete all the waypoints
  */
void PathCompiler::doDelAllWaypoints()
{
    Waypoint *waypointObj = Waypoint::GetInstance(getObjectManager(), 0);
    Q_ASSERT(waypointObj);
    if (waypointObj == NULL)
        return;

    int numWaypoints = getObjectManager()->getNumInstances(waypointObj->getObjID());
    for (int i = 0; i < numWaypoints; i++) {
        Waypoint *waypoint = Waypoint::GetInstance(getObjectManager(), i);
        Q_ASSERT(waypoint);
        if (waypoint) {
            Waypoint::DataFields waypointData = waypoint->getData();
            waypointData.Action = Waypoint::ACTION_STOP;
            waypoint->setData(waypointData);
        }
    }

    waypointObj->updated();
}

/**
  * When the UAV waypoints change trigger the pathcompiler to
  * get the latest version and then update the visualization
  */
void PathCompiler::doUpdateFromUAV()
{
    UAVObjectManager *objManager = getObjectManager();
    if (!objManager)
        return;

    Waypoint *waypointObj = Waypoint::GetInstance(getObjectManager());
    Q_ASSERT(waypointObj);
    if (waypointObj == NULL)
        return;

    /* Get all the waypoints from the UAVO and create a representation for the visualization */
    QList <struct PathCompiler::waypoint> waypoints;
    waypoints.clear();
    int numWaypoints = objManager->getNumInstances(waypointObj->getObjID());
    bool stopped = false;
    for (int i = 0; i < numWaypoints && !stopped; i++) {
        Waypoint *waypoint = Waypoint::GetInstance(objManager, i);
        Q_ASSERT(waypoint);
        if(waypoint == NULL)
            return;

        Waypoint::DataFields waypointData = waypoint->getData();
        waypoints.append(UavoToInternal(waypointData));
        stopped = waypointData.Action == Waypoint::ACTION_STOP;
    }

    /* Inform visualization */
    emit visualizationChanged(waypoints);
}

/**
  * Convert a UAVO waypoint to the local structure
  * @param uavo The UAVO data representation
  * @return The waypoint structure for visualization
  */
struct PathCompiler::waypoint PathCompiler::UavoToInternal(Waypoint::DataFields uavo)
{
    double homeLLA[3];
    double LLA[3];
    double NED[3];
    struct PathCompiler::waypoint internalWaypoint;

    HomeLocation *homeLocation = HomeLocation::GetInstance(getObjectManager());
    Q_ASSERT(homeLocation);
    if (homeLocation == NULL)
        return internalWaypoint;
    HomeLocation::DataFields homeLocationData = homeLocation->getData();
    homeLLA[0] = homeLocationData.Latitude / 10e6;
    homeLLA[1] = homeLocationData.Longitude / 10e6;
    homeLLA[2] = homeLocationData.Altitude;

    NED[0] = uavo.Position[Waypoint::POSITION_NORTH];
    NED[1] = uavo.Position[Waypoint::POSITION_EAST];
    NED[2] = uavo.Position[Waypoint::POSITION_DOWN];
    Utils::CoordinateConversions().GetLLA(homeLLA, NED, LLA);

    internalWaypoint.latitude = LLA[0];
    internalWaypoint.longitude = LLA[1];
    return internalWaypoint;
}

/**
  * Convert a UAVO waypoint to the local structure
  * @param internal The internal structure type
  * @returns The waypoint UAVO data structure
  */
Waypoint::DataFields PathCompiler::InternalToUavo(struct waypoint internal)
{
    Waypoint::DataFields uavo;

    double homeLLA[3];
    double LLA[3];
    double NED[3];

    HomeLocation *homeLocation = HomeLocation::GetInstance(getObjectManager());
    Q_ASSERT(homeLocation);
    if (homeLocation == NULL)
        return uavo;
    HomeLocation::DataFields homeLocationData = homeLocation->getData();
    homeLLA[0] = homeLocationData.Latitude / 10e6;
    homeLLA[1] = homeLocationData.Longitude / 10e6;
    homeLLA[2] = homeLocationData.Altitude;

    // TODO: Give the point a concept of altitude
    LLA[0] = internal.latitude;
    LLA[1] = internal.longitude;
    LLA[2] = -50;

    Utils::CoordinateConversions().GetNED(homeLLA, LLA, NED);

    uavo.Position[Waypoint::POSITION_NORTH] = NED[0];
    uavo.Position[Waypoint::POSITION_EAST] = NED[1];
    uavo.Position[Waypoint::POSITION_DOWN] = NED[2];

    uavo.Action = Waypoint::ACTION_PATHTONEXT;

    uavo.Velocity[Waypoint::VELOCITY_NORTH] = 5;
    uavo.Velocity[Waypoint::VELOCITY_EAST] = 0;
    uavo.Velocity[Waypoint::VELOCITY_DOWN] = 0;

    return uavo;
}

