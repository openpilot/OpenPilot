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
    SelectionPage(wizard, QString(":/setupwizard/resources/fixedwing-shapes-wizard-no-numbers.svg"), parent)
{}

AirSpeedPage::~AirSpeedPage()
{}

bool AirSpeedPage::validatePage(SelectionItem *seletedItem)
{
    getWizard()->setAirspeedType((SetupWizard::AIRSPEED_TYPE)seletedItem->id());
    return true;
}

void AirSpeedPage::setupSelection(Selection *selection)
{
    selection->setTitle(tr("OpenPilot Airspeed Sensor Selection"));
    selection->setText(tr("This part of the wizard will help you select and configure  "
                          "flying aircraft utilizing servos. The wizard supports the most common types of fixed-wing "
                          "aircraft, other variants of fixed-wing aircraft can be configured by using custom "
                          "configuration options in the Configuration plugin in the GCS.\n\n"
                          "Please select the type of fixed-wing you want to create a configuration for below:"));
    selection->addItem(tr("Estimated"),
                       tr("This setup expects a traditional airframe using two independent aileron servos "
                          "on their own channel (not connected by Y adapter) plus an elevator and a rudder."),
                       "aileron",
                       SetupWizard::ESTIMATE);

    selection->addItem(tr("EagleTree"),
                       tr("This setup expects a traditional airframe using a single alieron servo or two servos "
                          "connected by a Y adapter plus an elevator and a rudder."),
                       "aileron-single",
                       SetupWizard::EAGLETREE);

    selection->addItem(tr("MS4525 Based"),
                       tr("This setup currently expects a flying-wing setup, an elevon plus rudder setup is not yet "
                          "supported. Setup should include only two elevons, and should explicitly not include a rudder."),
                       "elevon",
                       SetupWizard::MS4525);
}
