/**
 ******************************************************************************
 *
 * @file       configstabilizationwidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#include "configstabilizationwidget.h"

#include <QDebug>
#include <QStringList>
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QList>
#include <QTabBar>
#include <QMessageBox>

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/generalsettings.h>
#include "altitudeholdsettings.h"
#include "stabilizationsettings.h"

ConfigStabilizationWidget::ConfigStabilizationWidget(QWidget *parent) : ConfigTaskWidget(parent),
    boardModel(0), m_pidBankCount(0), m_currentPIDBank(0)
{
    ui = new Ui_StabilizationWidget();
    ui->setupUi(this);

    StabilizationSettings *stabSettings = qobject_cast<StabilizationSettings *>(getObject("StabilizationSettings"));
    Q_ASSERT(stabSettings);

    m_pidBankCount = stabSettings->getField("FlightModeMap")->getOptions().count();

    // Set up fake tab widget stuff for pid banks support
    m_pidTabBars.append(ui->basicPIDBankTabBar);
    m_pidTabBars.append(ui->advancedPIDBankTabBar);
    foreach(QTabBar * tabBar, m_pidTabBars) {
        for (int i = 0; i < m_pidBankCount; i++) {
            tabBar->addTab(tr("PID Bank %1").arg(i + 1));
            tabBar->setTabData(i, QString("StabilizationSettingsBank%1").arg(i + 1));
        }
        tabBar->setExpanding(false);
        connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(pidBankChanged(int)));
    }

    for (int i = 0; i < m_pidBankCount; i++) {
        if (i > 0) {
            m_stabilizationObjectsString.append(",");
        }
        m_stabilizationObjectsString.append(m_pidTabBars.at(0)->tabData(i).toString());
    }

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings *settings = pm->getObject<Core::Internal::GeneralSettings>();

    if (!settings->useExpertMode()) {
        ui->saveStabilizationToRAM_6->setVisible(false);
    }

    autoLoadWidgets();

    realtimeUpdates = new QTimer(this);
    connect(realtimeUpdates, SIGNAL(timeout()), this, SLOT(apply()));

    connect(ui->realTimeUpdates_6, SIGNAL(toggled(bool)), this, SLOT(realtimeUpdatesSlot(bool)));
    addWidget(ui->realTimeUpdates_6);
    connect(ui->realTimeUpdates_8, SIGNAL(toggled(bool)), this, SLOT(realtimeUpdatesSlot(bool)));
    addWidget(ui->realTimeUpdates_8);
    connect(ui->realTimeUpdates_12, SIGNAL(toggled(bool)), this, SLOT(realtimeUpdatesSlot(bool)));
    addWidget(ui->realTimeUpdates_12);
    connect(ui->realTimeUpdates_7, SIGNAL(toggled(bool)), this, SLOT(realtimeUpdatesSlot(bool)));
    addWidget(ui->realTimeUpdates_7);

    connect(ui->checkBox_7, SIGNAL(toggled(bool)), this, SLOT(linkCheckBoxes(bool)));
    addWidget(ui->checkBox_7);
    connect(ui->checkBox_2, SIGNAL(toggled(bool)), this, SLOT(linkCheckBoxes(bool)));
    addWidget(ui->checkBox_2);
    connect(ui->checkBox_8, SIGNAL(toggled(bool)), this, SLOT(linkCheckBoxes(bool)));
    addWidget(ui->checkBox_8);
    connect(ui->checkBox_3, SIGNAL(toggled(bool)), this, SLOT(linkCheckBoxes(bool)));
    addWidget(ui->checkBox_3);

    addWidget(ui->pushButton_2);
    addWidget(ui->pushButton_3);
    addWidget(ui->pushButton_4);
    addWidget(ui->pushButton_5);
    addWidget(ui->pushButton_6);
    addWidget(ui->pushButton_7);
    addWidget(ui->pushButton_8);
    addWidget(ui->pushButton_9);
    addWidget(ui->pushButton_10);
    addWidget(ui->pushButton_11);
    addWidget(ui->pushButton_20);
    addWidget(ui->pushButton_22);
    addWidget(ui->pushButton_23);

    addWidget(ui->basicResponsivenessGroupBox);
    addWidget(ui->basicResponsivenessCheckBox);
    connect(ui->basicResponsivenessCheckBox, SIGNAL(toggled(bool)), this, SLOT(linkCheckBoxes(bool)));
    addWidget(ui->advancedResponsivenessGroupBox);
    addWidget(ui->advancedResponsivenessCheckBox);
    connect(ui->advancedResponsivenessCheckBox, SIGNAL(toggled(bool)), this, SLOT(linkCheckBoxes(bool)));

    connect(this, SIGNAL(widgetContentsChanged(QWidget *)), this, SLOT(processLinkedWidgets(QWidget *)));

    connect(this, SIGNAL(autoPilotConnected()), this, SLOT(onBoardConnected()));

    disableMouseWheelEvents();
    updateEnableControls();
}

ConfigStabilizationWidget::~ConfigStabilizationWidget()
{
    // Do nothing
}

void ConfigStabilizationWidget::refreshWidgetsValues(UAVObject *o)
{
    ConfigTaskWidget::refreshWidgetsValues(o);

    ui->basicResponsivenessCheckBox->setChecked(ui->rateRollKp_3->value() == ui->ratePitchKp_4->value() &&
                                                ui->rateRollKi_3->value() == ui->ratePitchKi_4->value());
}

void ConfigStabilizationWidget::realtimeUpdatesSlot(bool value)
{
    ui->realTimeUpdates_6->setChecked(value);
    ui->realTimeUpdates_8->setChecked(value);
    ui->realTimeUpdates_12->setChecked(value);
    ui->realTimeUpdates_7->setChecked(value);

    if (value && !realtimeUpdates->isActive()) {
        realtimeUpdates->start(AUTOMATIC_UPDATE_RATE);
        qDebug() << "Instant Update timer started.";
    } else if (!value && realtimeUpdates->isActive()) {
        realtimeUpdates->stop();
        qDebug() << "Instant Update timer stopped.";
    }
}

void ConfigStabilizationWidget::linkCheckBoxes(bool value)
{
    if (sender() == ui->checkBox_7) {
        ui->checkBox_3->setChecked(value);
    } else if (sender() == ui->checkBox_3) {
        ui->checkBox_7->setChecked(value);
    } else if (sender() == ui->checkBox_8) {
        ui->checkBox_2->setChecked(value);
    } else if (sender() == ui->checkBox_2) {
        ui->checkBox_8->setChecked(value);
    } else if (sender() == ui->basicResponsivenessCheckBox) {
        ui->advancedResponsivenessCheckBox->setChecked(!value);
        ui->basicResponsivenessControls->setEnabled(value);
        ui->advancedResponsivenessControls->setEnabled(!value);
        if (value) {
            processLinkedWidgets(ui->AttitudeResponsivenessSlider);
            processLinkedWidgets(ui->RateResponsivenessSlider);
        }
    } else if (sender() == ui->advancedResponsivenessCheckBox) {
        ui->basicResponsivenessCheckBox->setChecked(!value);
        ui->basicResponsivenessControls->setEnabled(!value);
        ui->advancedResponsivenessControls->setEnabled(value);
    }
}

void ConfigStabilizationWidget::processLinkedWidgets(QWidget *widget)
{
    if (ui->checkBox_7->isChecked()) {
        if (widget == ui->RateRollKp_2) {
            ui->RatePitchKp->setValue(ui->RateRollKp_2->value());
        } else if (widget == ui->RateRollKi_2) {
            ui->RatePitchKi->setValue(ui->RateRollKi_2->value());
        } else if (widget == ui->RatePitchKp) {
            ui->RateRollKp_2->setValue(ui->RatePitchKp->value());
        } else if (widget == ui->RatePitchKi) {
            ui->RateRollKi_2->setValue(ui->RatePitchKi->value());
        } else if (widget == ui->RollRateKd) {
            ui->PitchRateKd->setValue(ui->RollRateKd->value());
        } else if (widget == ui->PitchRateKd) {
            ui->RollRateKd->setValue(ui->PitchRateKd->value());
        }
    }

    if (ui->checkBox_8->isChecked()) {
        if (widget == ui->AttitudeRollKp) {
            ui->AttitudePitchKp_2->setValue(ui->AttitudeRollKp->value());
        } else if (widget == ui->AttitudeRollKi) {
            ui->AttitudePitchKi_2->setValue(ui->AttitudeRollKi->value());
        } else if (widget == ui->AttitudePitchKp_2) {
            ui->AttitudeRollKp->setValue(ui->AttitudePitchKp_2->value());
        } else if (widget == ui->AttitudePitchKi_2) {
            ui->AttitudeRollKi->setValue(ui->AttitudePitchKi_2->value());
        }
    }

    if (ui->basicResponsivenessCheckBox->isChecked()) {
        if (widget == ui->AttitudeResponsivenessSlider) {
            ui->ratePitchKp_4->setValue(ui->AttitudeResponsivenessSlider->value());
        } else if (widget == ui->RateResponsivenessSlider) {
            ui->ratePitchKi_4->setValue(ui->RateResponsivenessSlider->value());
        }
    }
}

void ConfigStabilizationWidget::onBoardConnected()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager *utilMngr     = pm->getObject<UAVObjectUtilManager>();

    Q_ASSERT(utilMngr);
    boardModel = utilMngr->getBoardModel();
    // If Revolution board enable misc tab, otherwise disable it
    ui->AltitudeHold->setEnabled((boardModel & 0xff00) == 0x0900);
}

void ConfigStabilizationWidget::pidBankChanged(int index)
{
    foreach(QTabBar * tabBar, m_pidTabBars) {
        disconnect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(pidBankChanged(int)));
        tabBar->setCurrentIndex(index);
        connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(pidBankChanged(int)));
    }

    for (int i = 0; i < m_pidTabBars.at(0)->count(); i++) {
        setWidgetBindingObjectEnabled(m_pidTabBars.at(0)->tabData(i).toString(), false);
    }

    setWidgetBindingObjectEnabled(m_pidTabBars.at(0)->tabData(index).toString(), true);

    m_currentPIDBank = index;
}

bool ConfigStabilizationWidget::shouldObjectBeSaved(UAVObject *object)
{
    // AltitudeHoldSettings should only be saved for Revolution board to avoid error.
    if ((boardModel & 0xff00) != 0x0900) {
        return dynamic_cast<AltitudeHoldSettings *>(object) == 0;
    } else {
        return true;
    }
}

QString ConfigStabilizationWidget::mapObjectName(const QString objectName)
{
    if (objectName == "StabilizationSettingsBankX") {
        return m_stabilizationObjectsString;
    }
    return ConfigTaskWidget::mapObjectName(objectName);
}
