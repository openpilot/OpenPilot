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
#include "notifyitemdelegate.h"

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
	delegateItems.clear();
	delegateItems << "Continue" << "Once";

	options_page->chkEnableSound->setChecked(owner->getEnableSound());
	options_page->SoundDirectoryPathChooser->setExpectedKind(Utils::PathChooser::Directory);
	options_page->SoundDirectoryPathChooser->setPromptDialogTitle(tr("Choose sound collection directory"));
//	connect(options_page->tableNotifications->model(),SIGNAL(rowsInserted ( const QModelIndex & , int , int )),this,
//			SLOT(showPersistentComboBox ( const QModelIndex & , int , int )));
//	connect(options_page->tableNotifications->model(),SIGNAL(dataChanged ( const QModelIndex & , const QModelIndex & )),this,
//			SLOT(showPersistentComboBox2( const QModelIndex & , const QModelIndex & )));


	options_page->tableNotifications->setRowCount(0);
	options_page->tableNotifications->setItemDelegate(new NotifyItemDelegate(delegateItems,options_page->tableNotifications));

	QStringList labels;
	labels << "Name" << "Repeats";
	options_page->tableNotifications->setHorizontalHeaderLabels(labels);
	privListNotifications.clear();
	listSoundFiles.clear();

	settings = Core::ICore::instance()->settings();
	settings->beginGroup(QLatin1String("NotifyPlugin"));

	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	objManager = pm->getObject<UAVObjectManager>();

	// Fills the combo boxes for the UAVObjects
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

	int size = settings->beginReadArray("listNotifies");
	for (int i = 0; i < size; ++i) {
		 settings->setArrayIndex(i);
		 NotifyPluginConfiguration* notification = new NotifyPluginConfiguration;
		 notification->restoreState(settings);
		 privListNotifications.append(notification);
	}
	settings->endArray();

	settings->beginReadArray("Current");
	settings->setArrayIndex(0);
	NotifyPluginConfiguration notification;
	notification.restoreState(settings);
	updateConfigView(&notification);
	settings->endArray();
	options_page->chkEnableSound->setChecked(settings->value(QLatin1String("EnableSound"),0).toBool());
	settings->endGroup();

	foreach(NotifyPluginConfiguration* notification,privListNotifications)
	{
		options_page->tableNotifications->setRowCount(options_page->tableNotifications->rowCount()+1);
		options_page->tableNotifications->setItem (options_page->tableNotifications->rowCount()-1,0,
											   new QTableWidgetItem(notification->parseNotifyMessage()));

		QString str = notification->getRepeatFlag();
		options_page->tableNotifications->setItem (options_page->tableNotifications->rowCount()-1,1,
											   new QTableWidgetItem(str));


	}

	options_page->buttonModify->setEnabled(false);
	options_page->buttonDelete->setEnabled(false);
	options_page->buttonPlayNotification->setEnabled(false);

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

void NotifyPluginOptionsPage::showPersistentComboBox( const QModelIndex & parent, int start, int end )
{
//	for (int i=start; i<end+1; i++) {
//		options_page->tableNotifications->openPersistentEditor(options_page->tableNotifications->item(i,1));
//	}
}

void NotifyPluginOptionsPage::showPersistentComboBox2( const QModelIndex & topLeft, const QModelIndex & bottomRight )
{
	//for (QModelIndex i=topLeft; i<bottomRight+1; i++)
	{
		options_page->tableNotifications->openPersistentEditor(options_page->tableNotifications->item(options_page->tableNotifications->currentRow(),1));
	}
}


