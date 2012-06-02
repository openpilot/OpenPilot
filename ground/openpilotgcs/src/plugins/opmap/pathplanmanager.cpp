/**
 ******************************************************************************
 *
 * @file       pathplanmanager.cpp
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
#include "pathplanmanager.h"
#include "ui_pathplanmanager.h"

pathPlanManager::pathPlanManager(QWidget *parent,OPMapWidget *map):
    QDialog(parent, Qt::Dialog),myMap(map),
    ui(new Ui::pathPlanManager)
{
    waypoints=new QList<WayPointItem*>();
    ui->setupUi(this);
    connect(myMap,SIGNAL(WPDeleted(int,WayPointItem*)),this,SLOT(on_WPDeleted(int,WayPointItem*)));
    connect(myMap,SIGNAL(WPInserted(int,WayPointItem*)),this,SLOT(on_WPInserted(int,WayPointItem*)));
    connect(myMap,SIGNAL(WPCreated(int,WayPointItem*)),this,SLOT(on_WPInserted(int,WayPointItem*)));
    connect(myMap,SIGNAL(WPNumberChanged(int,int,WayPointItem*)),this,SLOT(refreshOverlays()));
    connect(myMap,SIGNAL(WPValuesChanged(WayPointItem*)),this,SLOT(refreshOverlays()));
    }

pathPlanManager::~pathPlanManager()
{
    delete ui;
}
void pathPlanManager::on_WPDeleted(int wp_numberint,WayPointItem * wp)
{
    waypoints->removeOne(wp);
}

void pathPlanManager::on_WPInserted(int wp_number, WayPointItem * wp)
{
    qDebug()<<"pathplanner waypoint added";
    waypoints->append(wp);
    customData data;
    data.mode=PathAction::MODE_FLYENDPOINT;
    data.condition=PathAction::ENDCONDITION_NONE;
    data.velocity=0;
    wp->customData().setValue(data);
    refreshOverlays();
}

void pathPlanManager::on_WPNumberChanged(int oldNumber, int newNumber, WayPointItem * wp)
{
}

void pathPlanManager::on_WPValuesChanged(WayPointItem * wp)
{
}
//typedef enum { MODE_FLYENDPOINT=0, MODE_FLYVECTOR=1, MODE_FLYCIRCLERIGHT=2,
//MODE_FLYCIRCLELEFT=3, MODE_DRIVEENDPOINT=4, MODE_DRIVEVECTOR=5, MODE_DRIVECIRCLELEFT=6,
//MODE_DRIVECIRCLERIGHT=7, MODE_FIXEDATTITUDE=8, MODE_SETACCESSORY=9, MODE_DISARMALARM=10 } ModeOptions;

void pathPlanManager::refreshOverlays()
{
    myMap->deleteAllOverlays();
    qDebug()<<"foreach start";
    foreach(WayPointItem * wp,*waypoints)
    {
        qDebug()<<"wp:"<<wp->Number();
        customData data=wp->customData().value<customData>();
        switch(data.mode)
        {
        case PathAction::MODE_FLYENDPOINT:
        case PathAction::MODE_FLYVECTOR:
        case PathAction::MODE_DRIVEENDPOINT:
        case PathAction::MODE_DRIVEVECTOR:
            qDebug()<<"addline";
            if(wp->Number()==0)
                myMap->WPLineCreate((HomeItem*)myMap->Home,wp);
            else
                myMap->WPLineCreate(findWayPointNumber(wp->Number()-1),wp);
            break;
        case PathAction::MODE_FLYCIRCLERIGHT:
        case PathAction::MODE_DRIVECIRCLERIGHT:
            myMap->WPCircleCreate(findWayPointNumber(wp->Number()-1),wp,true);
            break;
        case PathAction::MODE_FLYCIRCLELEFT:
        case PathAction::MODE_DRIVECIRCLELEFT:
            myMap->WPCircleCreate(findWayPointNumber(wp->Number()-1),wp,false);
            break;
        default:
            break;

        }
    }
    qDebug()<<"foreach end";
}

WayPointItem * pathPlanManager::findWayPointNumber(int number)
{
    foreach(WayPointItem * wp,*waypoints)
    {
        if(wp->Number()==number)
            return wp;
    }
    return NULL;
}
