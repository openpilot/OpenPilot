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
    SelectionPage(wizard, QString(":/setupwizard/resources/airspeed-shapes.svg"), parent)
{}

GpsPage::~GpsPage()
{}

bool GpsPage::validatePage(SelectionItem *seletedItem)
{
    getWizard()->setGpsType((SetupWizard::GPS_TYPE)seletedItem->id());
    return true;
}

void GpsPage::setupSelection(Selection *selection)
{
    selection->setTitle(tr("OpenPilot GPS Selection"));
    selection->setText(tr("Please select the type of GPS you have below. As well as OpenPilot hardware "
                          "OpenPilot works hard to support 3rd party GPSs as well, although performance could
                          "be less than with using OpenPilot produced hardware; not all GPSs are created equal.\n\n"
                          "Please select your GPS type data below:"));

    selection->addItem(tr("Disabled"),
                       tr("GPS Features are not to be enabled"),
                       "disabled",
                       SetupWizard::GPS_DISABLED);

    selection->addItem(tr("OpenPilot Platinum"),
                       tr("Select this option to use the OpenPilot Platinum GPS with integrated Magnetometer "
                          "and Microcontroller connected to the Main Port of your controller.\n\n"
                          "Note: for the OpenPilot v8 GPS please select the U-Blox option."),
                       "platinum",
                       SetupWizard::GPS_PLAT);

    selection->addItem(tr("U-Blox Based"),
                       tr("Select this option for the OpenPilot V8 GPS or generic U-Blox chipset GPSs connected"
                          "to the Main Port of your controller."),
                       "ublox",
                       SetupWizard::GPS_UBX);

    selection->addItem(tr("NMEA Based"),
                       tr("Select this option for a generic NMEA based GPS connected to the Main Port of your"
                          "controller."),
                       "nmea",
                       SetupWizard::GPS_NMEA);

}
