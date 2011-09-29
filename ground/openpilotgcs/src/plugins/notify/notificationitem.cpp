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

// GCS headers
#include "extensionsystem/pluginmanager.h"
#include "utils/pathutils.h"
#include "uavobjectmanager.h"
#include "uavobject.h"

// Notify plugin headers
#include "notificationitem.h"
#include "notifylogging.h"


NotificationItem::NotificationItem(QObject *parent)
    : QObject(parent)
    , isNowPlaying(0)
    , firstStart(true)
    , _soundCollectionPath("")
    , _currentLanguage("default")
    , _dataObject("")
    , _objectField("")
    , _rangeLimit("Equal to")
    , _sound1("")
    , _sound2("")
    , _sound3("")
    , _sayOrder("Never")
    , _singleValue(0)
    , _valueRange2(0)
    , _repeatString("Repeat Instantly")
    , _expireTimeout(eDefaultTimeout)
    , _mute(false)

{
    _timer = NULL;
    _expireTimer = NULL;
}

void NotificationItem::copyTo(NotificationItem* that) const
{
    that->isNowPlaying = isNowPlaying;
    that->firstStart = firstStart;
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
    that->_repeatString = _repeatString;
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
    settings->setValue(QLatin1String("Repeat"), retryString());
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
    setSayOrder(settings->value(QLatin1String("SayOrder"), tr("")).toString());
    setSingleValue(settings->value(QLatin1String("Value1"), tr("")).toDouble());
    setValueRange2(settings->value(QLatin1String("Value2"), tr("")).toDouble());
    setRetryString(settings->value(QLatin1String("Repeat"), tr("")).toString());
    setLifetime(settings->value(QLatin1String("ExpireTimeout"), tr("")).toInt());
    setMute(settings->value(QLatin1String("Mute"), tr("")).toInt());

}

void NotificationItem::seriaize(QDataStream& stream)
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
    stream << this->_repeatString;
    stream << this->_expireTimeout;
    stream << this->_mute;
}

void NotificationItem::deseriaize(QDataStream& stream)
{
    stream >> this->_soundCollectionPath;
    stream >> this->_currentLanguage;
    stream >> this->_dataObject;
    stream >> this->_objectField;
    stream >> this->_rangeLimit;
    stream >> this->_sound1;
    stream >> this->_sound2;
    stream >> this->_sound3;
    stream >> this->_sayOrder;
    stream >> this->_singleValue;
    stream >> this->_valueRange2;
    stream >> this->_repeatString;
    stream >> this->_expireTimeout;
    stream >> this->_mute;
}

void NotificationItem::startTimer(int value) {
    if (!_timer) {
        _timer = new QTimer(this);
        _timer->setInterval(value);
    }
    if (!_timer->isActive())
        _timer->start();
}

void NotificationItem::stopTimer() {
    if (_timer) {
        if (_timer->isActive())
            _timer->stop();
    }
}

void NotificationItem::disposeTimer() {
    if (_timer) {
        _timer->stop();
        delete _timer;
        _timer = NULL;
    }
}

void NotificationItem::startExpireTimer() {
    if (!_expireTimer)
    {
        _expireTimer = new QTimer(this);
    }
    _expireTimer->start(_expireTimeout * 1000);
}

void NotificationItem::stopExpireTimer() {
    if (_expireTimer) {
        if (_expireTimer)
            _expireTimer->stop();
    }
}

void NotificationItem::disposeExpireTimer() {
    if (_expireTimer) {
        _expireTimer->stop();
        delete _expireTimer;
        _expireTimer = NULL;
    }
}

#define missed "missed sound"
#define CHECK_ADD_SOUND(n) ((!_missedSound##n) ? getSound##n() : (missed#n))
#define CHECK_REPLACE_SOUND(n) ((!_missedSound##n) ? str.replace(missed#n, getSound##n()) : (missed#n))

