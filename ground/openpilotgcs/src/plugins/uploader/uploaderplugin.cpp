/**
 ******************************************************************************
 *
 * @file       uploaderplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup YModemUploader YModem Serial Uploader Plugin
 * @{
 * @brief The YModem protocol serial uploader plugin
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
#include "uploaderplugin.h"
#include "uploadergadgetfactory.h"

#include "version_info/version_info.h"
#include "devicedescriptorstruct.h"
#include "uavobjectutilmanager.h"

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <uavtalk/telemetrymanager.h>

#include <QStringList>
#include <QErrorMessage>
#include <QWidget>
#include <QMainWindow>

UploaderPlugin::UploaderPlugin() : errorMsg(0)
{
    // Do nothing
}

UploaderPlugin::~UploaderPlugin()
{
    // Do nothing
}

bool UploaderPlugin::initialize(const QStringList & args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);

    mf = new UploaderGadgetFactory(this);
    addAutoReleasedObject(mf);

    // Listen to autopilot connection events
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    TelemetryManager *telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(versionMatchCheck()));

    return true;
}

void UploaderPlugin::extensionsInitialized()
{
    // Do nothing
}

void UploaderPlugin::shutdown()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    TelemetryManager *telMngr = pm->getObject<TelemetryManager>();
    disconnect(telMngr, SIGNAL(connected()), this, SLOT(versionMatchCheck()));
}

void UploaderPlugin::versionMatchCheck()
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
        QString gcsVersion = gcsGitDate + " (" + gcsGitHash + "-" + gcsUavoHashStr.left(8) + ")";
        QString fwVersion  = boardDescription.gitDate + " (" + boardDescription.gitHash + "-" + fwUavoHashStr.left(8) + ")";

        QString warning    = QString(tr(
                                         "GCS and firmware versions of the UAV objects set do not match which can cause configuration problems. "
                                         "GCS version: %1 Firmware version: %2.")).arg(gcsVersion).arg(fwVersion);
        if (!errorMsg) {
            errorMsg = new QErrorMessage(Core::ICore::instance()->mainWindow());
        }
        errorMsg->showMessage(warning);
    }
}
