/**
 ******************************************************************************
 *
 * @file       notifypluginoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Sound Notify Plugin options page
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

#include "notifypluginoptionspage.h"
#include <coreplugin/icore.h>
#include "notifypluginconfiguration.h"
#include "ui_notifypluginoptionspage.h"
#include "extensionsystem/pluginmanager.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>
#include <QtCore/QSettings>
#include <QTableWidget>
#include <QPalette>
#include <QBuffer>

#include "notifyplugin.h"


NotifyPluginOptionsPage::NotifyPluginOptionsPage(/*NotifyPluginConfiguration *config,*/ QObject *parent) :
		IOptionsPage(parent),
		owner((SoundNotifyPlugin*)parent),
		currentCollectionPath(""),
		privListNotifications(((SoundNotifyPlugin*)parent)->getListNotifications())
{

}


//creates options page widget (uses the UI file)
QWidget *NotifyPluginOptionsPage::createPage(QWidget *parent)
{

	options_page = new Ui::NotifyPluginOptionsPage();
	//main widget
	QWidget *optionsPageWidget = new QWidget;
	//main layout
	options_page->setupUi(optionsPageWidget);

	options_page->chkEnableSound->setChecked(owner->getEnableSound());
	options_page->SoundDirectoryPathChooser->setExpectedKind(Utils::PathChooser::Directory);
	options_page->SoundDirectoryPathChooser->setPromptDialogTitle(tr("Choose sound collection directory"));
	options_page->tableNotifications->setRowCount(0);
	privListNotifications.clear();
	listSoundFiles.clear();

	settings = Core::ICore::instance()->settings();
	settings->beginGroup(QLatin1String("NotifyPlugin"));

	int size = settings->beginReadArray("listNotifies");
	for (int i = 0; i < size; ++i) {
		 settings->setArrayIndex(i);
		 notify = new NotifyPluginConfiguration;
		 notify->restoreState(settings);
		 privListNotifications.append(notify);
	}
	settings->endArray();

	settings->beginReadArray("Current");
	settings->setArrayIndex(0);
	notify = new NotifyPluginConfiguration;
	notify->restoreState(settings);
	settings->endArray();
	options_page->chkEnableSound->setChecked(settings->value(QLatin1String("EnableSound"),0).toBool());
	settings->endGroup();

	// Fills the combo boxes for the UAVObjects
	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	objManager = pm->getObject<UAVObjectManager>();
	QList< QList<UAVDataObject*> > objList = objManager->getDataObjects();
	foreach (QList<UAVDataObject*> list, objList) {
		foreach (UAVDataObject* obj, list) {
			options_page->UAVObject->addItem(obj->getName());
		}
	}

	connect(options_page->SoundDirectoryPathChooser, SIGNAL(changed(const QString&)), this, SLOT(on_buttonSoundFolder_clicked(const QString&)));
	connect(options_page->SoundCollectionList, SIGNAL(currentIndexChanged (int)), this, SLOT(on_soundLanguage_indexChanged(int)));
	connect(options_page->buttonAdd, SIGNAL(pressed()), this, SLOT(on_buttonAddNotification_clicked()));
	connect(options_page->buttonDelete, SIGNAL(pressed()), this, SLOT(on_buttonDeleteNotification_clicked()));
	connect(options_page->buttonModify, SIGNAL(pressed()), this, SLOT(on_buttonModifyNotification_clicked()));
	connect(options_page->buttonTestSound1, SIGNAL(clicked()), this, SLOT(on_buttonTestSound1_clicked()));
	connect(options_page->buttonTestSound2, SIGNAL(clicked()), this, SLOT(on_buttonTestSound2_clicked()));
	connect(options_page->buttonPlayNotification, SIGNAL(clicked()), this, SLOT(on_buttonTestSoundNotification_clicked()));
	connect(options_page->chkEnableSound, SIGNAL(toggled(bool)), this, SLOT(on_chkEnableSound_toggled(bool)));
	connect(options_page->tableNotifications, SIGNAL(itemSelectionChanged()), this, SLOT(on_tableNotification_changeSelection()));
	connect(options_page->UAVObject, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_UAVObject_indexChanged(QString)));
	connect(this, SIGNAL(updateNotifications()), owner, SLOT(connectNotifications()));

	//connect(options_page->chkEnableSound, SIGNAL(stateChanged(int)), this, SLOT(enableSound(int)));

	updateConfigView(notify);

	foreach(NotifyPluginConfiguration* notification,privListNotifications)
	{
		options_page->tableNotifications->setRowCount(options_page->tableNotifications->rowCount()+1);
		options_page->tableNotifications->setItem (options_page->tableNotifications->rowCount()-1,0,
											   new QTableWidgetItem(notification->parseNotifyMessage()));

	}
	options_page->buttonModify->setEnabled(false);
	options_page->buttonDelete->setEnabled(false);

	sound1 = Phonon::createPlayer(Phonon::NotificationCategory);
	sound2 = Phonon::createPlayer(Phonon::NotificationCategory);
	notifySound = Phonon::createPlayer(Phonon::NotificationCategory);
