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
#include "notifypluginconfiguration.h"
#include "notifypluginoptionspage.h"
#include <coreplugin/icore.h>
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>
#include <iostream>


SoundNotifyPlugin::SoundNotifyPlugin()
{
   // Do nothing
}

SoundNotifyPlugin::~SoundNotifyPlugin()
{
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
	settings = Core::ICore::instance()->settings();
	settings->beginGroup(QLatin1String("NotifyPlugin"));

	// read list of notifications from settings
	int size = settings->beginReadArray("listNotifies");
	for (int i = 0; i < size; ++i) {
		 settings->setArrayIndex(i);
		 NotifyPluginConfiguration* notification = new NotifyPluginConfiguration;
		 notification->restoreState(settings);
		 lstNotifications.append(notification);
	}
	settings->endArray();
	setEnableSound(settings->value(QLatin1String("EnableSound"),0).toBool());
	settings->endGroup();

	connectNotifications();
} 

void SoundNotifyPlugin::shutdown()
{ 
   // Do nothing 
}


void SoundNotifyPlugin::connectNotifications()
{
	foreach(UAVDataObject* obj,lstNotifiedUAVObjects) {
		if (obj != NULL)
			disconnect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(playNotification(UAVObject*)));
	}
	if(!enableSound) return;

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

	lstNotifiedUAVObjects.clear();

//	QList<Phonon::MediaObject*> deleteList = mapMediaObjects.values();
//	if(!deleteList.isEmpty())
//		foreach(Phonon::MediaObject* mediaObj,deleteList)
//			delete mediaObj;

	// Check validity of arguments first, reject empty args and unknown fields.
	foreach(NotifyPluginConfiguration* notify,lstNotifications) {
		UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(notify->getDataObject()) );
		if (obj != NULL ) {
			if(!lstNotifiedUAVObjects.contains(obj))
			{
				lstNotifiedUAVObjects.append(obj);
				connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(playNotification(UAVObject*)));
			}
			//lstMediaSource.append(new QList<Phonon::MediaSource>);

			QMap<QString, PhononObject>::const_iterator iter = mapMediaObjects.find(obj->getName());
			if(iter==mapMediaObjects.end()) {
				// set notification message to current event
				mapMediaObjects[obj->getName()].mo = new Phonon::MediaObject;
				mapMediaObjects[obj->getName()].mo = Phonon::createPlayer(Phonon::NotificationCategory);
				mapMediaObjects[obj->getName()].ms = new QList<Phonon::MediaSource>;
				mapMediaObjects[obj->getName()].mo->clear();
			}

//			notify->parseNotifyMessage();
//			foreach(QString item, notify->getNotifyMessageList())
//			{
//				mapMediaObjects[obj->getName()].ms->clear();
//				mapMediaObjects[obj->getName()].ms->append(Phonon::MediaSource(item));
//			}

			//mapMediaObjects[obj->getName()].ms = lstMediaSource.last();
		} else {
			std::cout << "Error: Object is unknown (" << notify->getDataObject().toStdString() << ")." << std::endl;
		}
	}
}

void SoundNotifyPlugin::playNotification(UAVObject *object)
{
	UAVObjectField* field;
	double threshold;
	QString direction;
	QString fieldName;
	bool play = false;
	NotifyPluginConfiguration* notification;

	foreach(notification, lstNotifications) {
		if(object->getName()==notification->getDataObject()) {
			fieldName = notification->getObjectField();
			field = object->getField(fieldName);
			threshold = notification->getSpinBoxValue();
			direction = notification->getValue();
		}

		if (field) {
			double value = field->getDouble();
			//qDebug() << fieldName << " - value - " << value;

			switch(direction[0].toAscii())
			{
			case 'E':
				if(value==threshold)
					play = true;
				break;
			case 'G':
				if(value>threshold)
					play = true;
				break;
			case 'L':
				if(value<threshold)
					play = true;
				break;
			}

			if(play)
			{
				play = false;
				if((mapMediaObjects[object->getName()].mo->state()==Phonon::PausedState) ||
				   (mapMediaObjects[object->getName()].mo->state()==Phonon::StoppedState))
				{
					qDebug() << fieldName << " - value - " << value;
					mapMediaObjects[object->getName()].mo->clear();
					mapMediaObjects[object->getName()].ms->clear();
					notification->parseNotifyMessage();
					foreach(QString item, notification->getNotifyMessageList())
						mapMediaObjects[object->getName()].ms->append(Phonon::MediaSource(item));
					mapMediaObjects[object->getName()].mo->setQueue(*mapMediaObjects[object->getName()].ms);
					mapMediaObjects[object->getName()].mo->play();
					if(notification->getRepeatFlag()=="Once")
						lstNotifications.removeOne(notification);
					break;
				}
			}
		}
	}
}

Q_EXPORT_PLUGIN(SoundNotifyPlugin)