QString NotificationItem::parseNotifyMessage()
{
    // tips:
    // check of *.wav files exist needed for playing phonon queues;
    // if phonon player don't find next file in queue, it buzz

    QString str;
    QString value;
    QString sayOrder = getSayOrder();
    UAVObjectField* field = getUAVObjectField();
    if (UAVObjectField::ENUM == field->getType()) {
        Q_ASSERT(singleValue() < field->getOptions().size());
        value = QString("%L1").arg(field->getOptions().at(singleValue()));
    } else {
        value = QString("%L1").arg(singleValue());
    }

    int position = -1; // default don't play value wav file

    // generate queue of sound files to play
    _messageSequence.clear();

    bool _missedSound1 = false;
    bool _missedSound2 = false;
    bool _missedSound3 = false;

    checkSoundFilesExisting(_missedSound1, _missedSound2, _missedSound3);
    str = CHECK_ADD_SOUND(1)+" "+CHECK_ADD_SOUND(2)+" "+CHECK_ADD_SOUND(3);

    if(!_messageSequence.size()) {
        qNotifyDebug() << "no any files in message queue";
    }

    sayOrder = sayOrder.trimmed();
    switch(sayOrder.at(0).toUpper().toAscii())
    {
    case 'B'://BEFORE:        
        CHECK_REPLACE_SOUND(1);
        CHECK_REPLACE_SOUND(2);
        CHECK_REPLACE_SOUND(3);
        str.prepend(value + " ");
        position = 0;
        break;

    case 'A'://AFTER:
        switch(sayOrder.at(6).toLower().toAscii())
        {
        case 'f':
            str = CHECK_ADD_SOUND(1)+" "+value+" "+CHECK_ADD_SOUND(2)+" "+CHECK_ADD_SOUND(3);
            position = 1;
            break;

        case 's':
            str = CHECK_ADD_SOUND(1)+" "+CHECK_ADD_SOUND(2)+" "+value+" "+CHECK_ADD_SOUND(3);
            position = 2;
            break;

        case 't':
            CHECK_REPLACE_SOUND(1);
            CHECK_REPLACE_SOUND(2);
            CHECK_REPLACE_SOUND(3);
            str.append(" "+value);
            position = 3;
            break;
        }
        break;

    default:
        CHECK_REPLACE_SOUND(1);
        CHECK_REPLACE_SOUND(2);
        CHECK_REPLACE_SOUND(3);
        break;
    }

    if(-1 == position) {
        qNotifyDebug() << "NotificationItem::parseNotifyMessage() | value position undefined";
        return str;
    }

    if (UAVObjectField::ENUM == field->getType()) return str;

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
    foreach(QString fileName, digitWavs) {
        fileName+=".wav";
        QString filePath = QDir::toNativeSeparators(getSoundCollectionPath()+"/"+ getCurrentLanguage()+"/"+fileName);
        if(QFile::exists(filePath))
            _messageSequence.insert(position++,QDir::toNativeSeparators(getSoundCollectionPath()+ "/"+getCurrentLanguage()+"/"+fileName));
        else {
            if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath()+"/default/"+fileName)))
                _messageSequence.insert(position++,QDir::toNativeSeparators(getSoundCollectionPath()+"/default/"+fileName));
            else {
                _messageSequence.clear();
                break; // if no some of *.wav files, then don't play number!
            }
        }
    }
    return str;
}

UAVObjectField* NotificationItem::getUAVObjectField() {
    return getUAVObject()->getField(getObjectField());
}

UAVDataObject* NotificationItem::getUAVObject() {
    return dynamic_cast<UAVDataObject*>((ExtensionSystem::PluginManager::instance()->getObject<UAVObjectManager>())->getObject(getDataObject()));
}


void NotificationItem::checkSoundFilesExisting(bool& missed1, bool& missed2, bool& missed3)
{

    if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath() + "/" + getCurrentLanguage()+"/"+getSound1()+".wav")))
        _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath() + "/" + getCurrentLanguage()+"/"+getSound1()+".wav"));
    else {
        if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath() + "/default/"+getSound1()+".wav")))
            _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath() + "/default/"+getSound1()+".wav"));
        else
            missed1 = true;
    }

    if(getSound2().size()) {
        if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath() + "/" +  getCurrentLanguage()+"/"+getSound2()+".wav")))
            _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath() + "/" +  getCurrentLanguage()+"/"+getSound2()+".wav"));
        else {
            if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath() + "/default/"+getSound2()+".wav")))
                _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath() + "/default/"+getSound2()+".wav"));
            else
                missed2 = true;
        }
    }

    if(getSound3().size()) {
        if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath()+ "/" + getCurrentLanguage()+"/"+getSound3()+".wav")))
            _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath()+ "/" + getCurrentLanguage()+"/"+getSound3()+".wav"));
        else {
            if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath()+"/default/"+getSound3()+".wav")))
                _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath()+"/default/"+getSound3()+".wav"));
            else
                missed3 = true;
        }
    }
}
