/**
 ******************************************************************************
 *
 * @file       notifypluginoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Notify Plugin options page
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

#include "notifypluginoptionspage.h"
#include <coreplugin/icore.h>
#include "notificationitem.h"
#include "ui_notifypluginoptionspage.h"
#include "extensionsystem/pluginmanager.h"
#include "utils/pathutils.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>
#include <QtCore/QSettings>
#include <QTableWidget>
#include <QPalette>
#include <QBuffer>
#include <QSpinBox>
#include <QLineEdit>

#include "notifyplugin.h"
#include "notifyitemdelegate.h"
#include "notifytablemodel.h"
#include "notifylogging.h"

QStringList NotifyPluginOptionsPage::conditionValues;

NotifyPluginOptionsPage::NotifyPluginOptionsPage(QObject *parent)
    : IOptionsPage(parent)
    , _objManager(*ExtensionSystem::PluginManager::instance()->getObject<UAVObjectManager>())
    , _owner(qobject_cast<SoundNotifyPlugin*>(parent))
    , _dynamicFieldCondition(NULL)
    , _dynamicFieldWidget(NULL)
    , _dynamicFieldType(-1)
    , _sayOrder(NULL)
    , _form(NULL)
    , _selectedNotification(NULL)
{
    NotifyPluginOptionsPage::conditionValues.insert(equal,tr("Equal to"));
    NotifyPluginOptionsPage::conditionValues.insert(bigger,tr("Large than"));
    NotifyPluginOptionsPage::conditionValues.insert(smaller,tr("Lower than"));
    NotifyPluginOptionsPage::conditionValues.insert(inrange,tr("In range"));

}

NotifyPluginOptionsPage::~NotifyPluginOptionsPage()
{}

QWidget *NotifyPluginOptionsPage::createPage(QWidget * /* parent */)
{
    _optionsPage.reset(new Ui::NotifyPluginOptionsPage());
    //main widget
    QWidget* optionsPageWidget = new QWidget;
    _dynamicFieldWidget = NULL;
    _dynamicFieldCondition = NULL;
    resetFieldType();
    //save ref to form, needed for binding dynamic fields in future
    _form = optionsPageWidget;
    //main layout
    _optionsPage->setupUi(optionsPageWidget);

    _optionsPage->SoundDirectoryPathChooser->setExpectedKind(Utils::PathChooser::Directory);
    _optionsPage->SoundDirectoryPathChooser->setPromptDialogTitle(tr("Choose sound collection directory"));

    connect(_optionsPage->SoundDirectoryPathChooser, SIGNAL(changed(const QString&)),
            this, SLOT(on_clicked_buttonSoundFolder(const QString&)));
    connect(_optionsPage->SoundCollectionList, SIGNAL(currentIndexChanged (int)),
            this, SLOT(on_changedIndex_soundLanguage(int)));

    connect(this, SIGNAL(updateNotifications(QList<NotificationItem*>)),
        _owner, SLOT(updateNotificationList(QList<NotificationItem*>)));
    //connect(this, SIGNAL(resetNotification()),owner, SLOT(resetNotification()));

    _privListNotifications = _owner->getListNotifications();


    // [1]
    setSelectedNotification(_owner->getCurrentNotification());
    addDynamicFieldLayout();
    // [2]
    updateConfigView(_selectedNotification);

    initRulesTable();
    initButtons();
    initPhononPlayer();

    int curr_row = _privListNotifications.indexOf(_selectedNotification);
    _notifyRulesSelection->setCurrentIndex(_notifyRulesModel->index(curr_row, 0, QModelIndex()),
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    return optionsPageWidget;
}

void NotifyPluginOptionsPage::apply()
{
    getOptionsPageValues(_owner->getCurrentNotification());
    _owner->setEnableSound(_optionsPage->chkEnableSound->isChecked());
    emit updateNotifications(_privListNotifications);
}

void NotifyPluginOptionsPage::finish()
{
    disconnect(_optionsPage->UAVObjectField, SIGNAL(currentIndexChanged(QString)),
               this, SLOT(on_changedIndex_UAVField(QString)));

    disconnect(_testSound.data(),SIGNAL(stateChanged(Phonon::State,Phonon::State)),
               this,SLOT(on_changed_playButtonText(Phonon::State,Phonon::State)));
    if (_testSound) {
        _testSound->stop();
        _testSound->clear();
    }
}

void NotifyPluginOptionsPage::initButtons()
{
    _optionsPage->chkEnableSound->setChecked(_owner->getEnableSound());
    connect(_optionsPage->chkEnableSound, SIGNAL(toggled(bool)),
            this, SLOT(on_toggled_checkEnableSound(bool)));

    _optionsPage->buttonModify->setEnabled(false);
    _optionsPage->buttonDelete->setEnabled(false);
    _optionsPage->buttonPlayNotification->setEnabled(false);
    connect(_optionsPage->buttonAdd, SIGNAL(pressed()),
            this, SLOT(on_clicked_buttonAddNotification()));
    connect(_optionsPage->buttonDelete, SIGNAL(pressed()),
            this, SLOT(on_clicked_buttonDeleteNotification()));
    connect(_optionsPage->buttonModify, SIGNAL(pressed()),
            this, SLOT(on_clicked_buttonModifyNotification()));
    connect(_optionsPage->buttonPlayNotification, SIGNAL(clicked()),
            this, SLOT(on_clicked_buttonTestSoundNotification()));
}

void NotifyPluginOptionsPage::initPhononPlayer()
{
    _testSound.reset(Phonon::createPlayer(Phonon::NotificationCategory));
    connect(_testSound.data(),SIGNAL(stateChanged(Phonon::State,Phonon::State)),
        this,SLOT(on_changed_playButtonText(Phonon::State,Phonon::State)));
    connect(_testSound.data(), SIGNAL(finished(void)), this, SLOT(on_FinishedPlaying(void)));
}

void NotifyPluginOptionsPage::initRulesTable()
{
    qNotifyDebug_if(_notifyRulesModel.isNull()) << "_notifyRulesModel.isNull())";
    qNotifyDebug_if(!_notifyRulesSelection) << "_notifyRulesSelection.isNull())";
    _notifyRulesModel.reset(new NotifyTableModel(_privListNotifications));
    _notifyRulesSelection = new QItemSelectionModel(_notifyRulesModel.data());

    connect(_notifyRulesSelection, SIGNAL(selectionChanged ( const QItemSelection &, const QItemSelection & )),
            this, SLOT(on_changedSelection_notifyTable( const QItemSelection & , const QItemSelection & )));
    connect(this, SIGNAL(entryUpdated(int)),
            _notifyRulesModel.data(), SLOT(entryUpdated(int)));

    _optionsPage->notifyRulesView->setModel(_notifyRulesModel.data());
    _optionsPage->notifyRulesView->setSelectionModel(_notifyRulesSelection);
    _optionsPage->notifyRulesView->setItemDelegate(new NotifyItemDelegate(this));

    _optionsPage->notifyRulesView->resizeRowsToContents();
    _optionsPage->notifyRulesView->setColumnWidth(eMessageName,200);
    _optionsPage->notifyRulesView->setColumnWidth(eRepeatValue,120);
    _optionsPage->notifyRulesView->setColumnWidth(eExpireTimer,100);
    _optionsPage->notifyRulesView->setColumnWidth(eTurnOn,60);
    _optionsPage->notifyRulesView->setDragEnabled(true);
    _optionsPage->notifyRulesView->setAcceptDrops(true);
    _optionsPage->notifyRulesView->setDropIndicatorShown(true);
    _optionsPage->notifyRulesView->setDragDropMode(QAbstractItemView::InternalMove);
}

UAVObjectField* NotifyPluginOptionsPage::getObjectFieldFromSelected()
{
    return (_currUAVObject) ? _currUAVObject->getField(_selectedNotification->getObjectField()) : NULL;
}

void NotifyPluginOptionsPage::setSelectedNotification(NotificationItem* ntf)
{
    _selectedNotification = ntf;
    _currUAVObject = dynamic_cast<UAVDataObject*>(_objManager.getObject(_selectedNotification->getDataObject()));
    if(!_currUAVObject) {
        qNotifyDebug() << "no such UAVObject: " << _selectedNotification->getDataObject();
    }
}

void NotifyPluginOptionsPage::addDynamicFieldLayout()
{
    // there is no need to check (objField == NULL),
    // thus it doesn't use in this field directly

    QSizePolicy labelSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    labelSizePolicy.setHorizontalStretch(0);
    labelSizePolicy.setVerticalStretch(0);
//    labelSizePolicy.setHeightForWidth(UAVObject->sizePolicy().hasHeightForWidth());


    QLabel* labelSayOrder = new QLabel("Say order ", _form);
    labelSayOrder->setSizePolicy(labelSizePolicy);

    _optionsPage->dynamicValueLayout->addWidget(labelSayOrder);
    _sayOrder = new QComboBox(_form);
    _optionsPage->dynamicValueLayout->addWidget(_sayOrder);
    _sayOrder->addItems(NotificationItem::sayOrderValues);

    QLabel* labelValueIs = new QLabel("Value is ", _form);
    labelValueIs->setSizePolicy(labelSizePolicy);
    _optionsPage->dynamicValueLayout->addWidget(labelValueIs);

    _dynamicFieldCondition = new QComboBox(_form);
    _optionsPage->dynamicValueLayout->addWidget(_dynamicFieldCondition);
    UAVObjectField* field = getObjectFieldFromSelected();
    addDynamicField(field);
}

void NotifyPluginOptionsPage::addDynamicField(UAVObjectField* objField)
{
    if(!objField) {
        qNotifyDebug() << "addDynamicField | input objField == NULL";
        return;
    }
    if (objField->getType() == _dynamicFieldType) {
        if (QComboBox* fieldValue = dynamic_cast<QComboBox*>(_dynamicFieldWidget)) {
            fieldValue->clear();
            QStringList enumValues(objField->getOptions());
            fieldValue->addItems(enumValues);
        }
        return;
    }

    disconnect(_dynamicFieldCondition, SIGNAL(currentIndexChanged(QString)),
               this, SLOT(on_changedIndex_rangeValue(QString)));

    _dynamicFieldCondition->clear();
    _dynamicFieldCondition->addItems(NotifyPluginOptionsPage::conditionValues);
    if (UAVObjectField::ENUM == objField->getType()) {
        _dynamicFieldCondition->removeItem(smaller);
        _dynamicFieldCondition->removeItem(bigger);
    }
    int cond=_selectedNotification->getCondition();
    if(cond<0)
        return;
    _dynamicFieldCondition->setCurrentIndex(_dynamicFieldCondition->findText(NotifyPluginOptionsPage::conditionValues.at(cond)));

            connect(_dynamicFieldCondition, SIGNAL(currentIndexChanged(QString)),
                    this, SLOT(on_changedIndex_rangeValue(QString)));

    addDynamicFieldWidget(objField);
}

void NotifyPluginOptionsPage::addDynamicFieldWidget(UAVObjectField* objField)
{
    if(!objField) {
        qNotifyDebug() << "objField == NULL!";
        return;
    }
    // check if dynamic fileld already settled,
    // so if its exists remove it and install new field
    if (_dynamicFieldWidget) {
        _optionsPage->dynamicValueLayout->removeWidget(_dynamicFieldWidget);
        delete _dynamicFieldWidget;
        _dynamicFieldWidget = NULL;
    }

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    _dynamicFieldType = objField->getType();
    switch(_dynamicFieldType)
    {
    case UAVObjectField::ENUM:
        {
            _dynamicFieldWidget = new QComboBox(_form);
            QStringList enumValues(objField->getOptions());
            (dynamic_cast<QComboBox*>(_dynamicFieldWidget))->addItems(enumValues);
        }
        break;

    default:
        Q_ASSERT(_dynamicFieldCondition);
        if (NotifyPluginOptionsPage::conditionValues.indexOf(_dynamicFieldCondition->currentText()) == NotifyPluginOptionsPage::inrange) {
            _dynamicFieldWidget = new QLineEdit(_form);

            (static_cast<QLineEdit*>(_dynamicFieldWidget))->setInputMask("#999.99 : #999.99;");
            (static_cast<QLineEdit*>(_dynamicFieldWidget))->setText("0000000000");
            (static_cast<QLineEdit*>(_dynamicFieldWidget))->setCursorPosition(0);
        } else {
            _dynamicFieldWidget = new QDoubleSpinBox(_form);
            (dynamic_cast<QDoubleSpinBox*>(_dynamicFieldWidget))->setRange(-999.99, 999.99);
        }
        break;
    };
    enum { eDynamicFieldWidth = 100 };
    _dynamicFieldWidget->setSizePolicy(sizePolicy);
    _dynamicFieldWidget->setFixedWidth(eDynamicFieldWidth);
    _optionsPage->dynamicValueLayout->addWidget(_dynamicFieldWidget);
}

void NotifyPluginOptionsPage::setDynamicFieldValue(NotificationItem* notification)
{
    if (QDoubleSpinBox* seValue = dynamic_cast<QDoubleSpinBox*>(_dynamicFieldWidget))
        seValue->setValue(notification->singleValue().toDouble());
    else {
        if (QComboBox* cbValue = dynamic_cast<QComboBox*>(_dynamicFieldWidget)) {
            int idx = cbValue->findText(notification->singleValue().toString());
            if(-1 != idx)
                cbValue->setCurrentIndex(idx);
        } else {
            if (QLineEdit* rangeValue = dynamic_cast<QLineEdit*>(_dynamicFieldWidget)) {
                QString str = QString("%1%2").arg(notification->singleValue().toDouble(), 5, 'f', 2, '0')
                        .arg(notification->valueRange2(), 5, 'f', 2, '0');
                rangeValue->setText(str);
            } else {
                qNotifyDebug() << "NotifyPluginOptionsPage::setDynamicValueField | unknown _fieldValue: " << _dynamicFieldWidget;
            }
        }
    }
}

void NotifyPluginOptionsPage::resetFieldType()
{
    _dynamicFieldType = -1;
}

void NotifyPluginOptionsPage::getOptionsPageValues(NotificationItem* notification)
{
    Q_ASSERT(notification);
    notification->setSoundCollectionPath(_optionsPage->SoundDirectoryPathChooser->path());
    notification->setCurrentLanguage(_optionsPage->SoundCollectionList->currentText());
    notification->setDataObject(_optionsPage->UAVObject->currentText());
    notification->setObjectField(_optionsPage->UAVObjectField->currentText());
    notification->setSound1(_optionsPage->Sound1->currentText());
    notification->setSound2(_optionsPage->Sound2->currentText());
    notification->setSound3(_optionsPage->Sound3->currentText());
    notification->setSayOrder(_sayOrder->currentIndex());
    notification->setCondition(NotifyPluginOptionsPage::conditionValues.indexOf(_dynamicFieldCondition->currentText()));
    if (QDoubleSpinBox* spinValue = dynamic_cast<QDoubleSpinBox*>(_dynamicFieldWidget))
        notification->setSingleValue(spinValue->value());
    else {
        if (QComboBox* comboBoxValue = dynamic_cast<QComboBox*>(_dynamicFieldWidget))
            notification->setSingleValue(comboBoxValue->currentText());
        else {
            if (QLineEdit* rangeValue = dynamic_cast<QLineEdit*>(_dynamicFieldWidget)) {
                QString str = rangeValue->text();
                QStringList range = str.split(':');
                notification->setSingleValue(range.at(0).toDouble());
                notification->setValueRange2(range.at(1).toDouble());
            }
        }
    }
}

void NotifyPluginOptionsPage::updateConfigView(NotificationItem* notification)
{
    Q_ASSERT(notification);
    disconnect(_optionsPage->UAVObject, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(on_changedIndex_UAVObject(QString)));
    disconnect(_optionsPage->UAVObjectField, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(on_changedIndex_UAVField(QString)));

    QString path = notification->getSoundCollectionPath();
    if (path.isEmpty()) {
        path = Utils::PathUtils().InsertDataPath("%%DATAPATH%%sounds");
    }

    _optionsPage->SoundDirectoryPathChooser->setPath(path);

    if (-1 != _optionsPage->SoundCollectionList->findText(notification->getCurrentLanguage())) {
        _optionsPage->SoundCollectionList->setCurrentIndex(_optionsPage->SoundCollectionList->findText(notification->getCurrentLanguage()));
    } else {
        _optionsPage->SoundCollectionList->setCurrentIndex(_optionsPage->SoundCollectionList->findText("default"));
    }

    // Fills the combo boxes for the UAVObjects
    QList< QList<UAVDataObject*> > objList = _objManager.getDataObjects();
    foreach (QList<UAVDataObject*> list, objList) {
        foreach (UAVDataObject* obj, list) {
            _optionsPage->UAVObject->addItem(obj->getName());
        }
    }

    if (-1 != _optionsPage->UAVObject->findText(notification->getDataObject())) {
        _optionsPage->UAVObject->setCurrentIndex(_optionsPage->UAVObject->findText(notification->getDataObject()));
    }

    _optionsPage->UAVObjectField->clear();
    if(_currUAVObject) {
        QList<UAVObjectField*> fieldList = _currUAVObject->getFields();
        foreach (UAVObjectField* field, fieldList)
            _optionsPage->UAVObjectField->addItem(field->getName());
    }

    if (-1 != _optionsPage->UAVObjectField->findText(notification->getObjectField())) {
        _optionsPage->UAVObjectField->setCurrentIndex(_optionsPage->UAVObjectField->findText(notification->getObjectField()));
    }

    if (-1 != _optionsPage->Sound1->findText(notification->getSound1())) {
        _optionsPage->Sound1->setCurrentIndex(_optionsPage->Sound1->findText(notification->getSound1()));
    } else {
        // show item from default location
        _optionsPage->SoundCollectionList->setCurrentIndex(_optionsPage->SoundCollectionList->findText("default"));
        _optionsPage->Sound1->setCurrentIndex(_optionsPage->Sound1->findText(notification->getSound1()));
    }

    if (-1 != _optionsPage->Sound2->findText(notification->getSound2())) {
        _optionsPage->Sound2->setCurrentIndex(_optionsPage->Sound2->findText(notification->getSound2()));
    } else {
        // show item from default location
        _optionsPage->SoundCollectionList->setCurrentIndex(_optionsPage->SoundCollectionList->findText("default"));
        _optionsPage->Sound2->setCurrentIndex(_optionsPage->Sound2->findText(notification->getSound2()));
    }

    if (-1 != _optionsPage->Sound3->findText(notification->getSound3())) {
        _optionsPage->Sound3->setCurrentIndex(_optionsPage->Sound3->findText(notification->getSound3()));
    } else {
        // show item from default location
        _optionsPage->SoundCollectionList->setCurrentIndex(_optionsPage->SoundCollectionList->findText("default"));
        _optionsPage->Sound3->setCurrentIndex(_optionsPage->Sound3->findText(notification->getSound3()));
    }
    int cond=notification->getCondition();
    if(cond<0)
        return;
    _dynamicFieldCondition->setCurrentIndex(_dynamicFieldCondition->findText(NotifyPluginOptionsPage::conditionValues.at(cond)));

    _sayOrder->setCurrentIndex(notification->getSayOrder());

    setDynamicFieldValue(notification);

    connect(_optionsPage->UAVObject, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(on_changedIndex_UAVObject(QString)));
    connect(_optionsPage->UAVObjectField, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(on_changedIndex_UAVField(QString)));

}

void NotifyPluginOptionsPage::on_changedIndex_rangeValue(QString /* rangeStr */)
{
    Q_ASSERT(_dynamicFieldWidget);
    UAVObjectField* field = getObjectFieldFromSelected();
    Q_ASSERT(!!field);
    addDynamicFieldWidget(field);
    setDynamicFieldValue(_selectedNotification);
}

void NotifyPluginOptionsPage::on_changedIndex_UAVField(QString field)
{
    resetFieldType();
    Q_ASSERT(_currUAVObject);
    addDynamicField(_currUAVObject->getField(field));

}

void NotifyPluginOptionsPage::on_changedIndex_UAVObject(QString val)
{
    resetFieldType();
    _currUAVObject = dynamic_cast<UAVDataObject*>( _objManager.getObject(val) );
    if(!_currUAVObject) {
        qNotifyDebug() << "on_UAVObject_indexChanged | no such UAVOBject";
        return;
    }
    QList<UAVObjectField*> fieldList = _currUAVObject->getFields();
    disconnect(_optionsPage->UAVObjectField, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_changedIndex_UAVField(QString)));
    _optionsPage->UAVObjectField->clear();
    foreach (UAVObjectField* field, fieldList) {
            _optionsPage->UAVObjectField->addItem(field->getName());
    }
    connect(_optionsPage->UAVObjectField, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_changedIndex_UAVField(QString)));
    _selectedNotification->setObjectField(fieldList.at(0)->getName());
    addDynamicField(fieldList.at(0));
}