//	audioOutput = new Phonon::AudioOutput(Phonon::NotificationCategory, this);
//	Phonon::createPath(sound1, audioOutput);
//	Phonon::createPath(sound2, audioOutput);
//	Phonon::createPath(notifySound, audioOutput);

	connect(sound1,SIGNAL(stateChanged(Phonon::State,Phonon::State)),SLOT(changeButtonText(Phonon::State,Phonon::State)));
	connect(sound2,SIGNAL(stateChanged(Phonon::State,Phonon::State)),SLOT(changeButtonText(Phonon::State,Phonon::State)));
	connect(notifySound,SIGNAL(stateChanged(Phonon::State,Phonon::State)),SLOT(changeButtonText(Phonon::State,Phonon::State)));


	return optionsPageWidget;
}

void NotifyPluginOptionsPage::getOptionsPageValues()
{
	notify->setSoundCollectionPath(options_page->SoundDirectoryPathChooser->path());
	notify->setCurrentLanguage(options_page->SoundCollectionList->currentText());
	notify->setDataObject(options_page->UAVObject->currentText());
	notify->setObjectField(options_page->UAVObjectField->currentText());
	notify->setSound1(options_page->Sound1->currentText());
	notify->setSound2(options_page->Sound2->currentText());
	notify->setSayOrder(options_page->SayOrder->currentText());
	notify->setValue(options_page->Value->currentText());
	notify->setSpinBoxValue(options_page->ValueSpinBox->value());
}

////////////////////////////////////////////
// Called when the user presses apply or OK.
//
// Saves the current values
//
////////////////////////////////////////////
void NotifyPluginOptionsPage::apply()
{
	settings->beginGroup(QLatin1String("NotifyPlugin"));

	getOptionsPageValues();

	settings->beginWriteArray("Current");
	settings->setArrayIndex(0);
	notify->saveState(settings);
	settings->endArray();

	//settings->remove("listNotifies");

	settings->beginGroup("listNotifies");
	settings->remove("");
	settings->endGroup();

	settings->beginWriteArray("listNotifies");
	for (int i = 0; i < options_page->tableNotifications->rowCount(); i++) {
		settings->setArrayIndex(i);
		privListNotifications.at(i)->saveState(settings);
	}
	owner->setListNotifications(privListNotifications);
	settings->endArray();
	owner->setEnableSound(options_page->chkEnableSound->isChecked());
	settings->setValue(QLatin1String("EnableSound"), options_page->chkEnableSound->isChecked());
	settings->endGroup();

	emit updateNotifications();
}

void NotifyPluginOptionsPage::finish()
{
	delete options_page;
}



//////////////////////////////////////////////////////////////////////////////
//  Fills in the <Field> combo box when value is changed in the
//  <Object> combo box
//////////////////////////////////////////////////////////////////////////////
void NotifyPluginOptionsPage::on_UAVObject_indexChanged(QString val) {
	options_page->UAVObjectField->clear();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(val) );
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
		options_page->UAVObjectField->addItem(field->getName());
    }
}

// locate collection folder on disk
void NotifyPluginOptionsPage::on_buttonSoundFolder_clicked(const QString& path)
{
	//disconnect(options_page->SoundCollectionList, SIGNAL(currentIndexChanged (int)), this, SLOT(on_soundLanguage_indexChanged(int)));

	QDir dirPath(path);
	listDirCollections = dirPath.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
	options_page->SoundCollectionList->clear();
	options_page->SoundCollectionList->addItems(listDirCollections);

	//connect(options_page->SoundCollectionList, SIGNAL(currentIndexChanged (int)), this, SLOT(on_soundLanguage_indexChanged(int)));
}


void NotifyPluginOptionsPage::on_soundLanguage_indexChanged(int index)
{
	options_page->SoundCollectionList->setCurrentIndex(index);

	currentCollectionPath = options_page->SoundDirectoryPathChooser->path() +"\\"+options_page->SoundCollectionList->currentText();

	QDir dirPath(currentCollectionPath);
	QStringList filters;
	filters << "*.mp3" << "*.wav";
	dirPath.setNameFilters(filters);
	listSoundFiles = dirPath.entryList(filters);
	listSoundFiles.replaceInStrings(QRegExp(".mp3|.wav"), "");
	options_page->Sound1->clear();
	options_page->Sound2->clear();
	options_page->Sound1->addItems(listSoundFiles);
	options_page->Sound2->addItems(listSoundFiles);
	options_page->Sound2->addItem("");
}

