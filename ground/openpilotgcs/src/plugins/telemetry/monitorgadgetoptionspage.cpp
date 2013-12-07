/**
 ******************************************************************************
 *
 * @file       monitorgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Telemetry Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   monitorgadget
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

#include "monitorgadgetoptionspage.h"
#include <coreplugin/icore.h>
// #include "ui_telemetrypluginoptionspage.h"
#include "extensionsystem/pluginmanager.h"

MonitorGadgetOptionsPage::MonitorGadgetOptionsPage(MonitorGadgetConfiguration *config, QObject *parent)
    : IOptionsPage(parent)
{}

MonitorGadgetOptionsPage::~MonitorGadgetOptionsPage()
{}

QWidget *MonitorGadgetOptionsPage::createPage(QWidget * /* parent */)
{
// _optionsPage.reset(new Ui::TelemetryPluginOptionsPage());
//// main widget
// QWidget *optionsPageWidget = new QWidget;
// _dynamicFieldWidget    = NULL;
// _dynamicFieldCondition = NULL;
// resetFieldType();
//// save ref to form, needed for binding dynamic fields in future
// _form = optionsPageWidget;
//// main layout
// _optionsPage->setupUi(optionsPageWidget);
//
// _optionsPage->SoundDirectoryPathChooser->setExpectedKind(Utils::PathChooser::Directory);
// _optionsPage->SoundDirectoryPathChooser->setPromptDialogTitle(tr("Choose sound collection directory"));
//
// connect(_optionsPage->SoundDirectoryPathChooser, SIGNAL(changed(const QString &)),
// this, SLOT(on_clicked_buttonSoundFolder(const QString &)));
// connect(_optionsPage->SoundCollectionList, SIGNAL(currentIndexChanged(int)),
// this, SLOT(on_changedIndex_soundLanguage(int)));
//
// connect(this, SIGNAL(updateNotifications(QList<NotificationItem *>)),
// _owner, SLOT(updateNotificationList(QList<NotificationItem *>)));
//// connect(this, SIGNAL(resetNotification()),owner, SLOT(resetNotification()));
//
// _privListNotifications = _owner->getListNotifications();
//
//
//// [1]
// setSelectedNotification(_owner->getCurrentNotification());
// addDynamicFieldLayout();
//// [2]
// updateConfigView(_selectedNotification);
//
// initRulesTable();
// initButtons();
// initPhononPlayer();
//
// int curr_row = _privListNotifications.indexOf(_selectedNotification);
// _telemetryRulesSelection->setCurrentIndex(_telemetryRulesModel->index(curr_row, 0, QModelIndex()),
// QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
//
// return optionsPageWidget;
    return NULL;
}

void MonitorGadgetOptionsPage::apply()
{
// getOptionsPageValues(_owner->getCurrentNotification());
// _owner->setEnableSound(_optionsPage->chkEnableSound->isChecked());
// emit updateNotifications(_privListNotifications);
}

void MonitorGadgetOptionsPage::finish()
{
// disconnect(_optionsPage->UAVObjectField, SIGNAL(currentIndexChanged(QString)),
// this, SLOT(on_changedIndex_UAVField(QString)));
//
// disconnect(_testSound.data(), SIGNAL(stateChanged(Phonon::State, Phonon::State)),
// this, SLOT(on_changed_playButtonText(Phonon::State, Phonon::State)));
// if (_testSound) {
// _testSound->stop();
// _testSound->clear();
// }
}
