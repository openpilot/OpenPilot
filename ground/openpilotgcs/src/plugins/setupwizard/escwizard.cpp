/**
 ******************************************************************************
 *
 * @file       escwizard.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup [Group]
 * @{
 * @addtogroup ESCWizard
 * @{
 * @brief [Brief]
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

#include "escwizard.h"
#include "pages/escstartpage.h"
#include "pages/escvehiclepage.h"
#include "pages/escupdateratepage.h"
#include <pages/esccalibrationpage.h>
#include <pages/escendpage.h>


ESCWizard::ESCWizard(QWidget *parent) :
    AbstractWizard(parent)
{
    setWindowTitle(tr("OpenPilot ESC Calibration Wizard"));
    setOption(QWizard::IndependentPages, false);
    setWizardStyle(QWizard::ModernStyle);
    setMinimumSize(600, 450);
    resize(600, 450);
    createPages();
}

int ESCWizard::nextId() const
{
    switch (currentId()) {
        case PAGE_START: return PAGE_VEHICLE;
        case PAGE_VEHICLE: return PAGE_UPDATERATE;
        case PAGE_UPDATERATE: return PAGE_CALIBRATION;
        case PAGE_CALIBRATION: return PAGE_END;
        default: return -1;
    }
}

void ESCWizard::createPages()
{
    setPage(PAGE_START, new ESCStartPage(this));
    setPage(PAGE_VEHICLE, new ESCVehiclePage(this));
    setPage(PAGE_UPDATERATE, new ESCUpdateRatePage(this));
    setPage(PAGE_CALIBRATION, new ESCCalibrationPage(this));
    setPage(PAGE_END, new ESCEndPage(this));
}
