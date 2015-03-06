/**
 ******************************************************************************
 *
 * @file       filesyncplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @addtogroup GCSPlugins GCS Plugins
 * @brief A plugin to transfer files to/from the OP board.
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
#include "filesyncplugin.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>
#include <QtQuick>

#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icore.h>
#include <QKeySequence>
#include <coreplugin/modemanager.h>
#include "filesyncmanager.h"
#include "uavobject.h"

FileSyncPlugin::FileSyncPlugin() : m_syncDialog(0)
{}

FileSyncPlugin::~FileSyncPlugin()
{
    shutdown();
}

bool FileSyncPlugin::initialize(const QStringList & args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);

    // Add Menu entry
    Core::ActionManager *am   = Core::ICore::instance()->actionManager();
    Core::ActionContainer *ac = am->actionContainer(Core::Constants::M_TOOLS);

    Core::Command *cmd = am->registerAction(new QAction(this),
                                            "FileSyncPlugin.ShowFileSyncDialog",
                                            QList<int>() <<
                                            Core::Constants::C_GLOBAL_ID);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+S"));
    cmd->action()->setText(tr("Download/Upload files..."));

    Core::ModeManager::instance()->addAction(cmd, 1);

    ac->menu()->addSeparator();
    ac->appendGroup("FileSyncs");
    ac->addAction(cmd, "FileSyncs");

    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(ShowSyncManagementDialog()));
    return true;
}

void FileSyncPlugin::ShowSyncManagementDialog()
{
    if (!m_syncDialog) {
        qmlRegisterType<FileSyncEntry>("org.openpilot", 1, 0, "Sync");
        FileSyncManager *filesyncManager = new FileSyncManager();
        m_syncDialog = new QQuickView();
        m_syncDialog->setIcon(QIcon(":/core/images/openpilot_logo_32.png"));
        m_syncDialog->setTitle(tr("File Sync Manager"));
        m_syncDialog->rootContext()->setContextProperty("filesyncManager", filesyncManager);
        m_syncDialog->rootContext()->setContextProperty("syncDialog", m_syncDialog);
        m_syncDialog->setResizeMode(QQuickView::SizeRootObjectToView);
        m_syncDialog->setSource(QUrl("qrc:/filesync/FileSyncDialog.qml"));
        m_syncDialog->setModality(Qt::ApplicationModal);
        connect(m_syncDialog, SIGNAL(destroyed()), this, SLOT(SyncManagementDialogClosed()));
    }
    m_syncDialog->show();
}

void FileSyncPlugin::SyncManagementDialogClosed()
{
    if (m_syncDialog) {
        m_syncDialog->deleteLater();
        m_syncDialog = 0;
    }
}

void FileSyncPlugin::extensionsInitialized()
{}

void FileSyncPlugin::shutdown()
{
    if (m_syncDialog) {
        m_syncDialog->close();
        SyncManagementDialogClosed();
    }
}
