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
#include "escwizard.h"

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
    ac->menu()->addSeparator();
    ac->appendGroup("Wizard");

    Core::Command* cmd = am->registerAction(new QAction(this), "SetupWizardPlugin.ShowSetupWizard",
                                            QList<int>() << Core::Constants::C_GLOBAL_ID);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+Alt+V"));
    cmd->action()->setText(tr("Vehicle Setup Wizard"));

    //This is to prevent user to open more than one wizard at the time.
    //The RadioSetupWizard will be added to this logic when it is moved from the config plugin.
    connect(this, SIGNAL(wizardIsOpen(bool)), cmd->action(), SLOT(setDisabled(bool)));
    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(showSetupWizard()));

    Core::ModeManager::instance()->addAction(cmd, 1);
    ac->addAction(cmd, "Wizard");


    cmd = am->registerAction(new QAction(this), "SetupWizardPlugin.ShowESCWizard",
                                            QList<int>() << Core::Constants::C_GLOBAL_ID);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+Alt+E"));
    cmd->action()->setText(tr("ESC Calibration Wizard"));

    connect(this, SIGNAL(wizardIsOpen(bool)), cmd->action(), SLOT(setDisabled(bool)));
    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(showESCWizard()));

    Core::ModeManager::instance()->addAction(cmd, 1);
    ac->addAction(cmd, "Wizard");

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
        SetupWizard *wiz = new SetupWizard();
        connect(wiz, SIGNAL(finished(int)), this, SLOT(wizardTerminated()));
        wiz->setAttribute( Qt::WA_DeleteOnClose, true );
        wiz->setWindowFlags(wiz->windowFlags() | Qt::WindowStaysOnTopHint);
        emit wizardIsOpen(true);
        wiz->show();
    }
}

void SetupWizardPlugin::showESCWizard()
{
    if (!wizardRunning) {
        wizardRunning = true;
        ESCWizard *wiz = new ESCWizard();
        connect(wiz, SIGNAL(finished(int)), this, SLOT(wizardTerminated()));
        wiz->setAttribute( Qt::WA_DeleteOnClose, true );
        wiz->setWindowFlags(wiz->windowFlags() | Qt::WindowStaysOnTopHint);
        emit wizardIsOpen(true);
        wiz->show();
    }
}

void SetupWizardPlugin::wizardTerminated()
{
    wizardRunning = false;
    emit wizardIsOpen(false);
    disconnect(this, SLOT(wizardTerminated()));
}

Q_EXPORT_PLUGIN(SetupWizardPlugin)
