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

static const char* cStrEqualTo = "Equal to";
static const char* cStrLargeThan = "Large than";
static const char* cStrLowerThan = "Lower than";
static const char* cStrInRange = "In range";


NotifyPluginOptionsPage::NotifyPluginOptionsPage(QObject *parent)
    : IOptionsPage(parent)
    , _objManager(*ExtensionSystem::PluginManager::instance()->getObject<UAVObjectManager>())
    , _owner(qobject_cast<SoundNotifyPlugin*>(parent))
    , _currentCollectionPath("")
    , _valueRange(NULL)
    , _sayOrder(NULL)
    , _fieldValue(NULL)
    , _fieldType(-1)
    , _form(NULL)
    , _selectedNotification(NULL)
{}

NotifyPluginOptionsPage::~NotifyPluginOptionsPage()
{}

QWidget *NotifyPluginOptionsPage::createPage(QWidget *parent)
{
    _optionsPage.reset(new Ui::NotifyPluginOptionsPage());
    //main widget
    QWidget* optionsPageWidget = new QWidget;
    _fieldValue = NULL;
    _valueRange = NULL;
    resetFieldType();
    //save ref to form, needed for binding dynamic fields in future
    _form = optionsPageWidget;
    //main layout
    _optionsPage->setupUi(optionsPageWidget);

    _listSoundFiles.clear();

    _optionsPage->SoundDirectoryPathChooser->setExpectedKind(Utils::PathChooser::Directory);
    _optionsPage->SoundDirectoryPathChooser->setPromptDialogTitle(tr("Choose sound collection directory"));

    connect(_optionsPage->SoundDirectoryPathChooser, SIGNAL(changed(const QString&)),
            this, SLOT(on_buttonSoundFolder_clicked(const QString&)));
    connect(_optionsPage->SoundCollectionList, SIGNAL(currentIndexChanged (int)),
            this, SLOT(on_soundLanguage_indexChanged(int)));

    connect(this, SIGNAL(updateNotifications(QList<NotificationItem*>)),
        _owner, SLOT(updateNotificationList(QList<NotificationItem*>)));
    //connect(this, SIGNAL(resetNotification()),owner, SLOT(resetNotification()));

    _privListNotifications = _owner->getListNotifications();


    // [1]
    _selectedNotification = _owner->getCurrentNotification();
    addDynamicValueLayout();
    // [2]
    updateConfigView(_selectedNotification);

    initRulesTable();
    initButtons();
    initPhononPlayer();

    _notifyRulesSelection->setCurrentIndex(_notifyRulesModel->index(0, 0, QModelIndex()),
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
               this, SLOT(on_UAVField_indexChanged(QString)));

    disconnect(_notifySound.data(),SIGNAL(stateChanged(Phonon::State,Phonon::State)),
               this,SLOT(on_changeButtonText(Phonon::State,Phonon::State)));
    if (_notifySound) {
        _notifySound->stop();
        _notifySound->clear();
    }
}

void NotifyPluginOptionsPage::addDynamicValueLayout()
{
    NotificationItem* curr = _owner->getCurrentNotification();
    Q_ASSERT(curr);
    _optionsPage->dynamicValueLayout->addWidget(new QLabel("Say order ", _form));

    _sayOrder = new QComboBox(_form);
    _optionsPage->dynamicValueLayout->addWidget(_sayOrder);
    _sayOrder->addItems(NotificationItem::sayOrderValues);

    _optionsPage->dynamicValueLayout->addWidget(new QLabel("Value is ", _form));

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(_objManager.getObject(curr->getDataObject()));
    UAVObjectField* field = obj->getField(curr->getObjectField());
    Q_ASSERT(obj);
    Q_ASSERT(field);
    _valueRange = new QComboBox(_form);
    _optionsPage->dynamicValueLayout->addWidget(_valueRange);

    addDynamicField(field);
}

