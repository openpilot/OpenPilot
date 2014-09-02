/**
 ******************************************************************************
 *
 * @file       multipage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup MultiPage
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

#include "multipage.h"
#include "setupwizard.h"

MultiPage::MultiPage(SetupWizard *wizard, QWidget *parent) :
    SelectionPage(wizard, QString(":/configgadget/images/multirotor-shapes.svg"), parent)
{}

MultiPage::~MultiPage()
{}

bool MultiPage::validatePage(SelectionItem *selectedItem)
{
    getWizard()->setVehicleSubType((SetupWizard::VEHICLE_SUB_TYPE)selectedItem->id());
    return true;
}

void MultiPage::setupSelection(Selection *selection)
{
    selection->setTitle(tr("OpenPilot Multirotor Configuration"));
    selection->setText(tr("This part of the wizard will set up the OpenPilot controller for use with a flying platform utilizing multiple rotors. "
                          "The wizard supports the most common types of multirotors. Other variants of multirotors can be configured by using custom "
                          "configuration options in the Configuration plugin in the GCS.\n\n"
                          "Please select the type of multirotor you want to create a configuration for below:"));

    selection->addItem(tr("Tricopter"),
                       tr("The Tricopter uses three motors and one servo. The servo is used to give yaw authority to the rear motor. "
                          "The front motors are rotating in opposite directions. The Tricopter is known for its sweeping yaw movement and "
                          "it is very well suited for FPV since the front rotors are spread wide apart."),
                       "tri",
                       SetupWizard::MULTI_ROTOR_TRI_Y);

    selection->addItem(tr("Quadcopter X"),
                       tr("The X Quadcopter uses four motors and is the most common multi rotor configuration. Two of the motors rotate clockwise "
                          "and two counter clockwise. The motors positioned diagonal to each other rotate in the same direction. "
                          "This setup is perfect for sport flying and is also commonly used for FPV platforms."),
                       "quad-x",
                       SetupWizard::MULTI_ROTOR_QUAD_X);

    selection->addItem(tr("Quadcopter +"),
                       tr("The Plus(+) Quadcopter uses four motors and is similar to the X Quadcopter but the forward direction is offset by 45 degrees. "
                          "The motors front and rear rotate in clockwise and the motors right and left rotate counter-clockwise. "
                          "This setup was one of the first to be used and is still used for sport flying. This configuration is not that well suited "
                          "for FPV since the fore rotor tend to be in the way of the camera."),
                       "quad-plus",
                       SetupWizard::MULTI_ROTOR_QUAD_PLUS);

    selection->addItem(tr("Hexacopter"),
                       tr("A multirotor with six motors, one motor in front."),
                       "quad-hexa",
                       SetupWizard::MULTI_ROTOR_HEXA);

    selection->addItem(tr("Hexacopter X"),
                       tr("A multirotor with six motors, two motors in front."),
                       "quad-hexa-X",
                       SetupWizard::MULTI_ROTOR_HEXA_X);

    selection->addItem(tr("Hexacopter H"),
                       tr("A multirotor with six motors in two rows."),
                       "quad-hexa-H",
                       SetupWizard::MULTI_ROTOR_HEXA_H);

    selection->addItem(tr("Hexacopter Coax (Y6)"),
                       tr("A multirotor with six motors mounted in a coaxial fashion."),
                       "hexa-coax",
                       SetupWizard::MULTI_ROTOR_HEXA_COAX_Y);
}
