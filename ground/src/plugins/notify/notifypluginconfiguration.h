/**
 ******************************************************************************
 *
 * @file       notifypluginconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Notify Plugin configuration header
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


#ifndef NOTIFYPLUGINCONFIGURATION_H
#define NOTIFYPLUGINCONFIGURATION_H
#include <coreplugin/iuavgadgetconfiguration.h>
#include "qsettings.h"
#include <qstringlist.h>
#include <QTimer>

using namespace Core;

class NotifyPluginConfiguration : public QObject
{
	Q_OBJECT
public:
	explicit NotifyPluginConfiguration(QObject *parent = 0);

	QTimer* timer;
	QTimer* expireTimer;
	bool isNowPlaying; //
	bool firstStart;

	QString getSound1() const { return sound1; }
	void setSound1(QString text) {sound1 = text; }

	QString getSound2() const { return sound2; }
	void setSound2(QString text) {sound2 = text; }

	QString getSound3() const { return sound3; }
	void setSound3(QString text) {sound3 = text; }

	QString getValue() const { return value; }
	void setValue(QString text) {value = text; }

	QString getSayOrder() const { return sayOrder; }
	void setSayOrder(QString text) {sayOrder = text; }

	double getSpinBoxValue() const { return spinBoxValue; }
	void setSpinBoxValue(double value) {spinBoxValue = value; }


	QString getDataObject() const { return dataObject; }
	void setDataObject(QString text) {dataObject = text; }

	QString getObjectField() const { return objectField; }
	void setObjectField(QString text) { objectField = text; }

	QString getSoundCollectionPath() const { return soundCollectionPath; }
	void setSoundCollectionPath(QString text) { soundCollectionPath = text; }

	QString getCurrentLanguage() const { return currentLanguage; }
	void setCurrentLanguage(QString text) { currentLanguage = text; }

	QStringList getNotifyMessageList() const { return notifyMessageList; }
	void setNotifyMessageList(QStringList text) { notifyMessageList = text; }

	QString getRepeatFlag() const { return repeatString; }
	void setRepeatFlag(QString value) { repeatString = value; }

	bool getRepeatTimeout() const { return repeatTimeout; }
	void setRepeatTimeout(bool value) { repeatTimeout = value; }

	int getExpireTimeout() const { return expireTimeout; }
	void setExpireTimeout(int value) { expireTimeout = value; }



	void saveState(QSettings* settings) const;
	void restoreState(QSettings* settings);
	QString parseNotifyMessage();

private:
	QStringList notifyMessageList;
	QString soundCollectionPath;
	QString currentLanguage;
	QString dataObject;
	QString objectField;

	QString value;
	QString sound1;
	QString sound2;
	QString sound3;
	QString sayOrder;
	double spinBoxValue;
	QString repeatString;
	bool repeatTimeout;
	int repeatTimerValue;
	int expireTimeout;

};

Q_DECLARE_METATYPE(NotifyPluginConfiguration*)

#endif // NOTIFYPLUGINCONFIGURATION_H
