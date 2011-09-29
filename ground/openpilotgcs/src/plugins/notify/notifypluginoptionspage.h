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
#include <QComboBox>
#include <QSpinBox>

class NotifyTableModel;
class NotificationItem;
class SoundNotifyPlugin;

namespace Ui {
	class NotifyPluginOptionsPage;
};

using namespace Core;

class NotifyPluginOptionsPage : public IOptionsPage
{
	Q_OBJECT
public:
	explicit NotifyPluginOptionsPage(/*NotificationItem *config, */QObject *parent = 0);
	~NotifyPluginOptionsPage();
	QString id() const { return QLatin1String("settings"); }
	QString trName() const { return tr("settings"); }
	QString category() const { return QLatin1String("Notify Plugin");}
	QString trCategory() const { return tr("Notify Plugin");}

    QWidget *createPage(QWidget *parent);
    void apply();
	void finish();
	void restoreFromSettings();

	void updateConfigView(NotificationItem* notification);
	void getOptionsPageValues(NotificationItem* notification);

signals:
    void updateNotifications(QList<NotificationItem*> list);
        //void resetNotification(void);
    void entryUpdated(int index);

private:
    Q_DISABLE_COPY(NotifyPluginOptionsPage)

    void resetValueRange();
    void setDynamicValueField(NotificationItem* notification);
    void addDynamicField(UAVObjectField* objField);
    void addDynamicValueLayout();
    void initButtons();
    void initPhononPlayer();
    void initRulesTable();

private slots:
	void on_buttonTestSoundNotification_clicked();

	void on_buttonAddNotification_clicked();
	void on_buttonDeleteNotification_clicked();
	void on_buttonModifyNotification_clicked();
	void on_tableNotification_changeSelection( const QItemSelection & selected, const QItemSelection & deselected );
	void on_soundLanguage_indexChanged(int index);
	void on_buttonSoundFolder_clicked(const QString& path);
	void on_UAVObject_indexChanged(QString val);
	void onUAVField_indexChanged(QString val);
	void changeButtonText(Phonon::State newstate, Phonon::State oldstate);
	void on_chkEnableSound_toggled(bool state);
	void on_rangeValue_indexChanged(QString);

	void onFinishedPlaying(void);

private:
	UAVObjectManager& objManager;
	SoundNotifyPlugin* owner;
	QStringList listDirCollections;
	QStringList listSoundFiles;
	QString currentCollectionPath;
	Phonon::MediaObject *sound1;
	Phonon::MediaObject *sound2;
	QScopedPointer<Phonon::MediaObject> notifySound;
	Phonon::AudioOutput *audioOutput;

	QScopedPointer<NotifyTableModel> _notifyRulesModel;
	QItemSelectionModel* _notifyRulesSelection;
	QList<NotificationItem*> privListNotifications;

	QScopedPointer<Ui::NotifyPluginOptionsPage> options_page;

    QComboBox* _valueRange;
    QComboBox* _sayOrder;
    QWidget* _fieldValue;
    int _fieldType;
    QWidget* _form;
    NotificationItem* _selectedNotification;
};

#endif // NOTIFYPLUGINOPTIONSPAGE_H
