/**
 ******************************************************************************
 *
 * @file       modeluavproxy.h
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
#ifndef MODELUAVOPROXY_H
#define MODELUAVOPROXY_H

#include <QObject>
#include "flightdatamodel.h"
#include "pathaction.h"
#include "waypoint.h"

class modelUavoProxy:public QObject
{
    Q_OBJECT
public:
    explicit modelUavoProxy(QObject *parent, flightDataModel *model);
    int addAction(PathAction *actionObj, PathAction::DataFields actionFields,int lastaction);
public slots:
    void modelToObjects();
    void objectsToModel();
private:
    UAVObjectManager *objManager;
    Waypoint * waypointObj;
    PathAction * pathactionObj;
    flightDataModel * myModel;
};

#endif // MODELUAVOPROXY_H
