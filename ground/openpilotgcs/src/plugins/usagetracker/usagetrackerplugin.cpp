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
#include <QtNetwork/QNetworkReply>
#include <extensionsystem/pluginmanager.h>
#include <QCheckBox>
#include <QDebug>
#include <QMessageBox>
#include <uavobjectutil/devicedescriptorstruct.h>
#include <uavobjectutil/uavobjectutilmanager.h>
#include "version_info/version_info.h"
#include "coreplugin/icore.h"
#include <uavtalk/telemetrymanager.h>

UsageTrackerPlugin::UsageTrackerPlugin() :
    m_telemetryManager(NULL)
{}

UsageTrackerPlugin::~UsageTrackerPlugin()
{}

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
    Core::Internal::GeneralSettings *settings = getGeneralSettings();

    if (settings->collectUsageData()) {
        if (settings->showUsageDataDisclaimer()) {
            QMessageBox message;
            message.setWindowTitle(tr("Usage feedback"));
            message.setIcon(QMessageBox::Information);
            message.addButton(tr("Yes, count me in"), QMessageBox::AcceptRole);
            message.addButton(tr("No, I will not help"), QMessageBox::RejectRole);
            message.setText(tr("Openpilot GCS has a function to collect limited anonymous information about "
                               "the usage of the application itself and the OpenPilot hardware connected to it.<p>"
                               "The intention is to not include anything that can be considered sensitive "
                               "or a threat to the users integrity. The collected information will be sent "
                               "using a secure protocol to an OpenPilot web service and stored in a database "
                               "for later analysis and statistical purposes.<br>"
                               "No information will be sold or given to any third party. The sole purpose is "
                               "to collect statistics about the usage of our software and hardware to enable us "
                               "to make things better for you.<p>"
                               "The following things are collected:<ul>"
                               "<li>Bootloader version</li>"
                               "<li>Firmware version, tag and git hash</li>"
                               "<li>OP Hardware type, revision and mcu serial number</li>"
                               "<li>Selected configuration parameters</li>"
                               "<li>GCS version</li>"
                               "<li>Operating system version and architecture</li>"
                               "<li>Current local time</li></ul>"
                               "The information is collected only at the time when a board is connecting to GCS.<p>"
                               "It is possible to enable or disable this functionality in the general "
                               "settings part of the options for the GCS application at any time.<p>"
                               "We need your help, with your feedback we know where to improve things and what "
                               "platforms are in use. This is a community project that depends on people being involved.<br>"
                               "Thank You for helping us making things better and for supporting OpenPilot!"));
            QCheckBox *disclaimerCb = new QCheckBox(tr("&Don't show this message again."));
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
    QMap<QString, QString> parameters;
    collectUsageParameters(parameters);

    QUrlQuery query;
    QMapIterator<QString, QString> iter(parameters);
    while (iter.hasNext()) {
        iter.next();
        query.addQueryItem(iter.key(), iter.value());
    }

    // Add checksum
    QString hash = getQueryHash(query.toString());

    if (shouldSend(hash)) {
        query.addQueryItem("hash", hash);

        QUrl url("https://www.openpilot.org/opver?" + query.toString(QUrl::FullyEncoded));

        QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager();

        // This will delete the network access manager instance when we're done
        connect(networkAccessManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(onFinished(QNetworkReply *)));
        connect(networkAccessManager, SIGNAL(finished(QNetworkReply *)), networkAccessManager, SLOT(deleteLater()));

        qDebug() << "Sending usage tracking as:" << url.toEncoded(QUrl::FullyEncoded);
        networkAccessManager->get(QNetworkRequest(QUrl(url.toEncoded(QUrl::FullyEncoded))));
    }
}

void UsageTrackerPlugin::collectUsageParameters(QMap<QString, QString> &parameters)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager *utilMngr     = pm->getObject<UAVObjectUtilManager>();

    QByteArray description = utilMngr->getBoardDescription();
    deviceDescriptorStruct devDesc;

    if (UAVObjectUtilManager::descriptionToStructure(description, devDesc)) {
        int boardModel = utilMngr->getBoardModel();
        parameters["board_type"]   = "0x" + QString::number(boardModel, 16).toLower();
        parameters["board_serial"] = utilMngr->getBoardCPUSerial().toHex();
        parameters["bl_version"]   = QString::number(utilMngr->getBootloaderRevision());
        parameters["fw_tag"] = devDesc.gitTag;
        parameters["fw_hash"] = devDesc.gitHash;
        parameters["os_version"]   = QSysInfo::prettyProductName() + " " + QSysInfo::currentCpuArchitecture();
        parameters["os_threads"]   = QString::number(QThread::idealThreadCount());
        parameters["os_timezone"]  = QTimeZone::systemTimeZoneId();
        parameters["gcs_version"]  = VersionInfo::revision();

        // Configuration parameters
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

        parameters["conf_receiver"] = getUAVFieldValue(objManager, "ManualControlSettings", "ChannelGroups", 0);
        parameters["conf_vehicle"]  = getUAVFieldValue(objManager, "SystemSettings", "AirframeType");

        // Revolution family
        parameters["conf_rport"]  = getUAVFieldValue(objManager, "HwSettings", "RM_RcvrPort");
        parameters["conf_mport"]  = getUAVFieldValue(objManager, "HwSettings", "RM_MainPort");
        parameters["conf_fport"]  = getUAVFieldValue(objManager, "HwSettings", "RM_FlexiPort");
        parameters["conf_fusion"] = getUAVFieldValue(objManager, "RevoSettings", "FusionAlgorithm");

        parameters["conf_uport"]    = getUAVFieldValue(objManager, "HwSettings", "USB_HIDPort");
        parameters["conf_vport"]    = getUAVFieldValue(objManager, "HwSettings", "USB_VCPPort");

        parameters["conf_rotation"] = QString("[%1:%2:%3]")
                                      .arg(getUAVFieldValue(objManager, "AttitudeSettings", "BoardRotation", 0))
                                      .arg(getUAVFieldValue(objManager, "AttitudeSettings", "BoardRotation", 1))
                                      .arg(getUAVFieldValue(objManager, "AttitudeSettings", "BoardRotation", 2));
        parameters["conf_pidr"] = QString("[%1:%2:%3:%4][%5:%6:%7:%8][%9:%10:%11:%12]")
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "RollRatePID", 0))
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "RollRatePID", 1))
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "RollRatePID", 2))
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "RollRatePID", 3))
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "PitchRatePID", 0))
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "PitchRatePID", 1))
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "PitchRatePID", 2))
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "PitchRatePID", 3))
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "YawRatePID", 0))
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "YawRatePID", 1))
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "YawRatePID", 2))
                                  .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "YawRatePID", 3));
        parameters["conf_pia"] = QString("[%1:%2:%3][%4:%5:%6][%7:%8:%9]")
                                 .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "RollPI", 0))
                                 .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "RollPI", 1))
                                 .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "RollPI", 2))
                                 .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "PitchPI", 0))
                                 .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "PitchPI", 1))
                                 .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "PitchPI", 2))
                                 .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "YawPI", 0))
                                 .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "YawPI", 1))
                                 .arg(getUAVFieldValue(objManager, "StabilizationSettingsBank1", "YawPI", 2));

        parameters["conf_tps"]     = getUAVFieldValue(objManager, "StabilizationSettingsBank1", "EnableThrustPIDScaling");
        parameters["conf_piro"]    = getUAVFieldValue(objManager, "StabilizationSettingsBank1", "EnablePiroComp");

        parameters["conf_fmcount"] = getUAVFieldValue(objManager, "ManualControlSettings", "FlightModeNumber");
        parameters["conf_fmodes"]  = QString("[%1:%2:%3]").arg(getUAVFieldValue(objManager, "FlightModeSettings", "FlightModePosition", 0))
                                     .arg(getUAVFieldValue(objManager, "FlightModeSettings", "FlightModePosition", 1))
                                     .arg(getUAVFieldValue(objManager, "FlightModeSettings", "FlightModePosition", 2));
    }
}

