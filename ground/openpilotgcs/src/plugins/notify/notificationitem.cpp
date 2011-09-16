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

#include "notificationitem.h"
#include "notifylogging.h"
#include <QtCore/QDataStream>
#include <QFile>
#include "utils/pathutils.h"


NotificationItem::NotificationItem(QObject *parent)
    : QObject(parent)
    , isNowPlaying(0)
    , firstStart(true)
    , _soundCollectionPath("")
    , _currentLanguage("default")
    , _dataObject("")
    , _objectField("")
    , _dataValue("Equal to")
    , _sound1("")
    , _sound2("")
    , _sound3("")
    , _sayOrder("Never")
    , _spinBoxValue(0)
    , _repeatString("Repeat Instantly")
    , _repeatTimeout(true)
    , _expireTimeout(15)

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
    that->_dataValue = _dataValue;
    that->_sound1 = _sound1;
    that->_sound2 = _sound2;
    that->_sound3 = _sound3;
    that->_sayOrder = _sayOrder;
    that->_spinBoxValue = _spinBoxValue;
    that->_repeatString = _repeatString;
    that->_repeatTimeout = _repeatTimeout;
    that->_expireTimeout = _expireTimeout;
}


void NotificationItem::saveState(QSettings* settings) const
{
    settings->setValue("SoundCollectionPath", Utils::PathUtils().RemoveDataPath(getSoundCollectionPath()));
    settings->setValue(QLatin1String("CurrentLanguage"), getCurrentLanguage());
    settings->setValue(QLatin1String("ObjectField"), getObjectField());
    settings->setValue(QLatin1String("DataObject"), getDataObject());
    settings->setValue(QLatin1String("Value"), getValue());
    settings->setValue(QLatin1String("ValueSpinBox"), getSpinBoxValue());
    settings->setValue(QLatin1String("Sound1"), getSound1());
    settings->setValue(QLatin1String("Sound2"), getSound2());
    settings->setValue(QLatin1String("Sound3"), getSound3());
    settings->setValue(QLatin1String("SayOrder"), getSayOrder());
    settings->setValue(QLatin1String("Repeat"), getRepeatFlag());
    settings->setValue(QLatin1String("ExpireTimeout"), getExpireTimeout());
}

