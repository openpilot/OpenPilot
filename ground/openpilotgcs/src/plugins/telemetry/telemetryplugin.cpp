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

#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "coreplugin/icore.h"
#include "coreplugin/connectionmanager.h"

#include <QDebug>
#include <QtPlugin>
#include <QStringList>

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/iuavgadget.h>

TelemetryPlugin::TelemetryPlugin()
{
}

TelemetryPlugin::~TelemetryPlugin()
{
//    Core::ICore::instance()->saveSettings(this);
}

bool TelemetryPlugin::initialize(const QStringList & args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);

    MonitorGadgetFactory *mf = new MonitorGadgetFactory(this);
    addAutoReleasedObject(mf);

    //  mop = new TelemetryPluginOptionsPage(this);
    //addAutoReleasedObject(mop);

    // TODO not so good... g is probalby leaked...
    MonitorWidget *w = mf->createMonitorWidget(NULL);
    w->setMaximumWidth(180);

    //
    //setAlignment(Qt::AlignCenter);

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
//    connect(cm, SIGNAL(deviceConnected(QIODevice *)), w, SLOT(telemetryConnected()));
//    connect(cm, SIGNAL(deviceDisconnected()), w, SLOT(telemetryDisconnected()));

    cm->addWidget(w);

    return true;
}

void TelemetryPlugin::extensionsInitialized()
{
//    Core::ICore::instance()->readSettings(this);

    //ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();

//    connect(pm, SIGNAL(objectAdded(QObject *)), this, SLOT(onTelemetryManagerAdded(QObject *)));
//    _toRemoveNotifications.clear();
//    connectNotifications();
}

//void TelemetryPlugin::saveConfig(QSettings *settings, UAVConfigInfo *configInfo)
//{
//    configInfo->setVersion(VERSION);
//
//    settings->beginWriteArray("Current");
//    settings->setArrayIndex(0);
//    currentNotification.saveState(settings);
//    settings->endArray();
//
//    settings->beginGroup("listNotifies");
//    settings->remove("");
//    settings->endGroup();
//
//    settings->beginWriteArray("listNotifies");
//    for (int i = 0; i < _notificationList.size(); i++) {
//        settings->setArrayIndex(i);
//        _notificationList.at(i)->saveState(settings);
//    }
//    settings->endArray();
//    settings->setValue(QLatin1String("Enable"), enable);
//}

//void TelemetryPlugin::readConfig(QSettings *settings, UAVConfigInfo * /* configInfo */)
//{
//    // Just for migration to the new format.
//    // Q_ASSERT(configInfo->version() == UAVConfigVersion());
//
//    settings->beginReadArray("Current");
//    settings->setArrayIndex(0);
//    currentNotification.restoreState(settings);
//    settings->endArray();
//
//    // read list of notifications from settings
//    int size = settings->beginReadArray("listNotifies");
//    for (int i = 0; i < size; ++i) {
//        settings->setArrayIndex(i);
//        NotificationItem *notification = new NotificationItem;
//        notification->restoreState(settings);
//        _notificationList.append(notification);
//    }
//    settings->endArray();
//    setEnable(settings->value(QLatin1String("Enable"), 0).toBool());
//}

//void TelemetryPlugin::onTelemetryManagerAdded(QObject *obj)
//{
//    telMngr = qobject_cast<TelemetryManager *>(obj);
//    if (telMngr) {
//        connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));
//    }
//}

void TelemetryPlugin::shutdown()
{
    // Do nothing
}

//void TelemetryPlugin::onAutopilotDisconnect()
//{
//    connectNotifications();
//}

///*!
//    clear any telemetry timers from previous flight;
//    reset will be perform on start of option page
// */
//void TelemetryPlugin::resetNotification(void)
//{
//    // first, reject empty args and unknown fields.
//    foreach(NotificationItem * ntf, _notificationList) {
//        ntf->disposeTimer();
//        disconnect(ntf->getTimer(), SIGNAL(timeout()), this, SLOT(on_timerRepeated_Notification()));
//        ntf->disposeExpireTimer();
//        disconnect(ntf->getExpireTimer(), SIGNAL(timeout()), this, SLOT(on_timerRepeated_Notification()));
//    }
//}

//void TelemetryPlugin::connectNotifications()
//{
//    foreach(UAVDataObject * obj, lstNotifiedUAVObjects) {
//        if (obj != NULL) {
//            disconnect(obj, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(on_arrived_Notification(UAVObject *)));
//        }
//    }
//    if (phonon.mo != NULL) {
//        delete phonon.mo;
//        phonon.mo = NULL;
//    }
//
//    if (!enable) {
//        return;
//    }
//
//    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
//    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
//
//    lstNotifiedUAVObjects.clear();
//    _pendingNotifications.clear();
//    _notificationList.append(_toRemoveNotifications);
//    _toRemoveNotifications.clear();
//
//    // first, reject empty args and unknown fields.
//    foreach(NotificationItem * telemetry, _notificationList) {
//        telemetry->_isPlayed    = false;
//        telemetry->isNowPlaying = false;
//
//        if (telemetry->mute()) {
//            continue;
//        }
//        // check is all sounds presented for notification,
//        // if not - we must not subscribe to it at all
//        if (telemetry->toList().isEmpty()) {
//            continue;
//        }
//
//        UAVDataObject *obj = dynamic_cast<UAVDataObject *>(objManager->getObject(telemetry->getDataObject()));
//        if (obj != NULL) {
//            if (!lstNotifiedUAVObjects.contains(obj)) {
//                lstNotifiedUAVObjects.append(obj);
//
//                connect(obj, SIGNAL(objectUpdated(UAVObject *)),
//                        this, SLOT(on_arrived_Notification(UAVObject *)),
//                        Qt::QueuedConnection);
//            }
//        } else {
//            qTelemetryDebug() << "Error: Object is unknown (" << telemetry->getDataObject() << ").";
//        }
//    }
//
//    if (_notificationList.isEmpty()) {
//        return;
//    }
//    // set notification message to current event
//    phonon.mo = Phonon::createPlayer(Phonon::NotificationCategory);
//    phonon.mo->clearQueue();
//    phonon.firstPlay = true;
//    QList<Phonon::AudioOutputDevice> audioOutputDevices =
//        Phonon::BackendCapabilities::availableAudioOutputDevices();
//    foreach(Phonon::AudioOutputDevice dev, audioOutputDevices) {
//        qTelemetryDebug() << "Telemetry: Audio Output device: " << dev.name() << " - " << dev.description();
//    }
//    connect(phonon.mo, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
//            this, SLOT(stateChanged(Phonon::State, Phonon::State)));
//}