void NotifyPluginOptionsPage::on_changedIndex_soundLanguage(int index)
{
    _optionsPage->SoundCollectionList->setCurrentIndex(index);
    QString collectionPath = _optionsPage->SoundDirectoryPathChooser->path()
        + QDir::toNativeSeparators("/" + _optionsPage->SoundCollectionList->currentText());

    QDir dirPath(collectionPath);
    QStringList filters;
    filters << "*.mp3" << "*.wav";
    dirPath.setNameFilters(filters);
    QStringList listSoundFiles = dirPath.entryList(filters);
    listSoundFiles.replaceInStrings(QRegExp(".mp3|.wav"), "");
    _optionsPage->Sound1->clear();
    _optionsPage->Sound2->clear();
    _optionsPage->Sound3->clear();
    _optionsPage->Sound1->addItem("");
    _optionsPage->Sound1->addItems(listSoundFiles);
    _optionsPage->Sound2->addItem("");
    _optionsPage->Sound2->addItems(listSoundFiles);
    _optionsPage->Sound3->addItem("");
    _optionsPage->Sound3->addItems(listSoundFiles);
}


void  NotifyPluginOptionsPage::on_changed_playButtonText(Phonon::State newstate, Phonon::State /* oldstate */)
{
    //Q_ASSERT(Phonon::ErrorState != newstate);

    if (newstate  == Phonon::PausedState || newstate  == Phonon::StoppedState) {
        _optionsPage->buttonPlayNotification->setText("Play");
        _optionsPage->buttonPlayNotification->setIcon(QPixmap(":/notify/images/play.png"));
    } else {
        if (newstate  == Phonon::PlayingState) {
            _optionsPage->buttonPlayNotification->setText("Stop");
            _optionsPage->buttonPlayNotification->setIcon(QPixmap(":/notify/images/stop.png"));
        }
    }
}

