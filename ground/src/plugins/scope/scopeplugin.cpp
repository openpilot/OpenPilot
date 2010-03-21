/*
 * scopeplugin.cpp
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */
#include "scopeplugin.h"
#include "scopegadgetfactory.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>


ScopePlugin::ScopePlugin()
{
   // Do nothing
}

ScopePlugin::~ScopePlugin()
{
   // Do nothing
}

bool ScopePlugin::initialize(const QStringList& args, QString *errMsg)
{
   Q_UNUSED(args);
   Q_UNUSED(errMsg);
   mf = new ScopeGadgetFactory(this);
   addAutoReleasedObject(mf);

   return true;
}

void ScopePlugin::extensionsInitialized()
{
   // Do nothing
}

void ScopePlugin::shutdown()
{
   // Do nothing
}
Q_EXPORT_PLUGIN(ScopePlugin)

