/**
 ******************************************************************************
 *
 * @file       configplugin.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   configplugin
 * @{
 *
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
#include "configplugin.h"
#include "configgadgetfactory.h"
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>

ConfigPlugin::ConfigPlugin()
{
   // Do nothing
}

ConfigPlugin::~ConfigPlugin()
{
   // Do nothing
}

bool ConfigPlugin::initialize(const QStringList& args, QString *errMsg)
{
   Q_UNUSED(args);
   Q_UNUSED(errMsg);
  cf = new ConfigGadgetFactory(this);
  addAutoReleasedObject(cf);

   return true;
}

void ConfigPlugin::extensionsInitialized()
{
   // Do nothing
}

void ConfigPlugin::shutdown()
{
   // Do nothing
}
Q_EXPORT_PLUGIN(ConfigPlugin)
