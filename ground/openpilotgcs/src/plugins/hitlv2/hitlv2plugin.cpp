/**
 ******************************************************************************
 *
 * @file       hitlv2plugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup HITLPlugin HITLv2 Plugin
 * @{
 * @brief The Hardware In The Loop plugin version 2
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

#include "hitlv2plugin.h"
#include "hitlv2factory.h"
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>

#include "aerosimrc.h"

QList<SimulatorCreator* > HITLPlugin::typeSimulators;

HITLPlugin::HITLPlugin()
{
    // Do nothing
}

HITLPlugin::~HITLPlugin()
{
    // Do nothing
}

bool HITLPlugin::initialize(const QStringList& args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);
    mf = new HITLFactory(this);

    addAutoReleasedObject(mf);

    addSimulator(new AeroSimRCSimulatorCreator("ASimRC", "AeroSimRC"));

    return true;
}

void HITLPlugin::extensionsInitialized()
{
    // Do nothing
}

void HITLPlugin::shutdown()
{
    // Do nothing
}
Q_EXPORT_PLUGIN(HITLPlugin)

