/**
 ******************************************************************************
 *
 * @file       modelviewplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ModelViewPlugin ModelView Plugin
 * @{
 * @brief A gadget that displays a 3D representation of the UAV 
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
#include "modelviewplugin.h"
#include "modelviewgadgetfactory.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>


ModelViewPlugin::ModelViewPlugin()
{
   // Do nothing
}

ModelViewPlugin::~ModelViewPlugin()
{
   // Do nothing
}

bool ModelViewPlugin::initialize(const QStringList& args, QString *errMsg)
{
   Q_UNUSED(args);
   Q_UNUSED(errMsg);
   mvf = new ModelViewGadgetFactory(this);
   addAutoReleasedObject(mvf);

   return true;
}

void ModelViewPlugin::extensionsInitialized()
{
   // Do nothing
}

void ModelViewPlugin::shutdown()
{
   // Do nothing
}
Q_EXPORT_PLUGIN(ModelViewPlugin)

