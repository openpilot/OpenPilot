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
#ifndef PATHCOMPILER_H
#define PATHCOMPILER_H

#include <QObject>
#include <uavobject.h>
#include <uavobjectmanager.h>
#include <waypoint.h>

// TODO: Make this a singleton class and separate from map library.  Not sure of the proper design pattern in Qt.
// factory? static variables?

/**
  * This class is a two way adapter between a visualization of a path and the
  * UAVObject representation on the flight controller.  It also can support multiple
  * ways of converting a path from what the user clicked to the underlying representation
  * to achieve the desired end flight trajectory
  *
  * So the chain of data for the map lib is:
  *    FC <-> PathCompiler <-> OPMapGadget <-> OPMapLib
  *
  * The goal is that PathCompiler be as state free as is possible.  Eventually for more
  * complicated path compilation this will probably not be achievable.  That means it
  * should not cache a copy of waypoints locally if that can be avoided (i.e. it should
  * refer directly to what is stored on the FC).
  *
  * For the visualization to have the ability to manipulate the path though it needs to
  * be able to map unambiguously from the graphical items to the internal waypoints. It
  * must cache a lookup from the graphical item to the index from this tool.
  */
class PathCompiler : public QObject
{
    Q_OBJECT
public:
    explicit PathCompiler(QObject *parent = 0);

    //! This method opens a dialog (if filename is null) and saves the path
    int savePath(QString filename = NULL);

    //! This method opens a dialog (if filename is null) and loads the path
    int loadPath(QString filename = NULL);

    //! Waypoint representation that is exchanged between visualization
    class waypoint {
    public:
        waypoint() {}

        const bool operator==(const waypoint other) {
            return (other.latitude == latitude) && (other.longitude == longitude);
        }

        double latitude;
        double longitude;
        double altitude;
    };

private:
    //! Helper method to get uavobject manager
    UAVObjectManager * getObjectManager();

    //! Convert a UAVO waypoint to the local structure
    struct PathCompiler::waypoint UavoToInternal(Waypoint::DataFields);

    //! Convert a UAVO waypoint to the local structure
    Waypoint::DataFields InternalToUavo(waypoint);

    QList <waypoint> previousWaypoints;
signals:
    /**
      * Indicates something changed the waypoints and the map should
      * update the display
      */
    void visualizationChanged(QList<PathCompiler::waypoint>);

public slots:
    /**
      * These are slots that the visualization can call to manipulate the path.
      * It is an important design detail that the visualiation _not_ attempt to maintain
      * the list of waypoints itself.  This starts the slippery of moving the path logic
      * into the view.
      */

    /**
      * Called when new instances are registered
      */
    void doNewInstance(UAVObject*);

    /**
      * add a waypoint
      * @param waypoint the new waypoint to add
      * @param position which position to insert it to, defaults to end
      */
    void doAddWaypoint(waypoint, int position = -1);

    /**
      * Update waypoint
      */
    void doUpdateWaypoints(PathCompiler::waypoint, int position);

    /**
      * Delete a waypoint
      * @param index which waypoint to delete
      */
    void doDelWaypoint(int index);

    /**
      * Delete all the waypoints
      */
    void doDelAllWaypoints();

public slots:
    /**
      * These are slots that the UAV can call to update the path.
      */

    /**
      * When the UAV waypoints change trigger the pathcompiler to
      * get the latest version and then update the visualization
      */
    void doUpdateFromUAV(UAVObject *);
};

#endif // PATHCOMPILER_H