void  NotifyPluginOptionsPage::changeButtonText(Phonon::State newstate, Phonon::State oldstate)
{
	if(sender()==sound1)
	{
		if(newstate  == Phonon::PausedState)
			options_page->buttonTestSound1->setText("Play");
		else
			if(newstate  == Phonon::PlayingState)
				options_page->buttonTestSound1->setText("Stop");
	}
	else
	{
		if(sender()==sound2)
		{
			if(newstate  == Phonon::PausedState)
				options_page->buttonTestSound2->setText("Play");
			else
				if(newstate  == Phonon::PlayingState)
					options_page->buttonTestSound2->setText("Stop");
		}
		else
		{
			if(sender()==notifySound)
			{
				if(newstate  == Phonon::PausedState)
					options_page->buttonPlayNotification->setText("Play");
				else
					if(newstate  == Phonon::PlayingState)
						options_page->buttonPlayNotification->setText("Stop");
			}
		}
	}
}


void NotifyPluginOptionsPage::on_buttonTestSound1_clicked()
{
	if(options_page->Sound1->currentText()=="") return;

	QFile file;
	QString currPath = options_page->SoundDirectoryPathChooser->path()+"\\"+
					   options_page->SoundCollectionList->currentText()+"\\"+options_page->Sound1->currentText()+".wav";
	//currPath.replace(QString("\\"), QString("/"));

//		file.setFileName(currPath);
//		file.open(QIODevice::ReadOnly);
//		QDataStream in(&file);    // read the data serialized from the file
//		QByteArray buf;
//		in >> buf;

	sound1->setCurrentSource(Phonon::MediaSource(currPath));
	sound1->play();
}

void NotifyPluginOptionsPage::on_buttonTestSound2_clicked()
{
	if(options_page->Sound2->currentText()=="") return;

	QFile file;
	QString currPath = options_page->SoundDirectoryPathChooser->path()+"\\"+
					   options_page->SoundCollectionList->currentText()+"\\"+options_page->Sound2->currentText()+".wav";
	//currPath.replace(QString("\\"), QString("/"));

	sound2->setCurrentSource(Phonon::MediaSource(currPath));
	sound2->play();
}

void NotifyPluginOptionsPage::on_buttonTestSoundNotification_clicked()
{
	QList <Phonon::MediaSource> messageNotify;
	//getOptionsPageValues();
	notify = privListNotifications.at(options_page->tableNotifications->currentRow());
	notify->parseNotifyMessage();
	QStringList stringList = notify->getNotifyMessageList();
	foreach(QString item, stringList)
		messageNotify.append(Phonon::MediaSource(item));
	notifySound->clear();
	notifySound->setQueue(messageNotify);
	notifySound->play();
}

void NotifyPluginOptionsPage::on_chkEnableSound_toggled(bool state)
{
	bool state1 = 1^state;
	{
		QList<Phonon::Path> listOutputs = sound1->outputPaths();
		Phonon::AudioOutput * audioOutput = (Phonon::AudioOutput*)listOutputs.last().sink();
		audioOutput->setMuted(state1);
	}
	{
		QList<Phonon::Path> listOutputs = sound2->outputPaths();
		Phonon::AudioOutput * audioOutput = (Phonon::AudioOutput*)listOutputs.last().sink();
		audioOutput->setMuted(state1);
	}
	{
		QList<Phonon::Path> listOutputs = notifySound->outputPaths();
		Phonon::AudioOutput * audioOutput = (Phonon::AudioOutput*)listOutputs.last().sink();
		audioOutput->setMuted(state1);
	}
}

