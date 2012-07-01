/**
 ******************************************************************************
 * @file       waypointeditorplugin.cpp
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
#include "waypointeditorplugin.h"
#include "waypointeditorgadgetfactory.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>


WaypointEditorPlugin::WaypointEditorPlugin()
{
   // Do nothing
}

WaypointEditorPlugin::~WaypointEditorPlugin()
{
   // Do nothing
}

bool WaypointEditorPlugin::initialize(const QStringList& args, QString *errMsg)
{
   Q_UNUSED(args);
   Q_UNUSED(errMsg);
   mf = new WaypointEditorGadgetFactory(this);
   addAutoReleasedObject(mf);

   return true;
}

void WaypointEditorPlugin::extensionsInitialized()
{
   // Do nothing
}

void WaypointEditorPlugin::shutdown()
{
   // Do nothing
}
Q_EXPORT_PLUGIN(WaypointEditorPlugin)

/**
  * @}
  * @}
  */
