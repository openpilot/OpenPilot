/**
 ******************************************************************************
 *
 * @file       usagetrackerplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UsageTrackerPlugin Usage Tracker Plugin
 * @{
 * @brief A plugin tracking GCS usage
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
#include "usagetrackerplugin.h"
#include <QtPlugin>
#include <QStringList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <extensionsystem/pluginmanager.h>
#include <QDebug>
#include <uavobjectutil/devicedescriptorstruct.h>
#include <uavobjectutil/uavobjectutilmanager.h>
#include <coreplugin/generalsettings.h>
#include "version_info/version_info.h"

UsageTrackerPlugin::UsageTrackerPlugin() :
    m_telemetryManager(NULL)
{
}

UsageTrackerPlugin::~UsageTrackerPlugin()
{
}

bool UsageTrackerPlugin::initialize(const QStringList & args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);

    return true;
}

void UsageTrackerPlugin::extensionsInitialized()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_telemetryManager = pm->getObject<TelemetryManager>();
    connect(m_telemetryManager, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
}

void UsageTrackerPlugin::shutdown()
{
    if (m_telemetryManager != NULL) {
        disconnect(m_telemetryManager, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
    }
}

void UsageTrackerPlugin::onAutopilotConnect()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings *settings = pm->getObject<Core::Internal::GeneralSettings>();
    if (settings->collectUsageData()) {
        QTimer::singleShot(1000, this, SLOT(trackUsage()));
    }
}

void UsageTrackerPlugin::trackUsage()
{
    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager();

    // This will delete the network access manager instance when we're done
    connect(networkAccessManager, SIGNAL(finished(QNetworkReply *)), networkAccessManager, SLOT(deleteLater()));

    QMap<QString, QString> parameters;
    collectUsageParameters(parameters);

    QUrlQuery query;
    QMapIterator<QString, QString> iter(parameters);
    while (iter.hasNext()) {
        iter.next();
        query.addQueryItem(iter.key(), QUrl::toPercentEncoding(iter.value()));
    }
    QUrl url("https://www.openpilot.org/opver?" + query.toString(QUrl::FullyEncoded));
    qDebug() << "Sending usage tracking as:" << url.toEncoded(QUrl::FullyEncoded);
    networkAccessManager->get(QNetworkRequest(QUrl(url.toEncoded(QUrl::FullyEncoded))));
}

void UsageTrackerPlugin::collectUsageParameters(QMap<QString, QString> &parameters)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager *utilMngr     = pm->getObject<UAVObjectUtilManager>();

    QByteArray description = utilMngr->getBoardDescription();
    deviceDescriptorStruct devDesc;
    if (UAVObjectUtilManager::descriptionToStructure(description, devDesc)) {
        parameters["hi0"] = "0x" + QString::number(utilMngr->getBoardModel(), 16).toLower();
        parameters["hi1"] = utilMngr->getBoardCPUSerial().toHex();
        parameters["bi0"] = QString::number(utilMngr->getBootloaderRevision());
        parameters["fi0"] = devDesc.gitTag;
        parameters["fi1"] = devDesc.gitHash;
        parameters["oi0"] = QSysInfo::prettyProductName() + " " + QSysInfo::currentCpuArchitecture();
        parameters["si0"] = VersionInfo::revision();
        parameters["ti0"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    }
}
