/**
 ******************************************************************************
 *
 * @file       pathplanmanager.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#ifndef PATHPLANMANAGER_H
#define PATHPLANMANAGER_H

#include <QWidget>
#include "opmapcontrol/opmapcontrol.h"
#include "pathaction.h"
#include "waypoint.h"
#include "QMutexLocker"
#include "QPointer"
namespace mapcontrol
{
    struct customData
    {
        float velocity;
        int mode;
        float mode_params[4];
        int condition;
        float condition_params[4];
        int command;
        int jumpdestination;
        int errordestination;
    };

}
Q_DECLARE_METATYPE(mapcontrol::customData)
namespace Ui {
class pathPlanManager;
}
using namespace mapcontrol;
class pathPlanManager : public QDialog
{
    Q_OBJECT
public:
    explicit pathPlanManager(QWidget *parent,OPMapWidget * map);
    ~pathPlanManager();
    WayPointItem *findWayPointNumber(int number);
private slots:
    void refreshOverlays();
    void on_WPDeleted(int wp_numberint, WayPointItem *);
    void on_WPInserted(int,WayPointItem*);
    void on_WPValuesChanged(WayPointItem*);
private:
    Ui::pathPlanManager *ui;
    OPMapWidget * myMap;
    QList<QPointer<WayPointItem> > * waypoints;
    QMutex wplistmutex;
};

#endif // PATHPLANMANAGER_H
