/**
 ******************************************************************************
 *
 * @file       usagetrackerplugin.h
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
#ifndef USAGETRACKERPLUGIN_H
#define USAGETRACKERPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <uavtalk/telemetrymanager.h>

class UsageTrackerPlugin : public ExtensionSystem::IPlugin {
    Q_OBJECT
                                                   Q_PLUGIN_METADATA(IID "OpenPilot.UsageTracker")
public:
    UsageTrackerPlugin();
    ~UsageTrackerPlugin();

    void extensionsInitialized();
    bool initialize(const QStringList & arguments, QString *errorString);
    void shutdown();

private slots:
    void onAutopilotConnect();
    void trackUsage();
    void collectUsageParameters(QMap<QString, QString> &parameters);

private:
    TelemetryManager *m_telemetryManager;
    QString getUAVFieldValue(UAVObjectManager *objManager, QString objectName, QString fieldName, int index = 0) const;
    QString getQueryHash(QString source) const;
};

#endif // USAGETRACKERPLUGIN_H