void NotifyPluginOptionsPage::getOptionsPageValues(NotifyPluginConfiguration* notification)
{
	notification->setSoundCollectionPath(options_page->SoundDirectoryPathChooser->path());
	notification->setCurrentLanguage(options_page->SoundCollectionList->currentText());
	notification->setDataObject(options_page->UAVObject->currentText());
	notification->setObjectField(options_page->UAVObjectField->currentText());
	notification->setSound1(options_page->Sound1->currentText());
	notification->setSound2(options_page->Sound2->currentText());
	notification->setSayOrder(options_page->SayOrder->currentText());
	notification->setValue(options_page->Value->currentText());
	notification->setSpinBoxValue(options_page->ValueSpinBox->value());
	if(options_page->tableNotifications->currentRow()>-1)
	{
		qDebug() << "delegate value:" << options_page->tableNotifications->item(options_page->tableNotifications->currentRow(),1)->data(Qt::EditRole);
		notification->setRepeatFlag(options_page->tableNotifications->item(options_page->tableNotifications->currentRow(),1)->data(Qt::EditRole).toString());
	}
}

////////////////////////////////////////////
// Called when the user presses apply or OK.
//
// Saves the current values
//
////////////////////////////////////////////
void NotifyPluginOptionsPage::apply()
{
	NotifyPluginConfiguration notification;

	settings->beginGroup(QLatin1String("NotifyPlugin"));

	getOptionsPageValues(&notification);

	settings->beginWriteArray("Current");
	settings->setArrayIndex(0);
	notification.saveState(settings);
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
	settings->endArray();
	settings->setValue(QLatin1String("EnableSound"), options_page->chkEnableSound->isChecked());
	settings->endGroup();

	owner->setEnableSound(options_page->chkEnableSound->isChecked());
	owner->setListNotifications(privListNotifications);
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
		if(newstate  == Phonon::PausedState) {
			options_page->buttonTestSound1->setText("Play");
			options_page->buttonTestSound1->setIcon(QPixmap(":/notify/images/play.png"));
		}
		else
			if(newstate  == Phonon::PlayingState) {
				options_page->buttonTestSound1->setText("Stop");
				options_page->buttonTestSound1->setIcon(QPixmap(":/notify/images/stop.png"));
			}
	}
	else
	{
		if(sender()==sound2)
		{
			if(newstate  == Phonon::PausedState) {
				options_page->buttonTestSound2->setText("Play");
				options_page->buttonTestSound2->setIcon(QPixmap(":/notify/images/play.png"));
			}
			else
				if(newstate  == Phonon::PlayingState) {
					options_page->buttonTestSound2->setText("Stop");
					options_page->buttonTestSound2->setIcon(QPixmap(":/notify/images/stop.png"));
				}
		}
		else
		{
			if(sender()==notifySound)
			{
				if(newstate  == Phonon::PausedState){
					options_page->buttonPlayNotification->setText("Play");
					options_page->buttonPlayNotification->setIcon(QPixmap(":/notify/images/play.png"));
				}
				else
					if(newstate  == Phonon::PlayingState) {
						options_page->buttonPlayNotification->setText("Stop");
						options_page->buttonPlayNotification->setIcon(QPixmap(":/notify/images/stop.png"));
					}
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
	NotifyPluginConfiguration* notification = new NotifyPluginConfiguration;
	//getOptionsPageValues();
	if(options_page->tableNotifications->currentRow()==-1) return;
	notification = privListNotifications.at(options_page->tableNotifications->currentRow());
	notification->parseNotifyMessage();
	QStringList stringList = notification->getNotifyMessageList();
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
	QString path = notification->getSoundCollectionPath();
	if(path=="")
	{
		//QDir dir = QDir::currentPath();
		//path = QDir::currentPath().left(QDir::currentPath().indexOf("OpenPilot",0,Qt::CaseSensitive))+"../share/sounds";
		path = "../share/sounds";
	}
	options_page->SoundDirectoryPathChooser->setPath(path);

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
	QString uavDataObject = notification->getDataObject();
	UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(uavDataObject/*objList.at(0).at(0)->getName()*/) );
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

	if(options_page->Sound2->findText(notification->getSound2())!=-1) {
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

	if(options_page->Value->findText(notification->getValue())!=-1) {
		options_page->Value->setCurrentIndex(options_page->Value->findText(notification->getValue()));
	}

	if(options_page->SayOrder->findText(notification->getSayOrder())!=-1) {
		options_page->SayOrder->setCurrentIndex(options_page->SayOrder->findText(notification->getSayOrder()));
	}
	options_page->ValueSpinBox->setValue(notification->getSpinBoxValue());
}

void NotifyPluginOptionsPage::on_tableNotification_changeSelection()
{
	QTableWidgetItem * item = options_page->tableNotifications->item(options_page->tableNotifications->currentRow(),0);

	if(!item) return;

	updateConfigView(privListNotifications.at(item->row()));
	options_page->buttonModify->setEnabled(item->isSelected());
	options_page->buttonDelete->setEnabled(item->isSelected());
	options_page->buttonPlayNotification->setEnabled(item->isSelected());
}

void NotifyPluginOptionsPage::on_buttonAddNotification_clicked()
{
	NotifyPluginConfiguration* notification = new NotifyPluginConfiguration;

	if(options_page->SoundDirectoryPathChooser->path()=="")
	{
		QPalette textPalette=options_page->SoundDirectoryPathChooser->palette();
		textPalette.setColor(QPalette::Normal,QPalette::Text, Qt::red);
		options_page->SoundDirectoryPathChooser->setPalette(textPalette);
		options_page->SoundDirectoryPathChooser->setPath("please select sound collection folder");
		return;
	}

	notification->setSoundCollectionPath(options_page->SoundDirectoryPathChooser->path());
	notification->setCurrentLanguage(options_page->SoundCollectionList->currentText());
	notification->setDataObject(options_page->UAVObject->currentText());
	notification->setObjectField(options_page->UAVObjectField->currentText());
	notification->setValue(options_page->Value->currentText());
	notification->setSpinBoxValue(options_page->ValueSpinBox->value());

	if(options_page->Sound1->currentText()!="")
		notification->setSound1(options_page->Sound1->currentText());

	notification->setSound2(options_page->Sound2->currentText());

	if((options_page->Sound2->currentText()=="")&&(options_page->SayOrder->currentText()=="After second"))
		return;
	else
		notification->setSayOrder(options_page->SayOrder->currentText());

	int row = options_page->tableNotifications->rowCount();
	options_page->tableNotifications->setRowCount(row+1);
	row = options_page->tableNotifications->rowCount();
	options_page->tableNotifications->setItem (row-1,0,new QTableWidgetItem(notification->parseNotifyMessage()));
	options_page->tableNotifications->setItem (row-1,1,new QTableWidgetItem(delegateItems.first()));
	//options_page->tableNotifications->openPersistentEditor(options_page->tableNotifications->item(row-1,1));

	privListNotifications.append(notification);
}


void NotifyPluginOptionsPage::on_buttonDeleteNotification_clicked()
{
	QTableWidgetItem * item = options_page->tableNotifications->currentItem();
	int row = item->row();
	privListNotifications.removeAt(row);

//	if(!privListNotifications.size())
//		notify = new NotifyPluginConfiguration;

	if(options_page->tableNotifications->rowCount()>1 && row < options_page->tableNotifications->rowCount()-1
	   /*&& row*/)
		if(item->isSelected())
			item->setSelected(false);

	options_page->tableNotifications->removeRow(row);

	if(!options_page->tableNotifications->rowCount())
	{
		options_page->buttonDelete->setEnabled(false);
		options_page->buttonModify->setEnabled(false);
		options_page->buttonPlayNotification->setEnabled(false);
	}

}

void NotifyPluginOptionsPage::on_buttonModifyNotification_clicked()
{
	NotifyPluginConfiguration* notification = new NotifyPluginConfiguration;
	//NotifyPluginConfiguration notification;

	QTableWidgetItem * item = options_page->tableNotifications->item(options_page->tableNotifications->currentRow(),0);
	if(!item) return;
	//notify = privListNotifications.at(item->row()); // ???
	getOptionsPageValues(notification);
	privListNotifications.replace(item->row(),notification);
	item->setText(notification->parseNotifyMessage());
}