void NotifyPluginOptionsPage::resetValueRange()
{
    (static_cast<QLineEdit*>(_fieldValue))->setInputMask("999.99 - 999.99;");
    (static_cast<QLineEdit*>(_fieldValue))->setText("0000000000");
    (static_cast<QLineEdit*>(_fieldValue))->setCursorPosition(0);
}

void NotifyPluginOptionsPage::on_rangeValue_indexChanged(QString rangeStr)
{
    Q_ASSERT(_fieldValue);
    UAVObjectField* field = getObjectFieldFromPage();
    Q_ASSERT(!!field);
    setDynamicValueWidget(field);
    setDynamicValueField(_selectedNotification);
}

void NotifyPluginOptionsPage::resetFieldType()
{
    _fieldType = -1;
}

void NotifyPluginOptionsPage::addDynamicField(UAVObjectField* objField)
{
    //qDebugNotify_ if (!objField || !parent) << "null input params";
    Q_ASSERT(objField);
     if (objField->getType() == _fieldType) {
        if (QComboBox* fieldValue = dynamic_cast<QComboBox*>(_fieldValue)) {
            fieldValue->clear();
            QStringList enumValues(objField->getOptions());
            fieldValue->addItems(enumValues);
        }
        return;
    }

    disconnect(_valueRange, SIGNAL(currentIndexChanged(QString)),
        this, SLOT(on_rangeValue_indexChanged(QString)));

    _valueRange->clear();
    QStringList rangeValues;
    if (UAVObjectField::ENUM == objField->getType()) {
        rangeValues << cStrEqualTo << cStrInRange;
        _valueRange->addItems(rangeValues);
        _valueRange->setCurrentIndex(rangeValues.indexOf(_selectedNotification->range()));

    } else {
        rangeValues << cStrEqualTo << cStrLargeThan << cStrLowerThan << cStrInRange;
        _valueRange->addItems(rangeValues);
        connect(_valueRange, SIGNAL(currentIndexChanged(QString)),
                this, SLOT(on_rangeValue_indexChanged(QString)));
    }

    setDynamicValueWidget(objField);
}

void NotifyPluginOptionsPage::setDynamicValueWidget(UAVObjectField* objField)
{
    Q_ASSERT(_valueRange);

    // check if dynamic fileld already settled,
    // so if its exists remove it and install new field
    if (_fieldValue) {
        _optionsPage->dynamicValueLayout->removeWidget(_fieldValue);
        delete _fieldValue;
        _fieldValue = NULL;
    }
    _fieldType = objField->getType();
    switch(_fieldType)
    {
    case UAVObjectField::ENUM:
        {
            _fieldValue = new QComboBox(_form);
            QStringList enumValues(objField->getOptions());
            (dynamic_cast<QComboBox*>(_fieldValue))->addItems(enumValues);
        }
        break;

    default:
        if (_valueRange->currentText() == cStrInRange) {
            _fieldValue = new QLineEdit(_form);
            resetValueRange();
        } else {
            _fieldValue = new QSpinBox(_form);
        }
        break;
    };
    _optionsPage->dynamicValueLayout->addWidget(_fieldValue);
}

void NotifyPluginOptionsPage::initButtons()
{
    _optionsPage->chkEnableSound->setChecked(_owner->getEnableSound());
    connect(_optionsPage->chkEnableSound, SIGNAL(toggled(bool)),
            this, SLOT(on_checkEnableSound_toggled(bool)));

    _optionsPage->buttonModify->setEnabled(false);
    _optionsPage->buttonDelete->setEnabled(false);
    _optionsPage->buttonPlayNotification->setEnabled(false);
    connect(_optionsPage->buttonAdd, SIGNAL(pressed()),
            this, SLOT(on_button_AddNotification_clicked()));
    connect(_optionsPage->buttonDelete, SIGNAL(pressed()),
            this, SLOT(on_button_DeleteNotification_clicked()));
    connect(_optionsPage->buttonModify, SIGNAL(pressed()),
            this, SLOT(on_button_ModifyNotification_clicked()));
    connect(_optionsPage->buttonPlayNotification, SIGNAL(clicked()),
            this, SLOT(on_button_TestSoundNotification_clicked()));
}

