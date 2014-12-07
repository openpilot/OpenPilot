/**
 ******************************************************************************
 *
 * @file       vehiclepage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup VehiclePage
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

#include "vehiclepage.h"
#include "ui_vehiclepage.h"

VehiclePage::VehiclePage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::VehiclePage)
{
    ui->setupUi(this);
}

VehiclePage::~VehiclePage()
{
    delete ui;
}

bool VehiclePage::validatePage()
{
    if (ui->multirotorButton->isChecked()) {
        getWizard()->setVehicleType(SetupWizard::VEHICLE_MULTI);
    } else if (ui->fixedwingButton->isChecked()) {
        getWizard()->setVehicleType(SetupWizard::VEHICLE_FIXEDWING);
        getWizard()->setEscType(SetupWizard::ESC_STANDARD);
    } else if (ui->heliButton->isChecked()) {
        getWizard()->setVehicleType(SetupWizard::VEHICLE_HELI);
    } else if (ui->surfaceButton->isChecked()) {
        getWizard()->setVehicleType(SetupWizard::VEHICLE_SURFACE);
        getWizard()->setEscType(SetupWizard::ESC_STANDARD);
    } else {
        getWizard()->setVehicleType(SetupWizard::VEHICLE_UNKNOWN);
    }
    return true;
}

void VehiclePage::initializePage()
{
    // ui->fixedwingButton->setEnabled(getWizard()->getControllerType() == SetupWizard::CONTROLLER_REVO ||
    // getWizard()->getControllerType() == SetupWizard::CONTROLLER_NANO);
}
