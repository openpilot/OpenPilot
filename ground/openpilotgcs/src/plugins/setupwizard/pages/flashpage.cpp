/**
 ******************************************************************************
 *
 * @file       flashpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup FlashPage
 * @{
 * @brief
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

#include "flashpage.h"
#include "ui_flashpage.h"
#include "setupwizard.h"
#include "vehicleconfigurationhelper.h"

FlashPage::FlashPage(SetupWizard *wizard, QWidget *parent) :
        AbstractWizardPage(wizard, parent),
    ui(new Ui::FlashPage), m_successfulWrite(false)
{
    ui->setupUi(this);
    connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(writeToController()));
}

FlashPage::~FlashPage()
{
    delete ui;
}

bool FlashPage::validatePage()
{    
    return true;
}

bool FlashPage::isComplete() const
{
    return m_successfulWrite;
}

void FlashPage::writeToController()
{
    ui->saveButton->setEnabled(false);
    VehicleConfigurationHelper helper(getWizard());
    connect(&helper, SIGNAL(saveProgress(int, int, QString)),this, SLOT(saveProgress(int, int, QString)));
    m_successfulWrite = helper.setupVehicle();
    disconnect(&helper, SIGNAL(saveProgress(int, int, QString)),this, SLOT(saveProgress(int, int, QString)));
    emit completeChanged();
    ui->saveProgressLabel->setText(QString("<font color='%1'>%2</font>").arg(m_successfulWrite ? "green" : "red", ui->saveProgressLabel->text()));
    ui->saveButton->setEnabled(true);
}

void FlashPage::saveProgress(int total, int current, QString description)
{
    if(ui->saveProgressBar->maximum() != total) {
        ui->saveProgressBar->setMaximum(total);
    }
    if(ui->saveProgressBar->value() != current) {
        ui->saveProgressBar->setValue(current);
    }
    if(ui->saveProgressLabel->text() != description) {
        ui->saveProgressLabel->setText(description);
    }
}
