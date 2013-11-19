/**
 ******************************************************************************
 *
 * @file       flightlogplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @brief A plugin to view and download flight side logs.
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
#include "flightlogplugin.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>

#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icore.h>
#include <QKeySequence>
#include <coreplugin/modemanager.h>

FlightLogPlugin::FlightLogPlugin()
{
    m_manager = new FlightLogManager();
}

FlightLogPlugin::~FlightLogPlugin()
{
    delete m_manager;
    m_manager = 0;
}

bool FlightLogPlugin::initialize(const QStringList & args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);
    
    // Add Menu entry
    Core::ActionManager *am   = Core::ICore::instance()->actionManager();
    Core::ActionContainer *ac = am->actionContainer(Core::Constants::M_TOOLS);

    Core::Command *cmd = am->registerAction(new QAction(this),
                                            "FlightLogPlugin.ShowFlightLogDialog",
                                            QList<int>() <<
                                            Core::Constants::C_GLOBAL_ID);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+F"));
    cmd->action()->setText(tr("Manage flight side logs..."));

    Core::ModeManager::instance()->addAction(cmd, 1);

    ac->menu()->addSeparator();
    ac->appendGroup("FlightLogs");
    ac->addAction(cmd, "FlightLogs");

    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(ShowLogManagementDialog()));
    return true;
}

void FlightLogPlugin::ShowLogManagementDialog()
{

}

void FlightLogPlugin::extensionsInitialized()
{
}

void FlightLogPlugin::shutdown()
{
}
