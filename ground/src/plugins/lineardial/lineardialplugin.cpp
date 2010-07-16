/**
 ******************************************************************************
 *
 * @file       lineardialplugin.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup LinearDialPlugin Linear Dial Plugin
 * @{
 * @brief Impliments a gadget that displays linear gauges 
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

#include "lineardialplugin.h"
#include "lineardialgadgetfactory.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>


LineardialPlugin::LineardialPlugin()
{
   // Do nothing
}

LineardialPlugin::~LineardialPlugin()
{
   // Do nothing
}

bool LineardialPlugin::initialize(const QStringList& args, QString *errMsg)
{
   Q_UNUSED(args);
   Q_UNUSED(errMsg);
   mf = new LineardialGadgetFactory(this);
   addAutoReleasedObject(mf);

   return true;
}

void LineardialPlugin::extensionsInitialized()
{
   // Do nothing
}

void LineardialPlugin::shutdown()
{
   // Do nothing
}
Q_EXPORT_PLUGIN(LineardialPlugin)

