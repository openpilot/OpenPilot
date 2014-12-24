/**
 ******************************************************************************
 *
 * @file       powersensorpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup PowerSensorPage
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

#include "powersensorpage.h"
#include "setupwizard.h"

PowerSensorPage::PowerSensorPage(SetupWizard *wizard, QWidget *parent) :
    SelectionPage(wizard, QString(":/setupwizard/resources/sensor-shapes.svg"), parent)
{}

PowerSensorPage::~PowerSensorPage()
{}

void PowerSensorPage::initializePage(VehicleConfigurationSource *settings)
{
    Q_UNUSED(settings);
}

bool PowerSensorPage::validatePage(SelectionItem *selectedItem)
{
    getWizard()->setPowerSensorType((SetupWizard::POWERSENSOR_TYPE)selectedItem->id());
    return true;
}

void PowerSensorPage::setupSelection(Selection *selection)
{
    selection->setTitle(tr("OpenPilot Voltage/Current sensor Selection"));
    selection->setText(tr("Please select the type of sensors you wish to use.\n\n"
                          "Please select your power sensor type data below:"));

    selection->addItem(tr("Disabled"),
                       tr("Power sensor is not used"),
                       "no-power-sensor",
                       SetupWizard::POWERSENSOR_DISABLED);

    selection->addItem(tr("Voltage sensor"),
                       tr("Select this option for a basic sensor with only Voltage monitoring.\n"
                          "This is a simple voltage divider done with two resistors"),
                       "basic-voltage-sensor",
                       SetupWizard::POWERSENSOR_VOLTAGE);
/*
    selection->addItem(tr("APM sensor"),
                       tr("Select this option to use the APM sensor.\n"
                          "Voltage factor : 10\n"
                          "Current factor : 18 (to be adjusted)\n"
                          "    "),
                       "apm-power-sensor",
                       SetupWizard::POWERSENSOR_APM);
*/

    selection->addItem(tr("Generic Sensor"),
                       tr("Select this option for a generic power sensor.\n"
                          ""),
                       //"generic-power-sensor",
                       "apm-power-sensor",
                       SetupWizard::POWERSENSOR_GENERIC);


}