void UsageTrackerPlugin::onFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        getGeneralSettings()->setLastUsageHash(m_lastHash);
        qDebug() << "Updated last usage hash to:" << m_lastHash;
    } else {
        qDebug() << "Usage tracking failed with:" << reply->errorString();
    }
}

QString UsageTrackerPlugin::getUAVFieldValue(UAVObjectManager *objManager, QString objectName, QString fieldName, int index) const
{
    UAVObject *object = objManager->getObject(objectName);

    if (object != NULL) {
        UAVObjectField *field = object->getField(fieldName);
        if (field != NULL) {
            return field->getValue(index).toString();
        }
    }
    return tr("Unknown");
}

QString UsageTrackerPlugin::getQueryHash(QString source) const
{
    source += "OpenPilot Fuck Yeah!";
    return QString(QCryptographicHash::hash(QByteArray(source.toStdString().c_str()), QCryptographicHash::Md5).toHex());
}

Core::Internal::GeneralSettings *UsageTrackerPlugin::getGeneralSettings() const
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings *settings = pm->getObject<Core::Internal::GeneralSettings>();

    return settings;
}

bool UsageTrackerPlugin::shouldSend(const QString &hash)
{
    if (getGeneralSettings()->lastUsageHash() == hash) {
        return false;
    } else {
        m_lastHash = hash;
        return true;
    }
}
