/*
 * mapplugin.cpp
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */
#include "mapplugin.h"
#include "mapgadgetfactory.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>


MapPlugin::MapPlugin()
{
   // Do nothing
}

MapPlugin::~MapPlugin()
{
   // Do nothing
}

bool MapPlugin::initialize(const QStringList& args, QString *errMsg)
{
   Q_UNUSED(args);
   Q_UNUSED(errMsg);
   mf = new MapGadgetFactory(this);
   addAutoReleasedObject(mf);

   return true;
}

void MapPlugin::extensionsInitialized()
{
   // Do nothing
}

void MapPlugin::shutdown()
{
   // Do nothing
}
Q_EXPORT_PLUGIN(MapPlugin)

