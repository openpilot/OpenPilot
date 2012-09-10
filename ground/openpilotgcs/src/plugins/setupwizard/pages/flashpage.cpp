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

#include <QMessageBox>
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
    if(!getWizard()->getConnectionManager()->isConnected()) {
        QMessageBox msgBox;
        msgBox.setText(tr("An OpenPilot controller must be connected to your computer to save the "
                          "configuration.\nPlease connect your OpenPilot controller to your computer and try again."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    enableButtons(false);
    VehicleConfigurationHelper helper(getWizard());
    connect(&helper, SIGNAL(saveProgress(int, int, QString)),this, SLOT(saveProgress(int, int, QString)));

    m_successfulWrite = helper.setupVehicle();

    disconnect(&helper, SIGNAL(saveProgress(int, int, QString)),this, SLOT(saveProgress(int, int, QString)));
    ui->saveProgressLabel->setText(QString("<font color='%1'>%2</font>").arg(m_successfulWrite ? "green" : "red", ui->saveProgressLabel->text()));
    enableButtons(true);

    emit completeChanged();
}

void FlashPage::enableButtons(bool enable)
{
    ui->saveButton->setEnabled(enable);
    getWizard()->button(QWizard::NextButton)->setEnabled(enable);
    getWizard()->button(QWizard::CancelButton)->setEnabled(enable);
    getWizard()->button(QWizard::BackButton)->setEnabled(enable);
    QApplication::processEvents();
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