void NotifyPluginOptionsPage::initPhononPlayer()
{
    _notifySound.reset(Phonon::createPlayer(Phonon::NotificationCategory));
    connect(_notifySound.data(),SIGNAL(stateChanged(Phonon::State,Phonon::State)),
        this,SLOT(on_changeButtonText(Phonon::State,Phonon::State)));
    connect(_notifySound.data(), SIGNAL(finished(void)), this, SLOT(on_FinishedPlaying(void)));
}

void NotifyPluginOptionsPage::initRulesTable()
{
    qNotifyDebug_if(_notifyRulesModel.isNull()) << "_notifyRulesModel.isNull())";
    qNotifyDebug_if(!_notifyRulesSelection) << "_notifyRulesSelection.isNull())";
    _notifyRulesModel.reset(new NotifyTableModel(_privListNotifications));
    _notifyRulesSelection = new QItemSelectionModel(_notifyRulesModel.data());

    connect(_notifyRulesSelection, SIGNAL(selectionChanged ( const QItemSelection &, const QItemSelection & )),
            this, SLOT(on_table_changeSelection( const QItemSelection & , const QItemSelection & )));
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
    notification->setSayOrder(_sayOrder->currentText());
    notification->setRange(_valueRange->currentText());
    if (QSpinBox* spinValue = dynamic_cast<QSpinBox*>(_fieldValue))
        notification->setSingleValue(spinValue->value());
    else {
        if (QComboBox* comboBoxValue = dynamic_cast<QComboBox*>(_fieldValue))
            notification->setSingleValue(comboBoxValue->currentIndex());
        else {
            if (QLineEdit* rangeValue = dynamic_cast<QLineEdit*>(_fieldValue)) {
                QString str = rangeValue->text();
                QStringList range = str.split('-');
                notification->setSingleValue(range.at(0).toDouble());
                notification->setValueRange2(range.at(1).toDouble());
            }
        }
    }
}

void NotifyPluginOptionsPage::on_UAVField_indexChanged(QString field)
{
    resetFieldType();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( _objManager.getObject(_optionsPage->UAVObject->currentText()));
    addDynamicField(obj->getField(field));

}

void NotifyPluginOptionsPage::on_UAVObject_indexChanged(QString val)
{
    resetFieldType();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( _objManager.getObject(val) );
    QList<UAVObjectField*> fieldList = obj->getFields();
    disconnect(_optionsPage->UAVObjectField, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_UAVField_indexChanged(QString)));
    _optionsPage->UAVObjectField->clear();
    foreach (UAVObjectField* field, fieldList) {
        _optionsPage->UAVObjectField->addItem(field->getName());
    }
    connect(_optionsPage->UAVObjectField, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_UAVField_indexChanged(QString)));
    addDynamicField(fieldList.at(0));
}

void NotifyPluginOptionsPage::on_buttonSoundFolder_clicked(const QString& path)
{
    QDir dirPath(path);
    _listDirCollections = dirPath.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    _optionsPage->SoundCollectionList->clear();
    _optionsPage->SoundCollectionList->addItems(_listDirCollections);
}

void NotifyPluginOptionsPage::on_soundLanguage_indexChanged(int index)
{
    _optionsPage->SoundCollectionList->setCurrentIndex(index);
    _currentCollectionPath = _optionsPage->SoundDirectoryPathChooser->path()
        + QDir::toNativeSeparators("/" + _optionsPage->SoundCollectionList->currentText());

    QDir dirPath(_currentCollectionPath);
    QStringList filters;
    filters << "*.mp3" << "*.wav";
    dirPath.setNameFilters(filters);
    _listSoundFiles = dirPath.entryList(filters);
    _listSoundFiles.replaceInStrings(QRegExp(".mp3|.wav"), "");
    _optionsPage->Sound1->clear();
    _optionsPage->Sound2->clear();
    _optionsPage->Sound3->clear();
    _optionsPage->Sound1->addItems(_listSoundFiles);
    _optionsPage->Sound2->addItem("");
    _optionsPage->Sound2->addItems(_listSoundFiles);
    _optionsPage->Sound3->addItem("");
    _optionsPage->Sound3->addItems(_listSoundFiles);
}

