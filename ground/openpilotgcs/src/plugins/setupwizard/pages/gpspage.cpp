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

#include "gpspage.h"
#include "setupwizard.h"

GpsPage::GpsPage(SetupWizard *wizard, QWidget *parent) :
    SelectionPage(wizard, QString(":/setupwizard/resources/sensor-shapes.svg"), parent)
{}

GpsPage::~GpsPage()
{}

bool GpsPage::validatePage(SelectionItem *selectedItem)
{
    getWizard()->setGpsType((SetupWizard::GPS_TYPE)selectedItem->id());
    return true;
}

void GpsPage::setupSelection(Selection *selection)
{
    selection->setTitle(tr("OpenPilot GPS Selection"));
    selection->setText(tr("Please select the type of GPS you wish to use. As well as OpenPilot hardware, "
                          "3rd party GPSs are supported also, although please note that performance could "
                          "be less than optimal as not all GPSs are created equal.\n\n"
                          "Note: NMEA only GPSs perform poorly on VTOL aircraft and are not recommended for Helis and MultiRotors.\n\n"
                          "Please select your GPS type data below:"));

    selection->addItem(tr("Disabled"),
                       tr("GPS Features are not to be enabled"),
                       "no-gps",
                       SetupWizard::GPS_DISABLED);

    selection->addItem(tr("OpenPilot Platinum"),
                       tr("Select this option to use the OpenPilot Platinum GPS with integrated Magnetometer "
                          "and Microcontroller connected to the Main Port of your controller.\n\n"
                          "Note: for the OpenPilot v8 GPS please select the U-Blox option."),
                       "OPGPS-v9",
                       SetupWizard::GPS_PLAT);

    selection->addItem(tr("U-Blox Based"),
                       tr("Select this option for the OpenPilot V8 GPS or generic U-Blox chipset GPSs connected"
                          "to the Main Port of your controller."),
                       "OPGPS-v8-ublox",
                       SetupWizard::GPS_UBX);

    selection->addItem(tr("NMEA Based"),
                       tr("Select this option for a generic NMEA based GPS connected to the Main Port of your"
                          "controller."),
                       "generic-nmea",
                       SetupWizard::GPS_NMEA);

}
