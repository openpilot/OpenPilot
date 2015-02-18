/**
 ******************************************************************************
 *
 * @file       escpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup
 * @{
 * @addtogroup EscPage
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

#include "escpage.h"
#include "ui_escpage.h"
#include "setupwizard.h"

EscPage::EscPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),

    ui(new Ui::EscPage)
{
    ui->setupUi(this);
}

EscPage::~EscPage()
{
    delete ui;
}

bool EscPage::validatePage()
{
    if (ui->oneshotESCButton->isChecked()) {
        getWizard()->setEscType(SetupWizard::ESC_ONESHOT);
    }else if (ui->rapidESCButton->isChecked()) {
        getWizard()->setEscType(SetupWizard::ESC_RAPID);
    } else if (ui->defaultESCButton->isChecked()){
        getWizard()->setEscType(SetupWizard::ESC_STANDARD);
    }

    return true;
}


void EscPage::initializePage()
{
    if (getWizard()->getControllerType() == SetupWizard::CONTROLLER_CC ||
            getWizard()->getControllerType() == SetupWizard::CONTROLLER_CC3D) {
        ui->oneshotESCButton->setEnabled(getWizard()->getInputType() != SetupWizard::INPUT_PWM);
        if (ui->oneshotESCButton->isChecked() && getWizard()->getInputType() == SetupWizard::INPUT_PWM) {
            ui->oneshotESCButton->setChecked(false);
            ui->rapidESCButton->setChecked(true);
        }
    } else {
        ui->oneshotESCButton->setEnabled(true);
    }
}
