/**
 ******************************************************************************
 *
 * @file       NotifyPluginConfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Airspeed Plugin Gadget configuration
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   Airspeed
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

#include "notifypluginconfiguration.h"
#include <QtCore/QDataStream>
#include <QFile>


NotifyPluginConfiguration::NotifyPluginConfiguration(QObject *parent) :
	soundCollectionPath(""),
	currentLanguage("default"),
	dataObject(""),
	objectField(""),
	value("Equal to"),
	sound1(""),
	sound2(""),
	sayOrder("Never"),
	spinBoxValue(0)
{

}


void NotifyPluginConfiguration::saveState(QSettings* settings) const
{
	settings->setValue("SoundCollectionPath", getSoundCollectionPath());
	settings->setValue(QLatin1String("CurrentLanguage"), getCurrentLanguage());
	settings->setValue(QLatin1String("ObjectField"), getObjectField());
	settings->setValue(QLatin1String("DataObject"), getDataObject());
	settings->setValue(QLatin1String("Value"), getValue());
	settings->setValue(QLatin1String("ValueSpinBox"), getSpinBoxValue());
	settings->setValue(QLatin1String("Sound1"), getSound1());
	settings->setValue(QLatin1String("Sound2"), getSound2());
	settings->setValue(QLatin1String("SayOrder"), getSayOrder());

}

void NotifyPluginConfiguration::restoreState(QSettings* settings)
{
	//settings = Core::ICore::instance()->settings();
	setSoundCollectionPath(settings->value(QLatin1String("SoundCollectionPath"), tr("")).toString());
	setCurrentLanguage(settings->value(QLatin1String("CurrentLanguage"), tr("")).toString());
	setDataObject(settings->value(QLatin1String("DataObject"), tr("")).toString());
	setObjectField(settings->value(QLatin1String("ObjectField"), tr("")).toString());
	setValue(settings->value(QLatin1String("Value"), tr("")).toString());
	setSound1(settings->value(QLatin1String("Sound1"), tr("")).toString());
	setSound2(settings->value(QLatin1String("Sound2"), tr("")).toString());
	setSayOrder(settings->value(QLatin1String("SayOrder"), tr("")).toString());
	setSpinBoxValue(settings->value(QLatin1String("ValueSpinBox"), tr("")).toDouble());
}


QString NotifyPluginConfiguration::parseNotifyMessage()
{
	// tips:
	// check of *.wav files exist needed for playing phonon queues;
	// if phonon player don't find next file in queue, it buzz

	QString str,str1;
	str1= getSayOrder();
	str = QString("%L1 ").arg(getSpinBoxValue());
	int position = 0xFF;
	// generate queue of sound files to play
	notifyMessageList.clear();

	if(QFile::exists(getSoundCollectionPath()+"\\"+getCurrentLanguage()+"\\"+getSound1()+".wav"))
		notifyMessageList.append(getSoundCollectionPath()+"\\"+getCurrentLanguage()+"\\"+getSound1()+".wav");
	else
		if(QFile::exists(getSoundCollectionPath()+"\\default\\"+getSound2()+".wav"))
			notifyMessageList.append(getSoundCollectionPath()+"\\default\\"+getSound1()+".wav");

	if(getSound2()!="")
	{
		if(QFile::exists(getSoundCollectionPath()+"\\"+getCurrentLanguage()+"\\"+getSound2()+".wav"))
			notifyMessageList.append(getSoundCollectionPath()+"\\"+getCurrentLanguage()+"\\"+getSound2()+".wav");
		else
			if(QFile::exists(getSoundCollectionPath()+"\\"+"\\default\\"+"\\"+getSound2()+".wav"))
				notifyMessageList.append(getSoundCollectionPath()+"\\default\\"+getSound2()+".wav");
	}

	switch(str1.at(0).toAscii())
	{
		case 'N'://NEVER:
		   str = getSound1()+" "+getSound2();
		   position = 0xFF;
		   break;

		case 'B'://BEFORE:
		   str = QString("%L1 ").arg(getSpinBoxValue())+getSound1()+" "+getSound2();
		   position = 0;
		   break;

		case 'A'://AFTER1:
			switch(str1.at(6).toAscii())
			{
			case 'f':
				str = getSound1()+QString(" %L1 ").arg(getSpinBoxValue())+getSound2();
				position = 1;
				break;
			case 's':
				str = getSound1()+" "+getSound2()+QString(" %L1").arg(getSpinBoxValue());
				position = 2;
				break;
			}
			break;
	}

	if(position!=0xFF)
	{
		if(QFile::exists(getSoundCollectionPath()+"\\"+getCurrentLanguage()+"\\"+QString("%L1 ").arg(getSpinBoxValue())+".wav"))
			notifyMessageList.insert(position,getSoundCollectionPath()+"\\"+getCurrentLanguage()+"\\"+QString("%L1 ").arg(getSpinBoxValue())+".wav");
		else
			if(QFile::exists(getSoundCollectionPath()+"\\"+getCurrentLanguage()+"\\"+QString("%L1 ").arg(getSpinBoxValue())+".wav"))
				notifyMessageList.insert(position,getSoundCollectionPath()+"\\default\\"+QString("%L1 ").arg(getSpinBoxValue())+".wav");
	}

	//str.replace(QString(".wav | .mp3"), QString(""));
	return str;
}
