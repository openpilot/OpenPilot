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
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QList>

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/generalsettings.h>

ConfigStabilizationWidget::ConfigStabilizationWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    ui = new Ui_StabilizationWidget();
    ui->setupUi(this);


    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings* settings = pm->getObject<Core::Internal::GeneralSettings>();
    if(!settings->useExpertMode()) {
        ui->saveStabilizationToRAM_6->setVisible(false);
    }

    autoLoadWidgets();

    realtimeUpdates = new QTimer(this);
    connect(realtimeUpdates, SIGNAL(timeout()), this, SLOT(apply()));

    connect(ui->realTimeUpdates_6, SIGNAL(stateChanged(int)), this, SLOT(realtimeUpdatesSlot(int)));
    addWidget(ui->realTimeUpdates_6);
    connect(ui->realTimeUpdates_8, SIGNAL(stateChanged(int)), this, SLOT(realtimeUpdatesSlot(int)));
    addWidget(ui->realTimeUpdates_8);

    connect(ui->checkBox_7, SIGNAL(stateChanged(int)), this, SLOT(linkCheckBoxes(int)));
    addWidget(ui->checkBox_7);
    connect(ui->checkBox_2, SIGNAL(stateChanged(int)), this, SLOT(linkCheckBoxes(int)));
    addWidget(ui->checkBox_2);
    connect(ui->checkBox_8, SIGNAL(stateChanged(int)), this, SLOT(linkCheckBoxes(int)));
    addWidget(ui->checkBox_8);
    connect(ui->checkBox_3, SIGNAL(stateChanged(int)), this, SLOT(linkCheckBoxes(int)));
    addWidget(ui->checkBox_3);

    addWidget(ui->pushButton_2);
    addWidget(ui->pushButton_3);
    addWidget(ui->pushButton_4);
    addWidget(ui->pushButton_5);
    addWidget(ui->pushButton_6);
    addWidget(ui->pushButton_9);
    addWidget(ui->pushButton_20);
    addWidget(ui->pushButton_22);
    addWidget(ui->pushButton_23);

    connect(this, SIGNAL(widgetContentsChanged(QWidget*)), this, SLOT(processLinkedWidgets(QWidget*)));

    // Link by default
    ui->checkBox_7->setChecked(true);
    ui->checkBox_8->setChecked(true);

    disableMouseWheelEvents();
    updateEnableControls();
}

ConfigStabilizationWidget::~ConfigStabilizationWidget()
{
    // Do nothing
}

void ConfigStabilizationWidget::realtimeUpdatesSlot(int value)
{
    ui->realTimeUpdates_6->setCheckState((Qt::CheckState)value);
    ui->realTimeUpdates_8->setCheckState((Qt::CheckState)value);

    if(value == Qt::Checked && !realtimeUpdates->isActive()) {
        realtimeUpdates->start(300);
    }
    else if(value == Qt::Unchecked) {
        realtimeUpdates->stop();
    }
}

void ConfigStabilizationWidget::linkCheckBoxes(int value)
{
    if(sender() == ui->checkBox_7) {
        ui->checkBox_3->setCheckState((Qt::CheckState)value);
    }
    else if(sender() == ui->checkBox_3) {
        ui->checkBox_7->setCheckState((Qt::CheckState)value);
    }
    else if(sender( )== ui->checkBox_8) {
        ui->checkBox_2->setCheckState((Qt::CheckState)value);
    }
    else if(sender() == ui->checkBox_2) {
        ui->checkBox_8->setCheckState((Qt::CheckState)value);
    }
}

void ConfigStabilizationWidget::processLinkedWidgets(QWidget * widget)
{
    if(ui->checkBox_7->checkState()==Qt::Checked)
    {
        if(widget== ui->RateRollKp_2)
        {
            ui->RatePitchKp->setValue(ui->RateRollKp_2->value());
        }
        else if(widget== ui->RateRollKi_2)
        {
            ui->RatePitchKi->setValue(ui->RateRollKi_2->value());
        }
        else if(widget== ui->RateRollILimit_2)
        {
            ui->RatePitchILimit->setValue(ui->RateRollILimit_2->value());
        }
        else if(widget== ui->RatePitchKp)
        {
            ui->RateRollKp_2->setValue(ui->RatePitchKp->value());
        }
        else if(widget== ui->RatePitchKi)
        {
            ui->RateRollKi_2->setValue(ui->RatePitchKi->value());
        }
        else if(widget== ui->RatePitchILimit)
        {
            ui->RateRollILimit_2->setValue(ui->RatePitchILimit->value());
        }
        else if(widget== ui->RollRateKd)
        {
            ui->PitchRateKd->setValue(ui->RollRateKd->value());
        }
        else if(widget== ui->PitchRateKd)
        {
            ui->RollRateKd->setValue(ui->PitchRateKd->value());
        }
    }
    if(ui->checkBox_8->checkState()==Qt::Checked)
    {
        if(widget== ui->AttitudeRollKp)
        {
            ui->AttitudePitchKp_2->setValue(ui->AttitudeRollKp->value());
        }
        else if(widget== ui->AttitudeRollKi)
        {
            ui->AttitudePitchKi_2->setValue(ui->AttitudeRollKi->value());
        }
        else if(widget== ui->AttitudeRollILimit)
        {
            ui->AttitudePitchILimit_2->setValue(ui->AttitudeRollILimit->value());
        }
        else if(widget== ui->AttitudePitchKp_2)
        {
            ui->AttitudeRollKp->setValue(ui->AttitudePitchKp_2->value());
        }
        else if(widget== ui->AttitudePitchKi_2)
        {
            ui->AttitudeRollKi->setValue(ui->AttitudePitchKi_2->value());
        }
        else if(widget== ui->AttitudePitchILimit_2)
        {
            ui->AttitudeRollILimit->setValue(ui->AttitudePitchILimit_2->value());
        }
    }
}



