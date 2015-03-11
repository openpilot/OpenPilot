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
#include <QCheckBox>
#include <QDebug>
#include <QMessageBox>
#include <uavobjectutil/devicedescriptorstruct.h>
#include <uavobjectutil/uavobjectutilmanager.h>
#include <coreplugin/generalsettings.h>
#include "version_info/version_info.h"
#include "coreplugin/icore.h"
#include "qmainwindow.h"

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
        if (settings->showUsageDataDisclaimer()) {
            QMessageBox message;
            message.setWindowTitle(tr("Usage feedback"));
            message.setIcon(QMessageBox::Information);
            message.addButton(tr("Yes, count me in"), QMessageBox::AcceptRole);
            message.addButton(tr("No, I will not help"), QMessageBox::RejectRole);
            message.setText(tr("Openpilot GCS has a function to collect limited anonymous information about "
                                "the usage of the application itself and the OpenPilot hardware connected to it.\n\n"
                                "The intention is to not include anything that can be considered sensitive "
                                "or a threat to the users integrity. The collected information will be sent "
                                "using a secure protocol to an OpenPilot web service and stored in a database "
                                "for later analysis and statistical purposes.\n"
                                "No information will be sold or given to any third party. The sole purpose is "
                                "to collect statistics about the usage of our software and hardware to enable us "
                                "to make things better for you.\n\n"
                                "The following things are collected:\n"
                                "- Bootloader version\n"
                                "- Firmware version, tag and git hash\n"
                                "- OP Hardware type, revision and mcu serial number\n"
                                "- GCS version\n"
                                "- Operating system version and architecture\n"
                                "- Current local time\n"
                                "The information is collected only at the time when a board is connecting to GCS.\n\n"
                                "It is possible to enable or disable this functionality in the general "
                                "settings part of the options for the GCS application at any time.\n\n"
                                "We need your help, with your feedback we know where to improve things and what "
                                "platforms are in use. This is a community project that depends on people being involved.\n"
                                "Thank You for helping us making things better and for supporting OpenPilot!"));
            QCheckBox* disclaimerCb = new QCheckBox(tr("&Don't show this message again."));
            disclaimerCb->setChecked(true);
            message.setCheckBox(disclaimerCb);
            if (message.exec() != QMessageBox::AcceptRole) {
                settings->setCollectUsageData(false);
                settings->setShowUsageDataDisclaimer(!message.checkBox()->isChecked());
                return;
            } else {
                settings->setCollectUsageData(true);
                settings->setShowUsageDataDisclaimer(!message.checkBox()->isChecked());
            }
        }
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
