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
#include "pathcompiler.h"
#include "uavobjectmanager.h"
#include "waypoint.h"
#include "homelocation.h"

PathCompiler::PathCompiler(QObject *parent) :
    QObject(parent)
{
    Waypoint *waypoint = NULL;
    HomeLocation *homeLocation = NULL;

    /* Connect the object updates */
    waypoint = Waypoint::GetInstance(getObjectManger());
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
    UAVObjectUtilManager *objMngr = NULL;

    pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    if(pm)
        objMngr = pm->getObject<UAVObjectUtilManager>();
    Q_ASSERT(objMngr);

    return objMngr;
}

/**
 * This method opens a dialog (if filename is null) and saves the path
 * @param filename The file to save the path to
 * @returns -1 for failure, 0 for success
 */
int PathCompiler::savePath(QString filename = null)
{
    return -1;
}

/**
 * This method opens a dialog (if filename is null) and loads the path
 * @param filename The file to load from
 * @returns -1 for failure, 0 for success
 */
int PathCompiler::loadPath(QString filename = null)
{
    return -1;
}

/**
  * add a waypoint
  * @param waypoint the new waypoint to add
  * @param position which position to insert it to, defaults to end
  */
void PathCompiler::doAddWaypoint(struct PathCompiler::waypoint, int position = -1)
{
    emit visualizationChanged(waypoints);
}

/**
  * Delete a waypoint
  * @param index which waypoint to delete
  */
void PathCompiler::doDelWaypoint(int index)
{
    emit visualizationChanged(waypoints);
}

/**
  * When the UAV waypoints change trigger the pathcompiler to
  * get the latest version and then update the visualization
  */
void PathCompiler::doUpdateFromUAV()
{
    emit visualizationChanged(waypoints);
}
