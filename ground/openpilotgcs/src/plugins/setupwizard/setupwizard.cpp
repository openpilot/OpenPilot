/**
 ******************************************************************************
 *
 * @file       setupwizard.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Setup Wizard  Plugin
 * @{
 * @brief A Wizard to make the initial setup easy for everyone.
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

#include "setupwizard.h"
#include "pages/startpage.h"
#include "pages/endpage.h"
#include "pages/controllerpage.h"
#include "pages/vehiclepage.h"
#include "pages/multipage.h"
#include "pages/fixedwingpage.h"
#include "pages/helipage.h"
#include "pages/surfacepage.h"
#include "pages/notyetimplementedpage.h"

SetupWizard::SetupWizard(QWidget *parent) : QWizard(parent)
{
    setWindowTitle("OpenPilot Setup Wizard");
    m_controllerType = CONTROLLER_UNKNOWN;
    m_vehicleType = VEHICLE_UNKNOWN;
    createPages();
}

int SetupWizard::nextId() const
{
    switch (currentId()) {
        case PAGE_START:
            return PAGE_CONTROLLER;
        case PAGE_CONTROLLER: {
            switch(getControllerType())
            {
                case CONTROLLER_CC:
                case CONTROLLER_CC3D:
                    return PAGE_VEHICLES;
                default:
                    return PAGE_NOTYETIMPLEMENTED;
            }
        }
        case PAGE_VEHICLES: {
            switch(getVehicleType())
            {
                case VEHICLE_FIXEDWING:
                    return PAGE_FIXEDWING;
                case VEHICLE_HELI:
                    return PAGE_HELI;
                case VEHICLE_SURFACE:
                    return PAGE_SURFACE;
                case VEHICLE_MULTI:
                    return PAGE_MULTI;
                default:
                    return PAGE_NOTYETIMPLEMENTED;
            }
        }
        case PAGE_MULTI:
            return PAGE_END;
        case PAGE_NOTYETIMPLEMENTED:
            return PAGE_END;
        default:
            return -1;
    }
}

void SetupWizard::createPages()
{
    setPage(PAGE_START, new StartPage(this));
    setPage(PAGE_CONTROLLER, new ControllerPage(this));
    setPage(PAGE_VEHICLES, new VehiclePage(this));
    setPage(PAGE_MULTI, new MultiPage(this));
    setPage(PAGE_FIXEDWING, new FixedWingPage(this));
    setPage(PAGE_HELI, new HeliPage(this));
    setPage(PAGE_SURFACE, new SurfacePage(this));
    setPage(PAGE_NOTYETIMPLEMENTED, new NotYetImplementedPage(this));
    setPage(PAGE_END, new EndPage(this));

    setStartId(PAGE_START);
}
