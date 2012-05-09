/**
 ******************************************************************************
 * @file       waypointeditorgadgetfactor.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup Waypoint Editor GCS Plugins
 * @{
 * @addtogroup WaypointEditorGadgetPlugin Waypoint Editor Gadget Plugin
 * @{
 * @brief A gadget to edit a list of waypoints
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
#include "waypointeditorgadgetfactory.h"
#include "waypointeditorgadgetwidget.h"
#include "waypointeditorgadget.h"
#include <coreplugin/iuavgadget.h>
#include <QDebug>

WaypointEditorGadgetFactory::WaypointEditorGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("WaypointEditorGadget"),
                          tr("Waypoint Editor"),
                          parent)
{
}

WaypointEditorGadgetFactory::~WaypointEditorGadgetFactory()
{

}

IUAVGadget* WaypointEditorGadgetFactory::createGadget(QWidget *parent) {
    WaypointEditorGadgetWidget* gadgetWidget = new WaypointEditorGadgetWidget(parent);
    return new WaypointEditorGadget(QString("WaypointEditorGadget"), gadgetWidget, parent);
}