void NotifyPluginOptionsPage::on_changedSelection_notifyTable(const QItemSelection & selected, const QItemSelection & /* deselected */ )
{
    bool select = false;
    _testSound->stop();
    if (selected.indexes().size()) {
        select = true;
        setSelectedNotification(_privListNotifications.at(selected.indexes().at(0).row()));
        UAVObjectField* field = getObjectFieldFromSelected();
        addDynamicField(field);
        updateConfigView(_selectedNotification);
    }

    _optionsPage->buttonModify->setEnabled(select);
    _optionsPage->buttonDelete->setEnabled(select);
    _optionsPage->buttonPlayNotification->setEnabled(select);
}

void NotifyPluginOptionsPage::on_clicked_buttonSoundFolder(const QString& path)
{
    QDir dirPath(path);
    QStringList listDirCollections = dirPath.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    _optionsPage->SoundCollectionList->clear();
    _optionsPage->SoundCollectionList->addItems(listDirCollections);
}

void NotifyPluginOptionsPage::on_clicked_buttonTestSoundNotification()
{
    NotificationItem* notification = NULL;
    qNotifyDebug() << "on_buttonTestSoundNotification_clicked";
    Q_ASSERT(-1 != _notifyRulesSelection->currentIndex().row());
    _testSound->clearQueue();
    notification = _privListNotifications.at(_notifyRulesSelection->currentIndex().row());
    QStringList sequence = notification->toSoundList();
    if (sequence.isEmpty()) {
        qNotifyDebug() << "message sequense is empty!";
        return;
    }
    foreach(QString item, sequence) {
        qNotifyDebug() << item;
        _testSound->enqueue(Phonon::MediaSource(item));
    }
    _testSound->play();
}

