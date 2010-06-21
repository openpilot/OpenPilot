/**
 ******************************************************************************
 *
 * @file       donothingplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   donothingplugin
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
		 notify = new NotifyPluginConfiguration;
		 notify->restoreState(settings);
		 lstNotifications.append(notify);
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

	QList<Phonon::MediaObject*> deleteList = mapMediaObjects.values();
	if(!deleteList.isEmpty())
		foreach(Phonon::MediaObject* mediaObj,deleteList)
			delete mediaObj;

	// Check validity of arguments first, reject empty args and unknown fields.
	foreach(NotifyPluginConfiguration* notify,lstNotifications) {
		UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(notify->getDataObject()) );
		if (obj != NULL ) {
			std::cout << "Connected Object (" << notify->getDataObject().toStdString() << ")." << std::endl;
			connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(playNotification(UAVObject*)));
			lstNotifiedUAVObjects.append(obj);
			notify->parseNotifyMessage();
			mediaSource = notify->getNotifyMessageList();
			lstMediaSource.append(new QList<Phonon::MediaSource>);
			foreach(QString item, mediaSource)
					lstMediaSource.last()->append(Phonon::MediaSource(item));

			// set notification message to current event
			mapMediaObjects[obj->getName()] = new Phonon::MediaObject;
			mapMediaObjects[obj->getName()] = Phonon::createPlayer(Phonon::NotificationCategory);
			mapMediaObjects[obj->getName()]->setQueue(*lstMediaSource.last());
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
	bool play = false;

	foreach(NotifyPluginConfiguration* notify, lstNotifications) {
		if(object->getName()==notify->getDataObject()) {
			QString fld = notify->getObjectField();
			field = object->getField(fld);
			threshold = notify->getSpinBoxValue();
			direction = notify->getValue();
		}
	}

	if (field) {
		switch(direction[0].toAscii())
		{
		case 'E':
			if(field->getDouble()==threshold)
				play = true;
			break;
		case 'G':
			if(field->getDouble()>threshold)
				play = true;
			break;
		case 'L':
			if(field->getDouble()<threshold)
				play = true;
			break;
		}

		if(play)
		{
			if((mapMediaObjects[object->getName()]->state()==Phonon::PausedState) ||
			   (mapMediaObjects[object->getName()]->state()==Phonon::StoppedState))
			{
				mapMediaObjects[object->getName()]->clear();
				mapMediaObjects[object->getName()]->setQueue(*lstMediaSource.last());
				mapMediaObjects[object->getName()]->play();
			}
		}
	}
}

Q_EXPORT_PLUGIN(SoundNotifyPlugin)
