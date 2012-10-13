/**
 ******************************************************************************
 *
 * @file       setupwizardplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup SetupWizardPlugin
 * @{
 * @brief A Setup Wizard Plugin
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
#include "setupwizardplugin.h"

#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>

#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icore.h>
#include <QKeySequence>
#include <coreplugin/modemanager.h>

SetupWizardPlugin::SetupWizardPlugin() : wizardRunning(false)
{
}

SetupWizardPlugin::~SetupWizardPlugin()
{
}

bool SetupWizardPlugin::initialize(const QStringList& args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);

    // Add Menu entry
    Core::ActionManager* am = Core::ICore::instance()->actionManager();
    Core::ActionContainer* ac = am->actionContainer(Core::Constants::M_TOOLS);

    Core::Command* cmd = am->registerAction(new QAction(this),
                                            "SetupWizardPlugin.ShowSetupWizard",
                                            QList<int>() <<
                                            Core::Constants::C_GLOBAL_ID);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+V"));
    cmd->action()->setText(tr("Vehicle Setup Wizard"));

    Core::ModeManager::instance()->addAction(cmd, 1);

    ac->menu()->addSeparator();
    ac->appendGroup("Wizard");
    ac->addAction(cmd, "Wizard");

    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(showSetupWizard()));
    return true;
}

void SetupWizardPlugin::extensionsInitialized()
{
}

void SetupWizardPlugin::shutdown()
{
}

void SetupWizardPlugin::showSetupWizard()
{
    if (!wizardRunning) {
        wizardRunning = true;
        SetupWizard *m_wiz = new SetupWizard();
        connect(m_wiz, SIGNAL(finished(int)), this, SLOT(wizardTerminated()));
        m_wiz->setAttribute( Qt::WA_DeleteOnClose, true );
        m_wiz->setWindowFlags(m_wiz->windowFlags() | Qt::WindowStaysOnTopHint);
        m_wiz->show();
    }
}

void SetupWizardPlugin::wizardTerminated()
{
    wizardRunning = false;
    disconnect(this,SLOT(wizardTerminated()));
}

Q_EXPORT_PLUGIN(SetupWizardPlugin)