void NotificationItem::restoreState(QSettings* settings)
{
    //settings = Core::ICore::instance()->settings();
    setSoundCollectionPath(Utils::PathUtils().InsertDataPath(settings->value(QLatin1String("SoundCollectionPath"), tr("")).toString()));
    setCurrentLanguage(settings->value(QLatin1String("CurrentLanguage"), tr("")).toString());
    setDataObject(settings->value(QLatin1String("DataObject"), tr("")).toString());
    setObjectField(settings->value(QLatin1String("ObjectField"), tr("")).toString());
    setValue(settings->value(QLatin1String("Value"), tr("")).toString());
    setSound1(settings->value(QLatin1String("Sound1"), tr("")).toString());
    setSound2(settings->value(QLatin1String("Sound2"), tr("")).toString());
    setSound3(settings->value(QLatin1String("Sound3"), tr("")).toString());
    setSayOrder(settings->value(QLatin1String("SayOrder"), tr("")).toString());
    setSpinBoxValue(settings->value(QLatin1String("ValueSpinBox"), tr("")).toDouble());
    setRepeatFlag(settings->value(QLatin1String("Repeat"), tr("")).toString());
    setExpireTimeout(settings->value(QLatin1String("ExpireTimeout"), tr("")).toInt());
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

QString NotificationItem::parseNotifyMessage()
{
    // tips:
    // check of *.wav files exist needed for playing phonon queues;
    // if phonon player don't find next file in queue, it buzz

    QString str,str1;
    str1= getSayOrder();
    str = QString("%L1 ").arg(getSpinBoxValue());
    int position = 0xFF;
    // generate queue of sound files to play
    _messageSequence.clear();

    checkSoundFilesExisting();

    if(!_messageSequence.size()) {
        qNotifyDebug() << "no any files in message queue";
    }

    switch(str1.at(0).toAscii())
    {
    case 'N'://NEVER:
       str = getSound1()+" "+getSound2()+" "+getSound3();
       position = 0xFF;
       break;

    case 'B'://BEFORE:
       str = QString("%L1 ").arg(getSpinBoxValue())+getSound1()+" "+getSound2()+" "+getSound3();
       position = 0;
       break;

    case 'A'://AFTER:
        switch(str1.at(6).toAscii())
        {
        case 'f':
            str = getSound1()+QString(" %L1 ").arg(getSpinBoxValue())+getSound2()+" "+getSound3();
            position = 1;
            break;
        case 's':
            str = getSound1()+" "+getSound2()+QString(" %L1").arg(getSpinBoxValue())+" "+getSound3();
            position = 2;
            break;
        case 't':
            str = getSound1()+" "+getSound2()+" "+getSound3()+QString(" %L1").arg(getSpinBoxValue());
            position = 3;
            break;
        }
        break;
    }

    if(position != 0xFF)
    {
            QStringList numberParts = QString("%1").arg(getSpinBoxValue()).trimmed().split(".");
            QStringList numberFiles;

            if((numberParts.at(0).size()==1) || (numberParts.at(0).toInt()<20))
            {
                    //if(numberParts.at(0)!="0")
                            numberFiles.append(numberParts.at(0));
            } else {
                    int i=0;
                    if(numberParts.at(0).right(2).toInt()<20 && numberParts.at(0).right(2).toInt()!=0) {
                            if(numberParts.at(0).right(2).toInt()<10)
                                    numberFiles.append(numberParts.at(0).right(1));
                            else
                                    numberFiles.append(numberParts.at(0).right(2));
                            i=2;
                    }
                    for(;i<numberParts.at(0).size();i++)
                    {
                            numberFiles.prepend(numberParts.at(0).at(numberParts.at(0).size()-i-1));
                            if(numberFiles.first()==QString("0")) {
                                    numberFiles.removeFirst();
                                    continue;
                            }
                            if(i==1)
                                    numberFiles.replace(0,numberFiles.first()+'0');
                            if(i==2)
                                    numberFiles.insert(1,"100");
                            if(i==3)
                                    numberFiles.insert(1,"1000");
                    }
            }

            if(1 < numberParts.size()) {
                    numberFiles.append("point");
                    if((numberParts.at(1).size()==1))
                            numberFiles.append(numberParts.at(1));
                    else {
                            if(numberParts.at(1).left(1)=="0")
                                    numberFiles.append(numberParts.at(1).left(1));
                            else
                                    numberFiles.append(numberParts.at(1).left(1)+'0');
                            numberFiles.append(numberParts.at(1).right(1));
                    }
            }
            foreach(QString fileName,numberFiles) {
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
    }
    return str;
}


void NotificationItem::checkSoundFilesExisting() {

    if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath() + "/" + getCurrentLanguage()+"/"+getSound1()+".wav")))
        _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath() + "/" + getCurrentLanguage()+"/"+getSound1()+".wav"));
    else
        if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath() + "/default/"+getSound1()+".wav")))
            _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath() + "/default/"+getSound1()+".wav"));


    if(!getSound2().size())
    {
        if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath() + "/" +  getCurrentLanguage()+"/"+getSound2()+".wav")))
            _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath() + "/" +  getCurrentLanguage()+"/"+getSound2()+".wav"));
        else
            if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath() + "/default/"+getSound2()+".wav")))
                _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath() + "/default/"+getSound2()+".wav"));
    }

     if(!getSound3().size())
    {
        if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath()+ "/" + getCurrentLanguage()+"/"+getSound3()+".wav")))
            _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath()+ "/" + getCurrentLanguage()+"/"+getSound3()+".wav"));
        else
            if(QFile::exists(QDir::toNativeSeparators(getSoundCollectionPath()+"/default/"+getSound3()+".wav")))
                _messageSequence.append(QDir::toNativeSeparators(getSoundCollectionPath()+"/default/"+getSound3()+".wav"));
    }
}
