/**
 ******************************************************************************
 *
 * @file       notifyplugin.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   notifyplugin
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
#ifndef SOUNDNOTIFYPLUGIN_H
#define SOUNDNOTIFYPLUGIN_H

#include <extensionsystem/iplugin.h> 
#include <coreplugin/iconfigurableplugin.h>
#include "uavtalk/telemetrymanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "notificationitem.h"

#include <QSettings>
#include <phonon/MediaObject>
#include <phonon/Path>
#include <phonon/AudioOutput>
#include <phonon/Global>

class NotifyPluginOptionsPage;

typedef struct {
	Phonon::MediaObject* mo;
	NotificationItem* notify;
	bool firstPlay;
} PhononObject, *pPhononObject;


class SoundNotifyPlugin : public Core::IConfigurablePlugin
{
    Q_OBJECT
public:
    SoundNotifyPlugin();
    ~SoundNotifyPlugin();

    void extensionsInitialized();
    bool initialize(const QStringList & arguments, QString * errorString);
    void readConfig( QSettings* qSettings, Core::UAVConfigInfo *configInfo);
    void saveConfig( QSettings* qSettings, Core::UAVConfigInfo *configInfo);
    void shutdown();


    QList<NotificationItem*> getListNotifications() { return _notificationList; }
    NotificationItem* getCurrentNotification(){ return &currentNotification;}

    bool getEnableSound() const { return enableSound; }
    void setEnableSound(bool value) {enableSound = value; }

private:
    Q_DISABLE_COPY(SoundNotifyPlugin)

    bool playNotification(NotificationItem* notification);
    void checkNotificationRule(NotificationItem* notification, UAVObject* object);

private slots:

    void onTelemetryManagerAdded(QObject* obj);
    void onAutopilotDisconnect();
    void connectNotifications();
    void updateNotificationList(QList<NotificationItem*> list);
    void resetNotification(void);
    void on_arrived_Notification(UAVObject *object);
    void on_timerRepeated_Notification(void);
    void on_expiredTimer_Notification(void);
    void stateChanged(Phonon::State newstate, Phonon::State oldstate);

private:
    bool enableSound;

    QList<UAVDataObject*> lstNotifiedUAVObjects;
    QList<NotificationItem*> _notificationList;
    QList<NotificationItem*> _pendingNotifications;
    QList<NotificationItem*> _toRemoveNotifications;

    NotificationItem currentNotification;
    NotificationItem* _nowPlayingNotification;

    PhononObject phonon;
    NotifyPluginOptionsPage* mop;
    TelemetryManager* telMngr;
}; 

#endif // SOUNDNOTIFYPLUGIN_H
