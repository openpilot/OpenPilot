/**
 ******************************************************************************
 *
 * @file       NotificationItem.h
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


#ifndef NOTIFICATION_ITEM_H
#define NOTIFICATION_ITEM_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include "qsettings.h"
#include <qstringlist.h>
#include <QTimer>

using namespace Core;

#define DECLARE_SOUND(number) \
	QString getSound##number() const { return _sound##number; } \
	void setSound##number(QString text) { _sound##number = text; } \

class UAVDataObject;
class UAVObjectField;

class NotificationItem : public QObject
{
    Q_OBJECT
public:
    enum { eDefaultTimeout = 15 }; // in sec

    explicit NotificationItem(QObject *parent = 0);

    void copyTo(NotificationItem*) const;

    DECLARE_SOUND(1)
    DECLARE_SOUND(2)
    DECLARE_SOUND(3)

    QString range() const { return _rangeLimit; }
    void setRange(QString text) { _rangeLimit = text; }

    QString getSayOrder() const { return _sayOrder; }
    void setSayOrder(QString text) { _sayOrder = text; }

    double singleValue() const { return _singleValue; }
    void setSingleValue(double value) { _singleValue = value; }

    double valueRange2() const { return _valueRange2; }
    void setValueRange2(double value) { _valueRange2 = value; }

    QString getDataObject() const { return _dataObject; }
    void setDataObject(QString text) { _dataObject = text; }

    QString getObjectField() const { return _objectField; }
    void setObjectField(QString text) { _objectField = text; }

    QString getSoundCollectionPath() const { return _soundCollectionPath; }
    void setSoundCollectionPath(QString path) { _soundCollectionPath = path; }

    QString getCurrentLanguage() const { return _currentLanguage; }
    void setCurrentLanguage(QString text) { _currentLanguage = text; }

    QStringList getMessageSequence() const { return _messageSequence; }
    void setMessageSequence(QStringList sequence) { _messageSequence = sequence; }

    QString retryString() const { return _repeatString; }
    void setRetryString(QString value) { _repeatString = value; }

    int lifetime() const { return _expireTimeout; }
    void setLifetime(int value) { _expireTimeout = value; }

    bool mute() const { return _mute; }
    void setMute(bool value) { _mute = value; }

    void saveState(QSettings* settings) const;
    void restoreState(QSettings* settings);


    UAVDataObject* getUAVObject(void);
    UAVObjectField* getUAVObjectField(void);

    void seriaize(QDataStream& stream);
    void deseriaize(QDataStream& stream);

    QString parseNotifyMessage();

    QTimer* getTimer() const { return _timer; }
    void startTimer(int value);
    void stopTimer();
    void disposeTimer();

    QTimer* getExpireTimer() const { return _expireTimer; }
    void startExpireTimer();
    void stopExpireTimer();

    void disposeExpireTimer();

    bool isNowPlaying;
    bool firstStart;

private:
    void checkSoundFilesExisting(bool& missed1, bool& missed2, bool& missed3);

private:
    QTimer* _timer;

    //! time from putting notification in queue till moment when notification became out-of-date
    //! NOTE: each notification has it lifetime, this time setups individually for each notification
    //!       according to its priority
    QTimer* _expireTimer;

    QStringList _messageSequence;

    //! path to folder with sound files
    QString _soundCollectionPath;

    //! language in what notifications will be spelled
    QString _currentLanguage;

    //! one UAV object per one notification
    QString _dataObject;

    //! one field value change can be assigned to one notification
    QString _objectField;

    //! fire condition for UAV field value (lower, greater, in range)
    QString _rangeLimit;

    //! possible sounds(at least one required to play notification)
    QString _sound1;
    QString _sound2;
    QString _sound3;

    //! order in what sounds 1-3 will be played
    QString _sayOrder;

    //! one-side range, value maybe lower or greater
    double _singleValue;

    //! both-side range, value should be inside the range
    //double _valueRange1;
    double _valueRange2;

    //! how often or what periodicaly notification should be played
    QString _repeatString;

    //! time after event occured till notification became invalid
    //! and will be removed from list
    int _expireTimeout;

    //! enables/disables playing of current notification
    bool _mute;
};

Q_DECLARE_METATYPE(NotificationItem*)

#endif // NotificationItem_H
