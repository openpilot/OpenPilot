/**
 ******************************************************************************
 *
 * @file       importexportplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @brief      Import/Export Plugin for GCS Settings
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @defgroup   importexportplugin
 * @{
 *
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

#include "importexportplugin.h"
#include "importexportdialog.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>

#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icore.h>
#include <QKeySequence>


ImportExportPlugin::ImportExportPlugin()
{
    // Do nothing
}

ImportExportPlugin::~ImportExportPlugin()
{
    // Do nothing
}

bool ImportExportPlugin::initialize(const QStringList& args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);

    // Add Menu entry
    Core::ActionManager* am = Core::ICore::instance()->actionManager();
    Core::ActionContainer* ac = am->actionContainer(Core::Constants::M_FILE);

    Core::Command* cmd = am->registerAction(new QAction(this),
                                            "ImportExportPlugin.ImportExport",
                                            QList<int>() <<
                                            Core::Constants::C_GLOBAL_ID);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+S"));
    cmd->action()->setText(tr("GCS Settings Import/Export..."));

//    ac->menu()->addSeparator();
//    ac->appendGroup("ImportExport");
//    ac->addAction(cmd, "ImportExport");
    ac->addAction(cmd, Core::Constants::G_FILE_SAVE);


    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(importExport()));

    return true;
}

void ImportExportPlugin::importExport()
{
    ImportExportDialog().exec();
}

void ImportExportPlugin::extensionsInitialized()
{
    // Do nothing
}

void ImportExportPlugin::shutdown()
{
    // Do nothing
}
Q_EXPORT_PLUGIN(ImportExportPlugin)

/**
 * @}
 * @}
 */
