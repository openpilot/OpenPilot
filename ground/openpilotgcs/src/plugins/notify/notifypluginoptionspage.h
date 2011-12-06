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
    enum {equal,bigger,smaller,inrange};
    explicit NotifyPluginOptionsPage(QObject *parent = 0);
    ~NotifyPluginOptionsPage();
    QString id() const { return QLatin1String("settings"); }
    QString trName() const { return tr("settings"); }
    QString category() const { return QLatin1String("Notify Plugin");}
    QString trCategory() const { return tr("Notify Plugin");}

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();
    void restoreFromSettings();
    static QStringList conditionValues;

signals:
    void updateNotifications(QList<NotificationItem*> list);
    void entryUpdated(int index);

private slots:
    void on_clicked_buttonTestSoundNotification();
    void on_clicked_buttonAddNotification();
    void on_clicked_buttonDeleteNotification();
    void on_clicked_buttonModifyNotification();

    /**
     * We can use continuous selection, to select simultaneously
     * multiple rows to move them(using drag & drop) inside table ranges.
     */
    void on_changedSelection_notifyTable( const QItemSelection & selected, const QItemSelection & deselected );

    void on_changedIndex_soundLanguage(int index);
    void on_clicked_buttonSoundFolder(const QString& path);
    void on_changedIndex_UAVObject(QString val);
    void on_changedIndex_UAVField(QString val);
    void on_changed_playButtonText(Phonon::State newstate, Phonon::State oldstate);
    void on_toggled_checkEnableSound(bool state);

    /**
     * Important when we change to or from "In range" value
     * For enums UI layout stayed the same, but for numeric values
     * we need to change UI to show edit line,
     * to have possibility assign range limits for value.
     */
    void on_changedIndex_rangeValue(QString);

    void on_FinishedPlaying(void);


private:
    Q_DISABLE_COPY(NotifyPluginOptionsPage)

    void initButtons();
    void initPhononPlayer();
    void initRulesTable();

    void setSelectedNotification(NotificationItem* ntf);
    void resetValueRange();
    void resetFieldType();

    void updateConfigView(NotificationItem* notification);
    void getOptionsPageValues(NotificationItem* notification);
    UAVObjectField* getObjectFieldFromPage();
    UAVObjectField* getObjectFieldFromSelected();

    void addDynamicFieldLayout();
    void addDynamicField(UAVObjectField* objField);
    void addDynamicFieldWidget(UAVObjectField* objField);
    void setDynamicFieldValue(NotificationItem* notification);

private:

    UAVObjectManager& _objManager;
    SoundNotifyPlugin* _owner;

    //! Media object uses to test sound playing
    QScopedPointer<Phonon::MediaObject> _testSound;

    QScopedPointer<NotifyTableModel> _notifyRulesModel;
    QItemSelectionModel* _notifyRulesSelection;

    /**
     * Local copy of notification list, which owned by notify plugin.
     * Notification list readed once on application loaded, during
     * notify plugin startup, then on open options page.
     * This copy is simple assignment, but due to implicitly sharing
     * we don't have additional cost for that, copy will created
     * only after modification of private notify list.
     */
    QList<NotificationItem*> _privListNotifications;

    QScopedPointer<Ui::NotifyPluginOptionsPage> _optionsPage;

    //! Widget to convinient selection of condition for field value (equal, lower, greater)
    QComboBox* _dynamicFieldCondition;

    //! Represents edit widget for dynamic UAVObjectfield,
    //! can be spinbox - for numerics, combobox - enums, or
    //! lineedit - for numerics with range constraints
    QWidget* _dynamicFieldWidget;

    //! Type of UAVObjectField - numeric or ENUM,
    //! this variable needs to correctly set appropriate dynamic UI element (_dynamicFieldWidget)
    //! NOTE: ocassionaly it should be invalidated (= -1) to reset _dynamicFieldWidget
    int _dynamicFieldType;

    //! Widget to convinient selection of position of <dynamic field value>
    //! between sounds[1..3]
    QComboBox* _sayOrder;

    //! Actualy reference to optionsPageWidget,
    //! we MUST hold it beyond the scope of createPage func
    //! to have possibility change dynamic parts of options page layout in future
    QWidget* _form;

    //! Currently selected notification, all controls filled accroding to it.
    //! On options page startup, always points to first row.
    NotificationItem* _selectedNotification;

    //! Retrieved from UAVObjectManager by name from _selectedNotification,
    //! if UAVObjectManager doesn't have such object, this field will be NULL
    UAVDataObject* _currUAVObject;

};

#endif // NOTIFYPLUGINOPTIONSPAGE_H
