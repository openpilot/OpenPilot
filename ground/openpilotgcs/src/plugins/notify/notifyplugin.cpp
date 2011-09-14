/**
 ******************************************************************************
 *
 * @file       notifyplugin.cpp
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

#include "notifyplugin.h"
#include "notificationitem.h"
#include "notifypluginoptionspage.h"
#include <coreplugin/icore.h>
#include <QDebug>
#include <QtPlugin>
#include <QStringList>

#include <extensionsystem/pluginmanager.h>

#include <iostream>
#include "qxttimer.h"
#include "backendcapabilities.h"

static const QString VERSION = "1.0.0";

//#define DEBUG_NOTIFIES

#ifdef DEBUG_NOTIFIES
QDebug qNotifyDebug()
#endif
#ifndef DEBUG_NOTIFIES
QNoDebug qNotifyDebug()
#endif
{
#ifdef DEBUG_NOTIFIES
    return qDebug();
#endif
    return QNoDebug();
}

SoundNotifyPlugin::SoundNotifyPlugin()
{
    phonon.mo = NULL;
    configured = false;
   // Do nothing
}

SoundNotifyPlugin::~SoundNotifyPlugin()
{
    Core::ICore::instance()->saveSettings(this);
    if (phonon.mo != NULL)
        delete phonon.mo;
   // Do nothing
}

bool SoundNotifyPlugin::initialize(const QStringList& args, QString *errMsg)
{ 
   Q_UNUSED(args); 
   Q_UNUSED(errMsg); 

   mop = new NotifyPluginOptionsPage(this);
   addAutoReleasedObject(mop);

   return true; 
} 

void SoundNotifyPlugin::extensionsInitialized()
{ 
        Core::ICore::instance()->readSettings(this);
        if ( !configured ){
            readConfig_0_0_0();
        }

	ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
	connect(pm, SIGNAL(objectAdded(QObject*)), this, SLOT(onTelemetryManagerAdded(QObject*)));
	removedNotifies.clear();
	connectNotifications();
} 

void SoundNotifyPlugin::saveConfig( QSettings* settings, UAVConfigInfo *configInfo){

    configInfo->setVersion(VERSION);

    settings->beginWriteArray("Current");
    settings->setArrayIndex(0);
    currentNotification.saveState(settings);
    settings->endArray();

    settings->beginGroup("listNotifies");
    settings->remove("");
    settings->endGroup();

    settings->beginWriteArray("listNotifies");
    for (int i = 0; i < lstNotifications.size(); i++) {
            settings->setArrayIndex(i);
            lstNotifications.at(i)->saveState(settings);
    }
    settings->endArray();
    settings->setValue(QLatin1String("EnableSound"), enableSound);

}

void SoundNotifyPlugin::readConfig( QSettings* settings, UAVConfigInfo *configInfo){

    if ( configInfo->version() == UAVConfigVersion() ){
        // Just for migration to the new format.
        configured = false;
        return;
    }

    settings->beginReadArray("Current");
    settings->setArrayIndex(0);
    currentNotification.restoreState(settings);
    settings->endArray();

    // read list of notifications from settings
    int size = settings->beginReadArray("listNotifies");
    for (int i = 0; i < size; ++i) {
             settings->setArrayIndex(i);
			 NotificationItem* notification = new NotificationItem;
             notification->restoreState(settings);
             lstNotifications.append(notification);
    }
    settings->endArray();
    setEnableSound(settings->value(QLatin1String("EnableSound"),0).toBool());

    configured = true;
}

void SoundNotifyPlugin::readConfig_0_0_0(){

       settings = Core::ICore::instance()->settings();
       settings->beginGroup(QLatin1String("NotifyPlugin"));

       settings->beginReadArray("Current");
       settings->setArrayIndex(0);
       currentNotification.restoreState(settings);
       settings->endArray();

       // read list of notifications from settings
       int size = settings->beginReadArray("listNotifies");
       for (int i = 0; i < size; ++i) {
                settings->setArrayIndex(i);
				NotificationItem* notification = new NotificationItem;
                notification->restoreState(settings);
                lstNotifications.append(notification);
       }
       settings->endArray();
       setEnableSound(settings->value(QLatin1String("EnableSound"),0).toBool());
       settings->endGroup();
	
	ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
	connect(pm, SIGNAL(objectAdded(QObject*)), this, SLOT(onTelemetryManagerAdded(QObject*)));
	removedNotifies.clear();
	connectNotifications();
	
	configured = true;
     }

void SoundNotifyPlugin::onTelemetryManagerAdded(QObject* obj)
{
	telMngr = qobject_cast<TelemetryManager *>(obj);
	if(telMngr)
		connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));
}

void SoundNotifyPlugin::shutdown()
{ 
   // Do nothing 
}

void SoundNotifyPlugin::onAutopilotDisconnect()
{
	connectNotifications();
}

/*!
	clear any notify timers from previous flight;
	reset will be perform on start of option page
 */
