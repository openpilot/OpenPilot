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
#include <QtQuick>

#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icore.h>
#include <QKeySequence>
#include <coreplugin/modemanager.h>
#include "flightlogmanager.h"
#include "uavobject.h"

FlightLogPlugin::FlightLogPlugin() : m_logDialog(0)
{}

FlightLogPlugin::~FlightLogPlugin()
{
    shutdown();
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
    if (!m_logDialog) {
        qmlRegisterType<ExtendedDebugLogEntry>("org.openpilot", 1, 0, "DebugLogEntry");
        qmlRegisterType<UAVOLogSettingsWrapper>("org.openpilot", 1, 0, "UAVOLogSettingsWrapper");
        FlightLogManager *flightLogManager = new FlightLogManager();
        m_logDialog = new QQuickView();
        m_logDialog->setIcon(QIcon(":/core/images/openpilot_logo_32.png"));
        m_logDialog->setTitle(tr("Manage flight side logs"));
        m_logDialog->rootContext()->setContextProperty("logStatus", flightLogManager->flightLogStatus());
        m_logDialog->rootContext()->setContextProperty("logControl", flightLogManager->flightLogControl());
        m_logDialog->rootContext()->setContextProperty("logSettings", flightLogManager->flightLogSettings());
        m_logDialog->rootContext()->setContextProperty("logManager", flightLogManager);
        m_logDialog->rootContext()->setContextProperty("logDialog", m_logDialog);
        m_logDialog->setResizeMode(QQuickView::SizeRootObjectToView);
        m_logDialog->setSource(QUrl("qrc:/flightlog/FlightLogDialog.qml"));
        m_logDialog->setFlags(Qt::Dialog);
        m_logDialog->setModality(Qt::ApplicationModal);
        connect(m_logDialog, SIGNAL(destroyed()), this, SLOT(LogManagementDialogClosed()));
    }
    m_logDialog->show();
}

void FlightLogPlugin::LogManagementDialogClosed()
{
    if (m_logDialog) {
        m_logDialog->deleteLater();
        m_logDialog = 0;
    }
}

void FlightLogPlugin::extensionsInitialized()
{}

void FlightLogPlugin::shutdown()
{
    if (m_logDialog) {
        m_logDialog->close();
        LogManagementDialogClosed();
    }
}
