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
    QWidget(parent),myMap(map),
    ui(new Ui::pathPlanManager)
{
    ui->setupUi(this);
    connect(myMap,SIGNAL(WPDeleted(int)),this,SLOT(on_WPDeleted(int)));
    connect(myMap,SIGNAL(WPInserted(int,WayPointItem*)),this,SLOT(on_WPInserted(int,WayPointItem*)));
    connect(myMap,SIGNAL(WPNumberChanged(int,int,WayPointItem*)),this,SLOT(on_WPNumberChanged(int,int,WayPointItem*)));
    connect(myMap,SIGNAL(WPValuesChanged(WayPointItem*)),this,SLOT(on_WPValuesChanged(WayPointItem*)));

}

pathPlanManager::~pathPlanManager()
{
    delete ui;
}
void pathPlanManager::on_WPDeleted(int wp_number)
{
}

void pathPlanManager::on_WPInserted(int wp_number, WayPointItem * wp)
{
}

void pathPlanManager::on_WPNumberChanged(int oldNumber, int newNumber, WayPointItem * wp)
{
}

void pathPlanManager::on_WPValuesChanged(WayPointItem * wp)
{
}
