/**
 ******************************************************************************
 *
 * @file       opmapplugin.cpp
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
#include "opmapplugin.h"
#include "opmapgadgetfactory.h"
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>

OPMapPlugin::OPMapPlugin()
{
   // Do nothing
}

OPMapPlugin::~OPMapPlugin()
{
   // Do nothing
}

bool OPMapPlugin::initialize(const QStringList& args, QString *errMsg)
{
   Q_UNUSED(args);
   Q_UNUSED(errMsg);

   mf = new OPMapGadgetFactory(this);
   addAutoReleasedObject(mf);

   return true;
}

void OPMapPlugin::extensionsInitialized()
{
   // Do nothing
}

void OPMapPlugin::shutdown()
{
   // Do nothing
}
Q_EXPORT_PLUGIN(OPMapPlugin)
