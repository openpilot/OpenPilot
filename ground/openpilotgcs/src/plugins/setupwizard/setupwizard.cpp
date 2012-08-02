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
#include "pages/inputpage.h"
#include "pages/outputpage.h"
#include "pages/levellingpage.h"
#include "pages/summarypage.h"
#include "pages/flashpage.h"
#include "pages/notyetimplementedpage.h"

SetupWizard::SetupWizard(QWidget *parent) : QWizard(parent),
    m_controllerSelectionMode(CONTROLLER_SELECTION_UNKNOWN), m_controllerType(CONTROLLER_UNKNOWN),
    m_vehicleType(VEHICLE_UNKNOWN), m_inputType(INPUT_UNKNOWN), m_escType(ESC_UNKNOWN),
    m_levellingPerformed(false), m_connectionManager(0)
{
    setWindowTitle("OpenPilot Setup Wizard");
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
                case CONTROLLER_REVO:
                case CONTROLLER_PIPX:
                default:
                    return PAGE_NOTYETIMPLEMENTED;
            }
        }
        case PAGE_VEHICLES: {
            switch(getVehicleType())
            {
                case VEHICLE_MULTI:
                    return PAGE_MULTI;
                case VEHICLE_FIXEDWING:
                    return PAGE_FIXEDWING;
                case VEHICLE_HELI:
                    return PAGE_HELI;
                case VEHICLE_SURFACE:
                    return PAGE_SURFACE;
                default:
                    return PAGE_NOTYETIMPLEMENTED;
            }
        }
        case PAGE_MULTI:
            return PAGE_INPUT;
        case PAGE_INPUT:
            return PAGE_OUTPUT;
        case PAGE_OUTPUT:
        {
            if(getControllerSelectionMode() == CONTROLLER_SELECTION_AUTOMATIC) {
                return PAGE_LEVELLING;
            } else {
                return PAGE_SUMMARY;
            }
        }
        case PAGE_LEVELLING:
            return PAGE_SUMMARY;
        case PAGE_SUMMARY:
            return PAGE_FLASH;
        case PAGE_FLASH:
            return PAGE_END;
        case PAGE_NOTYETIMPLEMENTED:
            return PAGE_END;
        default:
            return -1;
    }
}

QString SetupWizard::getSummaryText()
{
    QString summary = "";
    summary.append(tr("Controller type: "));
    switch(getControllerType())
    {
        case CONTROLLER_CC:
            summary.append(tr("OpenPilot CopterControl"));
            break;
        case CONTROLLER_CC3D:
            summary.append(tr("OpenPilot CopterControl 3D"));
            break;
        case CONTROLLER_REVO:
            summary.append(tr("OpenPilot Revolution"));
            break;
        case CONTROLLER_PIPX:
            summary.append(tr("OpenPilot PipX Radio Modem"));
            break;
        default:
            summary.append(tr("Unknown"));
            break;
    }

    summary.append('\n');
    summary.append(tr("Vehicle type: "));
    switch (getVehicleType())
    {
        case VEHICLE_MULTI:
            summary.append(tr("Multirotor"));
            break;
        case VEHICLE_FIXEDWING:
            summary.append(tr("Fixed wing"));
            break;
        case VEHICLE_HELI:
            summary.append(tr("Helicopter"));
            break;
        case VEHICLE_SURFACE:
            summary.append(tr("Surface vehicle"));
            break;
        default:
            summary.append(tr("Unknown"));
    }

    summary.append('\n');
    summary.append(tr("Input type: "));
    switch (getInputType())
    {
        case INPUT_PWM:
            summary.append(tr("PWM (One cable per channel)"));
            break;
        case INPUT_PPM:
            summary.append(tr("PPM (One cable for all channels)"));
            break;
        case INPUT_SBUS:
            summary.append(tr("Futaba S.Bus"));
            break;
        case INPUT_DSM:
            summary.append(tr("Spectrum satellite"));
            break;
        default:
            summary.append(tr("Unknown"));
    }

    summary.append('\n');
    summary.append(tr("ESC type: "));
    switch (getInputType())
    {
        case ESC_DEFAULT:
            summary.append(tr("Default ESC (50 Hz)"));
            break;
        case ESC_RAPID:
            summary.append(tr("Rapid ESC (400 Hz)"));
            break;
        default:
            summary.append(tr("Unknown"));
    }

    summary.append('\n');
    summary.append(tr("Accel & Gyro bias calibrated: "));
    if (isLevellingPerformed()) {
        summary.append(tr("Yes"));
    }
    else {
        summary.append(tr("No"));
    }

    return summary;
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
    setPage(PAGE_INPUT, new InputPage(this));
    setPage(PAGE_OUTPUT, new OutputPage(this));
    setPage(PAGE_LEVELLING, new LevellingPage(this));
    setPage(PAGE_SUMMARY, new SummaryPage(this));
    setPage(PAGE_FLASH, new FlashPage(this));
    setPage(PAGE_NOTYETIMPLEMENTED, new NotYetImplementedPage(this));
    setPage(PAGE_END, new EndPage(this));

    setStartId(PAGE_START);
}
