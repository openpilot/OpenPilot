/**
 ******************************************************************************
 *
 * @file       notifypluginoptionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Notify Plugin options page header
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   notify
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


#ifndef NOTIFYPLUGINOPTIONSPAGE_H
#define NOTIFYPLUGINOPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"
#include "uavobjectmanager.h"
#include "uavobject.h"

#include "QString"
#include <QStringList>
#include <QItemSelectionModel>
#include <QDebug>
#include <QtCore/QSettings>
#include <phonon/MediaObject>
#include <phonon/Path>
#include <phonon/AudioOutput>
#include <phonon/Global>

class NotifyTableModel;

class NotifyPluginConfiguration;
class SoundNotifyPlugin;

namespace Ui {
    class NotifyPluginOptionsPage;
}


using namespace Core;

class NotifyPluginOptionsPage : public IOptionsPage
{
	Q_OBJECT
public:
    explicit NotifyPluginOptionsPage(/*NotifyPluginConfiguration *config, */QObject *parent = 0);

	QString id() const { return QLatin1String("settings"); }
	QString trName() const { return tr("settings"); }
	QString category() const { return QLatin1String("Notify Plugin");}
	QString trCategory() const { return tr("Notify Plugin");}



    QWidget *createPage(QWidget *parent);
    void apply();
	void finish();
	void restoreFromSettings();

	void updateConfigView(NotifyPluginConfiguration* notification);
	void getOptionsPageValues(NotifyPluginConfiguration* notification);

private:
	UAVObjectManager *objManager;
	SoundNotifyPlugin* owner;
	QStringList listDirCollections;
	QStringList listSoundFiles;
	QString currentCollectionPath;
	int sizeNotifyList;
	Phonon::MediaObject *sound1;
	Phonon::MediaObject *sound2;
	Phonon::MediaObject *notifySound;
	Phonon::AudioOutput *audioOutput;
	QStringList delegateItems;
	NotifyTableModel* notifyRulesModel;
	QItemSelectionModel *notifyRulesSelection;
	QList<NotifyPluginConfiguration*> privListNotifications;

	Ui::NotifyPluginOptionsPage *options_page;
	//NotifyPluginConfiguration *notify;

signals:
	void updateNotifications(QList<NotifyPluginConfiguration*> list);
	void resetNotification(void);
	void entryUpdated(int index);
	void entryAdded(int position);


private slots:
	void showPersistentComboBox( const QModelIndex & parent, int start, int end );
	void showPersistentComboBox2 ( const QModelIndex & topLeft, const QModelIndex & bottomRight );

//	void on_buttonTestSound1_clicked();
//	void on_buttonTestSound2_clicked();
	void on_buttonTestSoundNotification_clicked();

	void on_buttonAddNotification_clicked();
	void on_buttonDeleteNotification_clicked();
	void on_buttonModifyNotification_clicked();
	void on_tableNotification_changeSelection( const QItemSelection & selected, const QItemSelection & deselected );
	void on_soundLanguage_indexChanged(int index);
	void on_buttonSoundFolder_clicked(const QString& path);
	void on_UAVObject_indexChanged(QString val);
	void changeButtonText(Phonon::State newstate, Phonon::State oldstate);
	void on_chkEnableSound_toggled(bool state);
};

#endif // NOTIFYPLUGINOPTIONSPAGE_H
