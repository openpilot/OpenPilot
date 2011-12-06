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

    enum {never,beforeFirst,beforeSecond,afterSecond};
    enum {repeatOncePerUpdate,repeatOnce,repeatInstantly,repeat10seconds,
          repeat30seconds,repeat1minute};

    explicit NotificationItem(QObject *parent = 0);

    void copyTo(NotificationItem*) const;

    DECLARE_SOUND(1)
    DECLARE_SOUND(2)
    DECLARE_SOUND(3)

    bool getCurrentUpdatePlayed() const {return _currentUpdatePlayed;}
    void setCurrentUpdatePlayed(bool value){_currentUpdatePlayed=value;}

    int getCondition() const { return _condition; }
    void setCondition(int value) { _condition = value; }

    int getSayOrder() const { return _sayOrder; }
    void setSayOrder(int text) { _sayOrder = text; }

    QVariant singleValue() const { return _singleValue; }
    void setSingleValue(QVariant value) { _singleValue = value; }

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

    int retryValue() const { return _repeatValue; }
    void setRetryValue(int value) { _repeatValue = value; }

    int lifetime() const { return _expireTimeout; }
    void setLifetime(int value) { _expireTimeout = value; }

    bool mute() const { return _mute; }
    void setMute(bool value) { _mute = value; }

    void saveState(QSettings* settings) const;
    void restoreState(QSettings* settings);


    UAVDataObject* getUAVObject(void);
    UAVObjectField* getUAVObjectField(void);

    void serialize(QDataStream& stream);
    void deserialize(QDataStream& stream);

    /**
    * Convert notification item fields in single string,
    * to show in table for example
    *
    * @return string which describe notification
    */
    QString toString();

    /**
    * Generate list of sound files needed to play notification
    *
    * @return success - reference to non-empty _messageSequence;
    *         error   - if one of sounds doesn't exist returns
    *                   reference to empty _messageSequence;
    */
    QStringList& toSoundList();

    /**
    * Returns sound caption name, needed to create string representation of notification.
    *
    * @return success - string  == <sound filename>, if sound file exists
    *         error   - string  == [missind]<sound filename>, if sound file doesn't exist
    */
    QString getSoundCaption(QString fileName);


    QTimer* getTimer() const { return _timer; }
    void startTimer(int value);
    void restartTimer();
    void stopTimer();
    void disposeTimer();

    QTimer* getExpireTimer() const { return _expireTimer; }
    void startExpireTimer();
    void stopExpireTimer();

    void disposeExpireTimer();

    bool isNowPlaying;
    bool _isPlayed;

    static QStringList sayOrderValues;
    static QStringList retryValues;

private:
    QString checkSoundExists(QString fileName);

private:

    bool _currentUpdatePlayed;

    QTimer* _timer;

    //! time from putting notification in queue till moment when notification became out-of-date
    //! NOTE: each notification has it lifetime, this time setups individually for each notification
    //!       according to its priority
    QTimer* _expireTimer;

    //! list of wav files from which notification consists
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
    int _condition;

    //! possible sounds(at least one required to play notification)
    QString _sound1;
    QString _sound2;
    QString _sound3;

    //! order in what sounds 1-3 will be played
    int _sayOrder;

    //! one-side range, value(numeric or ENUM type) maybe lower, greater or in range
    QVariant _singleValue;

    //! both-side range, value should be inside the range
    //double _valueRange1;
    double _valueRange2;

    //! how often or what periodicaly notification should be played
    int _repeatValue;

    //! time after event occured till notification became invalid
    //! and will be removed from list
    int _expireTimeout;

    //! enables/disables playing of current notification
    bool _mute;
};

Q_DECLARE_METATYPE(NotificationItem*)

#endif // NotificationItem_H
