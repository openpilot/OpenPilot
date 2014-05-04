/**
 ******************************************************************************
 *
 * @file       telemetryplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   telemetryplugin
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

#include "telemetryplugin.h"
#include "monitorgadgetfactory.h"

#include "version_info/version_info.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "uavobjectutilmanager.h"

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/iuavgadget.h>
#include <coreplugin/connectionmanager.h>
#include <uavtalk/telemetrymanager.h>

#include <QDebug>
#include <QStringList>
#include <QWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QCheckBox>

TelemetryPlugin::TelemetryPlugin() : firmwareWarningMessageBox(0)
{}

TelemetryPlugin::~TelemetryPlugin()
{
// Core::ICore::instance()->saveSettings(this);
}

bool TelemetryPlugin::initialize(const QStringList & args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);

    MonitorGadgetFactory *mf = new MonitorGadgetFactory(this);
    addAutoReleasedObject(mf);

    // mop = new TelemetryPluginOptionsPage(this);
    // addAutoReleasedObject(mop);

    // TODO not so good... g is probalby leaked...
    MonitorWidget *w = mf->createMonitorWidget(NULL);
    w->setMaximumWidth(180);

    //
    // setAlignment(Qt::AlignCenter);

    // no border
    w->setFrameStyle(QFrame::NoFrame);
    w->setWindowFlags(Qt::FramelessWindowHint);

    // set svg background translucent
    w->setStyleSheet("background:transparent;");
    // set widget background translucent
    w->setAttribute(Qt::WA_TranslucentBackground);

    w->setBackgroundBrush(Qt::NoBrush);

    // add monitor widget to connection manager
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();

    cm->addWidget(w);

    // Listen to autopilot connection events
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    TelemetryManager *telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(versionMatchCheck()));

    return true;
}

void TelemetryPlugin::extensionsInitialized()
{
    // Do nothing
}

void TelemetryPlugin::shutdown()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    TelemetryManager *telMngr = pm->getObject<TelemetryManager>();

    disconnect(telMngr, SIGNAL(connected()), this, SLOT(versionMatchCheck()));

    if (firmwareWarningMessageBox) {
        delete firmwareWarningMessageBox;
    }
}

void TelemetryPlugin::versionMatchCheck()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager *utilMngr     = pm->getObject<UAVObjectUtilManager>();
    deviceDescriptorStruct boardDescription = utilMngr->getBoardDescriptionStruct();

    QString uavoHash = VersionInfo::uavoHashArray();

    uavoHash.chop(2);
    uavoHash.remove(0, 2);
    uavoHash = uavoHash.trimmed();

    QByteArray uavoHashArray;
    bool ok;
    foreach(QString str, uavoHash.split(",")) {
        uavoHashArray.append(str.toInt(&ok, 16));
    }

    QByteArray fwVersion = boardDescription.uavoHash;
    if (fwVersion != uavoHashArray) {
        QString gcsDescription = VersionInfo::revision();
        QString gcsGitHash     = gcsDescription.mid(gcsDescription.indexOf(":") + 1, 8);
        gcsGitHash.remove(QRegExp("^[0]*"));
        QString gcsGitDate     = gcsDescription.mid(gcsDescription.indexOf(" ") + 1, 14);

        QString gcsUavoHashStr;
        QString fwUavoHashStr;
        foreach(char i, fwVersion) {
            fwUavoHashStr.append(QString::number(i, 16).right(2));
        }
        foreach(char i, uavoHashArray) {
            gcsUavoHashStr.append(QString::number(i, 16).right(2));
        }
        QString versionFormat = "%1 (%2-%3)";
        QString gcsVersion    = versionFormat.arg(gcsGitDate, gcsGitHash, gcsUavoHashStr.left(8));
        QString fwVersion     = versionFormat.arg(boardDescription.gitDate, boardDescription.gitHash, fwUavoHashStr.left(8));

        if (!firmwareWarningMessageBox) {
            firmwareWarningMessageBox = new QMessageBox(Core::ICore::instance()->mainWindow());
            firmwareWarningMessageBox->setWindowModality(Qt::NonModal);
            firmwareWarningMessageBox->setWindowTitle(tr("Firmware Version Mismatch!"));
            firmwareWarningMessageBox->setIcon(QMessageBox::Warning);
            firmwareWarningMessageBox->setStandardButtons(QMessageBox::Ok);
            firmwareWarningMessageBox->setText(tr("GCS and firmware versions of the UAV objects set do not match which can cause configuration problems."));
            // should we want to re-introduce the checkbox
            // firmwareWarningMessageBox->setCheckBox(new QCheckBox(tr("&Don't show this message again.")));
        }
        QString detailTxt = tr("GCS version: %1").arg(gcsVersion) + "\n" + tr("Firmware version: %1").arg(fwVersion);
        firmwareWarningMessageBox->setDetailedText(detailTxt);
        firmwareWarningMessageBox->show();
    }
}