void  NotifyPluginOptionsPage::on_changeButtonText(Phonon::State newstate, Phonon::State oldstate)
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

void  NotifyPluginOptionsPage::on_FinishedPlaying()
{
    _notifySound->clear();
}

void NotifyPluginOptionsPage::on_button_TestSoundNotification_clicked()
{
    NotificationItem* notification = NULL;
    qNotifyDebug() << "on_buttonTestSoundNotification_clicked";
    Q_ASSERT(-1 != _notifyRulesSelection->currentIndex().row());
    _notifySound->clearQueue();
    notification = _privListNotifications.at(_notifyRulesSelection->currentIndex().row());
    QStringList sequence = notification->toSoundList();
    if (sequence.isEmpty()) {
        qNotifyDebug() << "message sequense is empty!";
        return;
    }
    foreach(QString item, sequence) {
        qNotifyDebug() << item;
        _notifySound->enqueue(Phonon::MediaSource(item));
    }
    _notifySound->play();
}

void NotifyPluginOptionsPage::on_checkEnableSound_toggled(bool state)
{
    bool state1 = 1^state;

    QList<Phonon::Path> listOutputs = _notifySound->outputPaths();
    Phonon::AudioOutput * audioOutput = (Phonon::AudioOutput*)listOutputs.last().sink();
    audioOutput->setMuted(state1);
}

void NotifyPluginOptionsPage::updateConfigView(NotificationItem* notification)
{
    Q_ASSERT(notification);
    disconnect(_optionsPage->UAVObject, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(on_UAVObject_indexChanged(QString)));
    disconnect(_optionsPage->UAVObjectField, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(on_UAVField_indexChanged(QString)));

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
    QString uavDataObject = notification->getDataObject();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(_objManager.getObject(uavDataObject));
    if (obj != NULL ) {
        QList<UAVObjectField*> fieldList = obj->getFields();
        foreach (UAVObjectField* field, fieldList) {
            _optionsPage->UAVObjectField->addItem(field->getName());
        }
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

    if (-1 != _valueRange->findText(notification->range())) {
        _valueRange->setCurrentIndex(_valueRange->findText(notification->range()));
    }

    if (-1 != _sayOrder->findText(notification->getSayOrder())) {
        _sayOrder->setCurrentIndex(_sayOrder->findText(notification->getSayOrder()));
    }

    setDynamicValueField(notification);

    connect(_optionsPage->UAVObject, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(on_UAVObject_indexChanged(QString)));
    connect(_optionsPage->UAVObjectField, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(on_UAVField_indexChanged(QString)));

}

void NotifyPluginOptionsPage::setDynamicValueField(NotificationItem* notification)
{
    if (QSpinBox* spinValue = dynamic_cast<QSpinBox*>(_fieldValue))
        spinValue->setValue(notification->singleValue());
    else {
        if (QComboBox* comboBoxValue = dynamic_cast<QComboBox*>(_fieldValue))
            comboBoxValue->setCurrentIndex(notification->singleValue());
        else {
            if (QLineEdit* rangeValue = dynamic_cast<QLineEdit*>(_fieldValue)) {
                //resetValueRange();
                QString str = QString("%1%2").arg(notification->singleValue(), 5, 'f', 2, '0')
                        .arg(notification->valueRange2(), 5, 'f', 2, '0');
                rangeValue->setText(str);
            } else {
                qNotifyDebug() << "NotifyPluginOptionsPage::setDynamicValueField | unknown _fieldValue: " << _fieldValue;
            }
        }
    }
}

UAVObjectField* NotifyPluginOptionsPage::getObjectFieldFromPage()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( _objManager.getObject(_optionsPage->UAVObject->currentText()));
    return obj->getField(_optionsPage->UAVObjectField->currentText());
}