void NotifyPluginOptionsPage::on_clicked_buttonAddNotification()
{
    NotificationItem* notification = new NotificationItem;

    if (_optionsPage->SoundDirectoryPathChooser->path().isEmpty()) {
        QPalette textPalette=_optionsPage->SoundDirectoryPathChooser->palette();
        textPalette.setColor(QPalette::Normal,QPalette::Text, Qt::red);
        _optionsPage->SoundDirectoryPathChooser->setPalette(textPalette);
        _optionsPage->SoundDirectoryPathChooser->setPath("please select sound collection folder");
        delete notification;
        return;
    }
    getOptionsPageValues(notification);

    if ( ((!_optionsPage->Sound2->currentText().isEmpty()) && (_sayOrder->currentText()=="After second"))
         || ((!_optionsPage->Sound3->currentText().isEmpty()) && (_sayOrder->currentText()=="After third")) ) {
        delete notification;
        return;
    } else {
        notification->setSayOrder(_sayOrder->currentIndex());
    }

    _notifyRulesModel->entryAdded(notification);
    _notifyRulesSelection->setCurrentIndex(_notifyRulesModel->index(_privListNotifications.size()-1,0,QModelIndex()),
                                                                              QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void NotifyPluginOptionsPage::on_clicked_buttonDeleteNotification()
{
    _notifyRulesModel->removeRow(_notifyRulesSelection->currentIndex().row());
    if (!_notifyRulesModel->rowCount()
        && (_notifyRulesSelection->currentIndex().row() > 0
            && _notifyRulesSelection->currentIndex().row() < _notifyRulesModel->rowCount()) )
    {
        _optionsPage->buttonDelete->setEnabled(false);
        _optionsPage->buttonModify->setEnabled(false);
        _optionsPage->buttonPlayNotification->setEnabled(false);
    }
}

void NotifyPluginOptionsPage::on_clicked_buttonModifyNotification()
{
    NotificationItem* notification = new NotificationItem;
    getOptionsPageValues(notification);
    notification->setRetryValue(_privListNotifications.at(_notifyRulesSelection->currentIndex().row())->retryValue());
    notification->setLifetime(_privListNotifications.at(_notifyRulesSelection->currentIndex().row())->lifetime());
    notification->setMute(_privListNotifications.at(_notifyRulesSelection->currentIndex().row())->mute());

    _privListNotifications.replace(_notifyRulesSelection->currentIndex().row(),notification);
    entryUpdated(_notifyRulesSelection->currentIndex().row());
}

void  NotifyPluginOptionsPage::on_FinishedPlaying()
{
    _testSound->clear();
}

void NotifyPluginOptionsPage::on_toggled_checkEnableSound(bool state)
{
    bool state1 = 1^state;

    QList<Phonon::Path> listOutputs = _testSound->outputPaths();
    Phonon::AudioOutput * audioOutput = (Phonon::AudioOutput*)listOutputs.last().sink();
    audioOutput->setMuted(state1);
}