void SoundNotifyPlugin::resetNotification(void)
{
	//first, reject empty args and unknown fields.
	foreach(NotificationItem* notify,lstNotifications) {
		if(notify->timer)
		{
			disconnect(notify->timer, SIGNAL(timeout()), this, SLOT(repeatTimerHandler()));
			notify->timer->stop();
			delete notify->timer;
			notify->timer = NULL;
		}
		if(notify->expireTimer)
		{
			disconnect(notify->expireTimer, SIGNAL(timeout()), this, SLOT(expireTimerHandler()));
			notify->expireTimer->stop();
			delete notify->expireTimer;
			notify->expireTimer = NULL;
		}
	}
}

/*!
	update list of notifies;
	will be perform on OK or APPLY press of option page
 */
void SoundNotifyPlugin::updateNotificationList(QList<NotificationItem*> list)
{
	removedNotifies.clear();
        resetNotification();
	lstNotifications.clear();
	lstNotifications=list;
	connectNotifications();

        Core::ICore::instance()->saveSettings(this);
}

void SoundNotifyPlugin::connectNotifications()
{
	foreach(UAVDataObject* obj,lstNotifiedUAVObjects) {
		if (obj != NULL)
			disconnect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(appendNotification(UAVObject*)));
	}
	if(phonon.mo != NULL) {
		delete phonon.mo;
		phonon.mo = NULL;
	}

	if(!enableSound) return;

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

	lstNotifiedUAVObjects.clear();
	pendingNotifications.clear();
	lstNotifications.append(removedNotifies);
	removedNotifies.clear();

	//first, reject empty args and unknown fields.
	foreach(NotificationItem* notify, lstNotifications) {
		notify->firstStart=true;
		notify->isNowPlaying=false;

		UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(notify->getDataObject()) );
		if (obj != NULL ) {
			if(!lstNotifiedUAVObjects.contains(obj)) {
				lstNotifiedUAVObjects.append(obj);
				connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(appendNotification(UAVObject*)));
			}
		} else
			std::cout << "Error: Object is unknown (" << notify->getDataObject().toStdString() << ")." << std::endl;
	}

    if(lstNotifications.isEmpty()) return;
    // set notification message to current event
    phonon.mo = Phonon::createPlayer(Phonon::NotificationCategory);
    phonon.mo->clearQueue();
    phonon.firstPlay = true;
    QList<Phonon::AudioOutputDevice> audioOutputDevices =
              Phonon::BackendCapabilities::availableAudioOutputDevices();
    foreach(Phonon::AudioOutputDevice dev, audioOutputDevices) {
        qNotifyDebug() << "Notify: Audio Output device: " << dev.name() << " - " << dev.description();
    }
    connect(phonon.mo, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
        this, SLOT(stateChanged(Phonon::State,Phonon::State)));
}

void SoundNotifyPlugin::appendNotification(UAVObject *object)
{
	disconnect(object, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(appendNotification(UAVObject*)));

	foreach(NotificationItem* notification, lstNotifications) {
		if(object->getName()!=notification->getDataObject())
			continue;

		if(nowPlayingConfiguration == notification)
			continue;

		if(notification->getRepeatFlag()!= "Repeat Instantly" &&
		   notification->getRepeatFlag()!= "Repeat Once" && !notification->firstStart)
		   continue;

		checkNotificationRule(notification,object);
	}
	connect(object, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(appendNotification(UAVObject*)));
}


void SoundNotifyPlugin::checkNotificationRule(NotificationItem* notification, UAVObject* object)
{
	bool condition=false;
	double threshold;
	QString direction;
	QString fieldName;
	threshold = notification->getSpinBoxValue();
	direction = notification->getValue();
	fieldName = notification->getObjectField();
	UAVObjectField* field = object->getField(fieldName);
	if (field->getName()!="") {
		double value = field->getDouble();

		switch(direction[0].toAscii())
		{
		case 'E':
			if(value==threshold)
				condition = true;
			break;
		case 'G':
			if(value>threshold)
				condition = true;
			break;
		case 'L':
			if(value<threshold)
				condition = true;
			break;
		}
	}

	if(condition)
	{
		if(!playNotification(notification))
		{
			if(!pendingNotifications.contains(notification))
			{
				if(notification->timer)
					if(notification->timer->isActive())
						notification->timer->stop();
#ifdef DEBUG_NOTIFIES
				qDebug() << "add to pending list - " << notification->parseNotifyMessage();
#endif
				// if audio is busy, start expiration timer
				//ms = (notification->getExpiredTimeout()[in sec])*1000
				//QxtTimer::singleShot(notification->getExpireTimeout()*1000, this, SLOT(expirationTimerHandler(NotificationItem*)), qVariantFromValue(notification));
				pendingNotifications.append(notification);
				if(!notification->expireTimer)
				{
					notification->expireTimer = new QTimer(notification);
					connect(notification->expireTimer, SIGNAL(timeout()), this, SLOT(expireTimerHandler()));
				}
				notification->expireTimer->start(notification->getExpireTimeout()*1000);
			}
		}
	}
}