UAVObjectField* NotifyPluginOptionsPage::getObjectFieldFromSelected()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(_objManager.getObject(_selectedNotification->getDataObject()));
    return obj->getField(_selectedNotification->getObjectField());
}

void NotifyPluginOptionsPage::on_table_changeSelection( const QItemSelection & selected, const QItemSelection & deselected )
{
    bool select = false;
    _notifySound->stop();
    if (selected.indexes().size()) {
        select = true;
        _selectedNotification = _privListNotifications.at(selected.indexes().at(0).row());
        UAVObjectField* field = getObjectFieldFromSelected();
        addDynamicField(field);
        updateConfigView(_selectedNotification);
    }

    _optionsPage->buttonModify->setEnabled(select);
    _optionsPage->buttonDelete->setEnabled(select);
    _optionsPage->buttonPlayNotification->setEnabled(select);
}

void NotifyPluginOptionsPage::on_button_AddNotification_clicked()
{
    NotificationItem* notification = new NotificationItem;

    if (_optionsPage->SoundDirectoryPathChooser->path().isEmpty()) {
        QPalette textPalette=_optionsPage->SoundDirectoryPathChooser->palette();
        textPalette.setColor(QPalette::Normal,QPalette::Text, Qt::red);
        _optionsPage->SoundDirectoryPathChooser->setPalette(textPalette);
        _optionsPage->SoundDirectoryPathChooser->setPath("please select sound collection folder");
        return;
    }

    notification->setSoundCollectionPath(_optionsPage->SoundDirectoryPathChooser->path());
    notification->setCurrentLanguage(_optionsPage->SoundCollectionList->currentText());
    notification->setDataObject(_optionsPage->UAVObject->currentText());
    notification->setObjectField(_optionsPage->UAVObjectField->currentText());
    notification->setRange(_valueRange->currentText());

    if (QSpinBox* spinValue = dynamic_cast<QSpinBox*>(_fieldValue))
        notification->setSingleValue(spinValue->value());

    if (_optionsPage->Sound1->currentText().size() > 0)
        notification->setSound1(_optionsPage->Sound1->currentText());

    notification->setSound2(_optionsPage->Sound2->currentText());
    notification->setSound3(_optionsPage->Sound3->currentText());

    if ( ((!_optionsPage->Sound2->currentText().size()) && (_sayOrder->currentText()=="After second"))
         || ((!_optionsPage->Sound3->currentText().size()) && (_sayOrder->currentText()=="After third")) ) {
            return;
    } else {
        notification->setSayOrder(_sayOrder->currentText());
    }

    _notifyRulesModel->entryAdded(notification);
    _notifyRulesSelection->setCurrentIndex(_notifyRulesModel->index(_privListNotifications.size()-1,0,QModelIndex()),
                                                                              QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void NotifyPluginOptionsPage::on_button_DeleteNotification_clicked()
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

void NotifyPluginOptionsPage::on_button_ModifyNotification_clicked()
{
    NotificationItem* notification = new NotificationItem;
    getOptionsPageValues(notification);
    notification->setRetryString(_privListNotifications.at(_notifyRulesSelection->currentIndex().row())->retryString());
    notification->setLifetime(_privListNotifications.at(_notifyRulesSelection->currentIndex().row())->lifetime());
    notification->setMute(_privListNotifications.at(_notifyRulesSelection->currentIndex().row())->mute());

    _privListNotifications.replace(_notifyRulesSelection->currentIndex().row(),notification);
    entryUpdated(_notifyRulesSelection->currentIndex().row());
}
