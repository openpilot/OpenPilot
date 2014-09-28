/**
 ******************************************************************************
 *
 * @file       EscCalibrationPage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup
 * @{
 * @addtogroup EscCalibrationPage
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


#include "esccalibrationpage.h"
#include "ui_esccalibrationpage.h"
#include "setupwizard.h"

EscCalibrationPage::EscCalibrationPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),

    ui(new Ui::EscCalibrationPage)
{
    ui->setupUi(this);
}

EscCalibrationPage::~EscCalibrationPage()
{
    delete ui;
}

bool EscCalibrationPage::validatePage()
{
    return true;
}