bool SoundNotifyPlugin::playNotification(NotificationItem* notification)
{
    // Check: race condition, if phonon.mo got deleted don't go further
    if (phonon.mo == NULL)
        return false;

	if(!notification->getEnableFlag()) return true;

#ifdef DEBUG_NOTIFIES
    qDebug() << "Phonon State: " << phonon.mo->state();
#endif
	if((phonon.mo->state()==Phonon::PausedState)
		|| (phonon.mo->state()==Phonon::StoppedState)
		   || phonon.firstPlay)
	{
		// don't fire expire timer
		nowPlayingConfiguration = notification;
		if(notification->expireTimer)
			notification->expireTimer->stop();

		if(notification->getRepeatFlag()=="Repeat Once")
		{
			removedNotifies.append(lstNotifications.takeAt(lstNotifications.indexOf(notification)));
		}
		else {
			if(notification->getRepeatFlag()!="Repeat Instantly")
			{
				QRegExp rxlen("(\\d+)");
				QString value;
				int timer_value;
				int pos = rxlen.indexIn(notification->getRepeatFlag());
				if (pos > -1) {
					value = rxlen.cap(1); // "189"
                                        timer_value = (value.toInt()+8)*1000; //ms*1000 + average duration of meassage
				}

				if(!notification->timer)
				{
					notification->timer = new QTimer(notification);
					notification->timer->setInterval(timer_value);
					connect(notification->timer, SIGNAL(timeout()), this, SLOT(repeatTimerHandler()));
				}
				if(!notification->timer->isActive())
					notification->timer->start();
			}
		}
		notification->firstStart=false;
		phonon.mo->clear();
		QString str = notification->parseNotifyMessage();
		qNotifyDebug() << "play notification - " << str;

		foreach(QString item, notification->getMessageSequence()) {
			Phonon::MediaSource *ms = new Phonon::MediaSource(item);
			ms->setAutoDelete(true);
			phonon.mo->enqueue(*ms);
		}
		phonon.mo->play();
		phonon.firstPlay = false; // On Linux, you sometimes have to nudge Phonon to play 1 time before
								  // the state is not "Loading" anymore.
	}
	else
		return false; // if audio is busy

	return true;
}

void SoundNotifyPlugin::repeatTimerHandler()
{
	NotificationItem* notification = static_cast<NotificationItem*>(sender()->parent());
#ifdef DEBUG_NOTIFIES
	qDebug() << "repeatTimerHandler - " << notification->parseNotifyMessage();
#endif
	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
	UAVObject* object = objManager->getObject(notification->getDataObject());
	if(object)
		checkNotificationRule(notification,object);
}

void SoundNotifyPlugin::expireTimerHandler()
{
	// fire expire timer
	NotificationItem* notification = static_cast<NotificationItem*>(sender()->parent());
	notification->expireTimer->stop();

	if(!pendingNotifications.isEmpty())
	{
#ifdef DEBUG_NOTIFIES
		qDebug() << "expireTimerHandler - " << notification->parseNotifyMessage();
#endif
		pendingNotifications.removeOne(notification);
	}
}

void SoundNotifyPlugin::stateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    Q_UNUSED(oldstate)

#ifdef DEBUG_NOTIFIES
    qDebug() << "File length (ms): " << phonon.mo->totalTime();
    qDebug() << "New State: " << newstate;
#endif

#ifndef Q_OS_WIN
	// This is a hack to force Linux to wait until the end of the
	// wav file before moving to the next in the queue.
	// I wish I did not have to go through a #define, but I did not
	// manage to make this work on both platforms any other way!
	if (phonon.mo->totalTime()>0)
	    phonon.mo->setTransitionTime(phonon.mo->totalTime());
#endif
	if((newstate  == Phonon::PausedState) ||
	   (newstate  == Phonon::StoppedState))
	{
		nowPlayingConfiguration=NULL;
		if(!pendingNotifications.isEmpty())
		{
			NotificationItem* notification = pendingNotifications.takeFirst();
#ifdef DEBUG_NOTIFIES
			qDebug() << "play audioFree - " << notification->parseNotifyMessage();
#endif
			playNotification(notification);
                }
        } else if (newstate  == Phonon::ErrorState)
	{
		if(phonon.mo->errorType()==0) {
			qDebug() << "Phonon::ErrorState: ErrorType = " << phonon.mo->errorType();
                        phonon.mo->clearQueue();
		}
	}
//	if(newstate  == Phonon::BufferingState)
//		qDebug() << "Phonon::BufferingState!!!";
}

Q_EXPORT_PLUGIN(SoundNotifyPlugin)
