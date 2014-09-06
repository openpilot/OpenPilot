/**
 ******************************************************************************
 *
 * @file       fixedwingpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup FixedWingPage
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

#include "airspeedpage.h"
#include "setupwizard.h"

AirSpeedPage::AirSpeedPage(SetupWizard *wizard, QWidget *parent) :
    SelectionPage(wizard, QString(":/setupwizard/resources/sensor-shapes.svg"), parent)
{}

AirSpeedPage::~AirSpeedPage()
{}

void AirSpeedPage::initializePage(VehicleConfigurationSource *settings)
{
    // Enable all
    setItemDisabled(-1, false);
    if (settings->getInputType() == VehicleConfigurationSource::INPUT_SBUS ||
            settings->getInputType() == VehicleConfigurationSource::INPUT_DSM2 ||
            settings->getInputType() == VehicleConfigurationSource::INPUT_DSMX10 ||
            settings->getInputType() == VehicleConfigurationSource::INPUT_DSMX11) {
        // Disable non estimated sensors if ports are taken by receivers
        setItemDisabled(VehicleConfigurationSource::AIRSPEED_EAGLETREE, true);
        setItemDisabled(VehicleConfigurationSource::AIRSPEED_MS4525, true);
    }
}

bool AirSpeedPage::validatePage(SelectionItem *selectedItem)
{
    getWizard()->setAirspeedType((SetupWizard::AIRSPEED_TYPE)selectedItem->id());
    return true;
}

void AirSpeedPage::setupSelection(Selection *selection)
{
    selection->setTitle(tr("OpenPilot Airspeed Sensor Selection"));
    selection->setText(tr("This part of the wizard will help you select and configure a way to obtain "
                          "airspeed data. OpenPilot support three methods to achieve this, one is a "
                          "software estimation technique and the other two utilize hardware sensors.\n\n"
                          "Note: if previously selected input combinations use the Flexi-port for input, "
                          "only estimated airspeed will be available.\n\n"));
    selection->addItem(tr("Estimated"),
                       tr("This option uses an intelligent estimation algorithm which utilizes the OpenPilot INS/GPS "
                          "to estimate wind speed and subtract it from ground speed obtained from the GPS.\n\n"
                          "This solution is highly accurate in normal level flight with the drawback of being less "
                          "accurate in rapid altitude changes.\n\n"),
                       "estimated-airspeed-sensor",
                       SetupWizard::AIRSPEED_ESTIMATE);

    selection->addItem(tr("EagleTree"),
                       tr("Select this option to use the Airspeed MicroSensor V3 from EagleTree, this is an accurate "
                          "airspeed sensor that includes on-board Temperature Compensation.\n\n"
                          "Selecting this option will set your board's Flexi-Port in to I2C mode."),
                       "eagletree-speed-sensor",
                       SetupWizard::AIRSPEED_EAGLETREE);

    selection->addItem(tr("MS4525 Based"),
                       tr("Select this option to use an airspeed sensor based on the MS4525DO  pressure transducer "
                          "from Measurement Specialties. This includes the PixHawk sensor and their clones.\n\n"
                          "Selecting this option will set your board's Flexi-Port in to I2C mode."),
                       "ms4525-speed-sensor",
                       SetupWizard::AIRSPEED_MS4525);
}

