/*
 * emptyplugin.cpp
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */
#include "emptyplugin.h"
#include "emptygadgetfactory.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>


EmptyPlugin::EmptyPlugin()
{
   // Do nothing
}

EmptyPlugin::~EmptyPlugin()
{
   // Do nothing
}

bool EmptyPlugin::initialize(const QStringList& args, QString *errMsg)
{
   Q_UNUSED(args);
   Q_UNUSED(errMsg);
   mf = new EmptyGadgetFactory(this);
   addAutoReleasedObject(mf);

   return true;
}

void EmptyPlugin::extensionsInitialized()
{
   // Do nothing
}

void EmptyPlugin::shutdown()
{
   // Do nothing
}
Q_EXPORT_PLUGIN(EmptyPlugin)

