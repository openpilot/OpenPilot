/**
 ******************************************************************************
 *
 * @file       GCSControlplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A gadget to control the UAV, either from the keyboard or a joystick
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
#include "gcscontrolplugin.h"
#include "gcscontrolgadgetfactory.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>


GCSControlPlugin::GCSControlPlugin()
{
   // Do nothing
}

GCSControlPlugin::~GCSControlPlugin()
{
   // Do nothing
}

bool GCSControlPlugin::initialize(const QStringList& args, QString *errMsg)
{
   Q_UNUSED(args);
   Q_UNUSED(errMsg);
    sdlGamepad = new SDLGamepad();
   if(sdlGamepad->init()) {
       sdlGamepad->start();
       qRegisterMetaType<QListInt16>("QListInt16");
       qRegisterMetaType<ButtonNumber>("ButtonNumber");
   }
   mf = new GCSControlGadgetFactory(this);
   addAutoReleasedObject(mf);

   return true;
}

void GCSControlPlugin::extensionsInitialized()
{
   // Do nothing
}

void GCSControlPlugin::shutdown()
{
   // Do nothing
}
Q_EXPORT_PLUGIN(GCSControlPlugin)

/**
  * @}
  * @}
  */