void NotifyPluginOptionsPage::updateConfigView(NotifyPluginConfiguration* notification)
{

	options_page->SoundDirectoryPathChooser->setPath(notification->getSoundCollectionPath());

	if(options_page->SoundCollectionList->findText(notification->getCurrentLanguage())!=-1){
		options_page->SoundCollectionList->setCurrentIndex(options_page->SoundCollectionList->findText(notification->getCurrentLanguage()));
	}
	else
		options_page->SoundCollectionList->setCurrentIndex(options_page->SoundCollectionList->findText("default"));


	if(options_page->UAVObject->findText(notification->getDataObject())!=-1){
		options_page->UAVObject->setCurrentIndex(options_page->UAVObject->findText(notification->getDataObject()));
	}

	// Now load the object field values:
	options_page->UAVObjectField->clear();
	UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(notification->getDataObject()/*objList.at(0).at(0)->getName()*/) );
	if (obj != NULL ) {
		QList<UAVObjectField*> fieldList = obj->getFields();
		foreach (UAVObjectField* field, fieldList) {
			options_page->UAVObjectField->addItem(field->getName());
		}
	}

	if(options_page->UAVObjectField->findText(notification->getObjectField())!=-1){
		options_page->UAVObjectField->setCurrentIndex(options_page->UAVObjectField->findText(notification->getObjectField()));
	}

	if(options_page->Sound1->findText(notification->getSound1())!=-1){
		options_page->Sound1->setCurrentIndex(options_page->Sound1->findText(notification->getSound1()));
	}
	else
	{
		// show item from default location
		options_page->SoundCollectionList->setCurrentIndex(options_page->SoundCollectionList->findText("default"));
		options_page->Sound1->setCurrentIndex(options_page->Sound1->findText(notification->getSound1()));

		// don't show item if it wasn't find in stored location
		//options_page->Sound1->setCurrentIndex(-1);
	}

	if(options_page->Sound2->findText(notification->getSound2())!=-1){
		options_page->Sound2->setCurrentIndex(options_page->Sound2->findText(notification->getSound2()));
	}
	else
	{
		// show item from default location
		options_page->SoundCollectionList->setCurrentIndex(options_page->SoundCollectionList->findText("default"));
		options_page->Sound2->setCurrentIndex(options_page->Sound2->findText(notification->getSound2()));

		// don't show item if it wasn't find in stored location
		//options_page->Sound2->setCurrentIndex(-1);
	}
	if(options_page->Value->findText(notification->getValue())!=-1){
		options_page->Value->setCurrentIndex(options_page->Value->findText(notification->getValue()));
	}

	if(options_page->SayOrder->findText(notification->getSayOrder())!=-1){
		options_page->SayOrder->setCurrentIndex(options_page->SayOrder->findText(notification->getSayOrder()));
	}
	options_page->ValueSpinBox->setValue(notification->getSpinBoxValue());
}

void NotifyPluginOptionsPage::on_tableNotification_changeSelection()
{
	QTableWidgetItem * item = options_page->tableNotifications->currentItem();
	//qDebug()
	if(!item) return;
//	if(notify)
//		delete notify;
	//notify = );
	updateConfigView(privListNotifications.at(item->row()));
	options_page->buttonModify->setEnabled(item->isSelected());
	options_page->buttonDelete->setEnabled(item->isSelected());
}

void NotifyPluginOptionsPage::on_buttonAddNotification_clicked()
{
	if(options_page->SoundDirectoryPathChooser->path()=="")
	{
		QPalette textPalette=options_page->SoundDirectoryPathChooser->palette();
		textPalette.setColor(QPalette::Normal,QPalette::Text, Qt::red);
		options_page->SoundDirectoryPathChooser->setPalette(textPalette);
		options_page->SoundDirectoryPathChooser->setPath("please select sound collection folder");
		return;
	}



	notify->setSoundCollectionPath(options_page->SoundDirectoryPathChooser->path());
	notify->setCurrentLanguage(options_page->SoundCollectionList->currentText());
	notify->setDataObject(options_page->UAVObject->currentText());
	notify->setObjectField(options_page->UAVObjectField->currentText());
	notify->setValue(options_page->Value->currentText());
	notify->setSpinBoxValue(options_page->ValueSpinBox->value());

	if(options_page->Sound1->currentText()!="")
		notify->setSound1(options_page->Sound1->currentText());

	notify->setSound2(options_page->Sound2->currentText());

	if((options_page->Sound2->currentText()=="")&&(options_page->SayOrder->currentText()=="After second"))
		return;
	else
		notify->setSayOrder(options_page->SayOrder->currentText());

	int row = options_page->tableNotifications->rowCount();
	options_page->tableNotifications->setRowCount(row+1);
	row = options_page->tableNotifications->rowCount();
	options_page->tableNotifications->setItem (row-1,0,
											   new QTableWidgetItem(notify->parseNotifyMessage()));

	privListNotifications.append(notify);
	notify = new NotifyPluginConfiguration;
}


void NotifyPluginOptionsPage::on_buttonDeleteNotification_clicked()
{
	QTableWidgetItem * item = options_page->tableNotifications->currentItem();
	int row = item->row();
	privListNotifications.removeAt(row);

//	if(!privListNotifications.size())
//		notify = new NotifyPluginConfiguration;

	options_page->tableNotifications->removeRow(row);

	if(!options_page->tableNotifications->rowCount())
	{
		options_page->buttonDelete->setEnabled(false);
		options_page->buttonModify->setEnabled(false);
	}

}

void NotifyPluginOptionsPage::on_buttonModifyNotification_clicked()
{
	QTableWidgetItem * item = options_page->tableNotifications->currentItem();
	if(!item) return;
	notify = privListNotifications.at(item->row());
	getOptionsPageValues();
	item->setText(notify->parseNotifyMessage());
}

