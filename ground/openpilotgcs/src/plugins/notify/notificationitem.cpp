/**
 ******************************************************************************
 *
 * @file       NotificationItem.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Notify Plugin configuration
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

//Qt headers
#include <QtCore/QDataStream>
#include <QFile>
#include <QMap>

// GCS headers
#include "extensionsystem/pluginmanager.h"
#include "utils/pathutils.h"
#include "uavobjectmanager.h"
#include "uavobject.h"

// Notify plugin headers
#include "notificationitem.h"
#include "notifylogging.h"


static const QString cStrNever(QT_TR_NOOP("Never"));
static const QString cStrBefore1st(QT_TR_NOOP("Before first"));
static const QString cStrBefore2nd(QT_TR_NOOP("Before second"));
static const QString cStrAfter2nd(QT_TR_NOOP("After second"));

static const QString cStrRetryOnce(QT_TR_NOOP("Repeat Once"));
static const QString cStrRetryInstantly(QT_TR_NOOP("Repeat Instantly"));
static const QString cStrRetry10sec(QT_TR_NOOP("Repeat 10 seconds"));
static const QString cStrRetry30sec(QT_TR_NOOP("Repeat 30 seconds"));
static const QString cStrRetry1min(QT_TR_NOOP("Repeat 1 minute"));

QMap<QString, NotificationItem::ESayOrder> NotificationItem::sayOrderValues;
QMap<QString, NotificationItem::ERetryValues> NotificationItem::retryValues;


NotificationItem::NotificationItem(QObject *parent)
    : QObject(parent)
    , isNowPlaying(0)
    , _isPlayed(false)
    , _timer(NULL)
    , _expireTimer(NULL)
    , _soundCollectionPath("")
    , _currentLanguage("default")
    , _dataObject("")
    , _objectField("")
    , _rangeLimit("Equal to")
    , _sound1("")
    , _sound2("")
    , _sound3("")
    , _sayOrder(eNever)
    , _singleValue(0)
    , _valueRange2(0)
    , _repeatValue(eInstantly)
    , _expireTimeout(eDefaultTimeout)
    , _mute(false)
{
    NotificationItem::sayOrderValues.clear();
    NotificationItem::sayOrderValues[cStrNever] = eNever;
    NotificationItem::sayOrderValues[cStrBefore1st] = eBeforeFirst;
    NotificationItem::sayOrderValues[cStrBefore2nd] = eBeforeSecond;
    NotificationItem::sayOrderValues[cStrAfter2nd] = eAfterSecond;

    NotificationItem::retryValues.clear();
    NotificationItem::retryValues[cStrRetryOnce] = eOnce;
    NotificationItem::retryValues[cStrRetryInstantly] = eInstantly;
    NotificationItem::retryValues[cStrRetry10sec] = eRepeatTenSec;
    NotificationItem::retryValues[cStrRetry30sec] = eRepeatThirtySec;
    NotificationItem::retryValues[cStrRetry1min] = eRepeatOneMin;

}

void NotificationItem::copyTo(NotificationItem* that) const
{
    that->isNowPlaying = isNowPlaying;
    that->_isPlayed = _isPlayed;
    that->_soundCollectionPath = _soundCollectionPath;
    that->_currentLanguage = _currentLanguage;
    that->_soundCollectionPath = _soundCollectionPath;
    that->_dataObject = _dataObject;
    that->_objectField = _objectField;
    that->_rangeLimit = _rangeLimit;
    that->_sound1 = _sound1;
    that->_sound2 = _sound2;
    that->_sound3 = _sound3;
    that->_sayOrder = _sayOrder;
    that->_singleValue = _singleValue;
    that->_valueRange2 = _valueRange2;
    that->_repeatValue = _repeatValue;
    that->_expireTimeout = _expireTimeout;
    that->_mute = _mute;

}


void NotificationItem::saveState(QSettings* settings) const
{
    settings->setValue("SoundCollectionPath", Utils::PathUtils().RemoveDataPath(getSoundCollectionPath()));
    settings->setValue(QLatin1String("CurrentLanguage"), getCurrentLanguage());
    settings->setValue(QLatin1String("ObjectField"), getObjectField());
    settings->setValue(QLatin1String("DataObject"), getDataObject());
    settings->setValue(QLatin1String("RangeLimit"), range());
    settings->setValue(QLatin1String("Value1"), singleValue());
    settings->setValue(QLatin1String("Value2"), valueRange2());
    settings->setValue(QLatin1String("Sound1"), getSound1());
    settings->setValue(QLatin1String("Sound2"), getSound2());
    settings->setValue(QLatin1String("Sound3"), getSound3());
    settings->setValue(QLatin1String("SayOrder"), getSayOrder());
    settings->setValue(QLatin1String("Repeat"), retryValue());
    settings->setValue(QLatin1String("ExpireTimeout"), lifetime());
    settings->setValue(QLatin1String("Mute"), mute());
}

void NotificationItem::restoreState(QSettings* settings)
{
    //settings = Core::ICore::instance()->settings();
    setSoundCollectionPath(Utils::PathUtils().InsertDataPath(settings->value(QLatin1String("SoundCollectionPath"), tr("")).toString()));
    setCurrentLanguage(settings->value(QLatin1String("CurrentLanguage"), tr("")).toString());
    setDataObject(settings->value(QLatin1String("DataObject"), tr("")).toString());
    setObjectField(settings->value(QLatin1String("ObjectField"), tr("")).toString());
    setRange(settings->value(QLatin1String("RangeLimit"), tr("")).toString());
    setSound1(settings->value(QLatin1String("Sound1"), tr("")).toString());
    setSound2(settings->value(QLatin1String("Sound2"), tr("")).toString());
    setSound3(settings->value(QLatin1String("Sound3"), tr("")).toString());
    setSayOrder((ESayOrder)settings->value(QLatin1String("SayOrder"), eNever).toInt());
    QVariant value = settings->value(QLatin1String("Value1"), tr(""));
    setSingleValue(value);
    setValueRange2(settings->value(QLatin1String("Value2"), tr("")).toDouble());
    setRetryValue((ERetryValues)settings->value(QLatin1String("Repeat"), eOnce).toInt());
    setLifetime(settings->value(QLatin1String("ExpireTimeout"), tr("")).toInt());
    setMute(settings->value(QLatin1String("Mute"), tr("")).toInt());
}

void NotificationItem::serialize(QDataStream& stream)
{
    stream << this->_soundCollectionPath;
    stream << this->_currentLanguage;
    stream << this->_dataObject;
    stream << this->_objectField;
    stream << this->_rangeLimit;
    stream << this->_sound1;
    stream << this->_sound2;
    stream << this->_sound3;
    stream << this->_sayOrder;
    stream << this->_singleValue;
    stream << this->_valueRange2;
    stream << this->_repeatValue;
    stream << this->_expireTimeout;
    stream << this->_mute;
}

void NotificationItem::deserialize(QDataStream& stream)
{
    int sayOrderInt = 0;
    int repeatValueInt = 0;
    stream >> this->_soundCollectionPath;
    stream >> this->_currentLanguage;
    stream >> this->_dataObject;
    stream >> this->_objectField;
    stream >> this->_rangeLimit;
    stream >> this->_sound1;
    stream >> this->_sound2;
    stream >> this->_sound3;
    stream >> sayOrderInt;
    this->_sayOrder = (ESayOrder)sayOrderInt;
    stream >> this->_singleValue;
    stream >> this->_valueRange2;
    stream >> repeatValueInt;
    this->_repeatValue = (ERetryValues)repeatValueInt;
    stream >> this->_expireTimeout;
    stream >> this->_mute;
}

void NotificationItem::startTimer(int msec)
{
    if (!_timer) {
        _timer = new QTimer(this);
        _timer->setInterval(msec);
    }
    if (!_timer->isActive())
        _timer->start();
}


void NotificationItem::restartTimer()
{
    if (!_timer) {
        if (!_timer->isActive())
            _timer->start();
    }
}


void NotificationItem::stopTimer()
{
    if (_timer) {
        if (_timer->isActive())
            _timer->stop();
    }
}

void NotificationItem::disposeTimer()
{
    if (_timer) {
        _timer->stop();
        delete _timer;
        _timer = NULL;
    }
}

void NotificationItem::startExpireTimer()
{
    if (!_expireTimer) {
        _expireTimer = new QTimer(this);
    }
    _expireTimer->start(_expireTimeout * 1000);
}

void NotificationItem::stopExpireTimer()
{
    if (_expireTimer) {
        if (_expireTimer)
            _expireTimer->stop();
    }
}

void NotificationItem::disposeExpireTimer()
{
    if (_expireTimer) {
        _expireTimer->stop();
        delete _expireTimer;
        _expireTimer = NULL;
    }
}

NotificationItem::ESayOrder getValuePosition(QString sayOrder)
{
    return NotificationItem::sayOrderValues.find(sayOrder).value();
}

QString NotificationItem::checkSoundExists(QString fileName)
{
    QString name(fileName + ".wav");
    QString filePath = QDir::toNativeSeparators(getSoundCollectionPath() + "/" +
                                                getCurrentLanguage() + "/" +
                                                name);
    if(QFile::exists(filePath))
        return filePath;
    else {
        filePath = QDir::toNativeSeparators(getSoundCollectionPath() +
                                            "/default/" +
                                            name);
        if(!QFile::exists(filePath))
            filePath.clear();
    }
    return filePath;
}

QStringList valueToSoundList(QString value)
{
    // replace point chr if exists
    value = value.replace(',', '.');
    QStringList numberParts = value.trimmed().split(".");
    QStringList digitWavs;

    if ( (numberParts.at(0).size() == 1) || (numberParts.at(0).toInt() < 20) ) {
        // [1] check, is this number < 20, these numbers played by one wav file
        digitWavs.append(numberParts.at(0));
    } else {
        int i=0;
        // [2] store two lowest digits of number
        int num = numberParts.at(0).right(2).toInt();
        if (num < 20 && num != 0) {
            // store eighter number in range [0...10) or in range [10...20)
            digitWavs.append(numberParts.at(0).right(1 + num/11));
            i=2;
        }
        // [3] prepend 100 and 1000 digits of number
        for (;i<numberParts.at(0).size();i++)
        {
            digitWavs.prepend(numberParts.at(0).at(numberParts.at(0).size()-i-1));
            if(digitWavs.first()==QString("0")) {
                digitWavs.removeFirst();
                continue;
            }
            if (i==1)
                digitWavs.replace(0,digitWavs.first()+'0');
            if (i==2)
                digitWavs.insert(1,"100");
            if (i==3)
                digitWavs.insert(1,"1000");
        }
    }
    // check, is there fractional part of number?
    if (1 < numberParts.size()) {
        digitWavs.append("point");
        if (numberParts.at(1).size()==1) {
            // this mean -> number < 1
            digitWavs.append(numberParts.at(1));
        } else {
            // append fractional part of number
            QString left = numberParts.at(1).left(1);
            (left == "0") ? digitWavs.append(left) : digitWavs.append(left + '0');
            digitWavs.append(numberParts.at(1).right(1));
        }
    }
    return digitWavs;
}

QString stringFromValue(QVariant value, UAVObjectField* field)
{
    Q_ASSERT(field);
    Q_ASSERT(!value.isNull());
    QString str;
    if (UAVObjectField::ENUM == field->getType()) {
        if(!field->getOptions().contains(value.toString()))
            return QString();
        str = value.toString();
    } else {
        str = QString("%L1").arg(value.toDouble());
    }
    return str;
}

QString NotificationItem::toString()
{
    QString str;
    UAVObjectField* field = getUAVObjectField();
    QString value = stringFromValue(singleValue(), field);

    QStringList lst;
    lst.append(getSoundCaption(getSound1()));
    lst.append(getSoundCaption(getSound2()));
    lst.append(getSoundCaption(getSound3()));
    QStringList valueSounds = valueToSoundList(value);
    bool missed = false;
    foreach(QString sound, valueSounds) {
        if(checkSoundExists(sound).isEmpty()) {
         missed = true;
         break;
        }
    }

    // if not "Never" case
    if(eNever != _sayOrder) {
        if(missed)
            lst.insert((int)_sayOrder, "[missed]" + value);
        else
            lst.insert((int)_sayOrder, value);
    }
    str = lst.join(" ");
    return str;
}

QStringList& NotificationItem::toSoundList()
{
    // tips:
    // check of *.wav files exist needed for playing phonon queues;
    // if phonon player don't find next file in queue, it buzz
    UAVObjectField* field = getUAVObjectField();
    QString value = stringFromValue(singleValue(), field);

    // generate queue of sound files to play
    _messageSequence.clear();
    QStringList lst;
    if(!getSound1().isEmpty())
        lst.append(getSound1());
    if(!getSound2().isEmpty())
        lst.append(getSound2());
    if(!getSound3().isEmpty())
        lst.append(getSound3());

    // if not "Never" case
    if(eNever != _sayOrder) {
        QStringList valueSounds = valueToSoundList(value);
        int pos = (int)_sayOrder;
        foreach(QString sound, valueSounds)
            lst.insert(pos++, sound);
    }

    foreach(QString sound, lst) {
        QString path = checkSoundExists(sound);
        if (!path.isEmpty()) {
            _messageSequence.append(path);
        } else {
            _messageSequence.clear();
            break;
        }
    }
    return _messageSequence;
}

QString NotificationItem::getSoundCaption(QString fileName)
{
    if(fileName.isEmpty()) return QString();
    if(checkSoundExists(fileName).isEmpty()) {
        return QString("[missed]") + fileName;
    }
    return fileName;
}

UAVObjectField* NotificationItem::getUAVObjectField() {
    return getUAVObject()->getField(getObjectField());
}

UAVDataObject* NotificationItem::getUAVObject() {
    return dynamic_cast<UAVDataObject*>((ExtensionSystem::PluginManager::instance()->getObject<UAVObjectManager>())->getObject(getDataObject()));
}
