/**
 ******************************************************************************
 *
 * @file       surfacepage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup
 * @{
 * @addtogroup SurfacePage
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

#include "surfacepage.h"
#include "setupwizard.h"

SurfacePage::SurfacePage(SetupWizard *wizard, QWidget *parent) :
    SelectionPage(wizard, QString(":/setupwizard/resources/ground-shapes-wizard-no-numbers.svg"), parent)
{}

SurfacePage::~SurfacePage()
{}

void SurfacePage::initializePage(VehicleConfigurationSource *settings)
{
    Q_UNUSED(settings);
}

bool SurfacePage::validatePage(SelectionItem *selectedItem)
{
    getWizard()->setVehicleSubType((SetupWizard::VEHICLE_SUB_TYPE)selectedItem->id());
    return true;
}

void SurfacePage::setupSelection(Selection *selection)
{
    selection->setTitle(tr("OpenPilot Ground Vehicle Configuration"));
    selection->setText(tr("This part of the wizard will set up the OpenPilot controller for use with a ground "
                          "vehicle utilizing servos. The wizard supports the most common types of ground vehicle, "
                          "other variants can be configured by using custom configuration options in the "
                          "Configuration plugin in the GCS.\n\n"
                          "Please select the type of ground vehicle you want to create a configuration for below:"));

    selection->addItem(tr("Car"),
                       tr("This setup expects a traditional car with a rear motor and a front streering servo"),
                       "car",
                       SetupWizard::GROUNDVEHICLE_CAR);

    selection->addItem(tr("Tank"),
                       tr("This setup expects a traditional vehicle using only two motors and differential steering"),
                       "tank",
                       SetupWizard::GROUNDVEHICLE_DIFFERENTIAL);

    selection->addItem(tr("Motorcycle"),
                       tr("This setup currently expects a motorcyle setup, using one motor and one servo for steering."),
                       "motorbike",
                       SetupWizard::GROUNDVEHICLE_MOTORCYCLE);
}
