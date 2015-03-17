/**
 ******************************************************************************
 *
 * @file       airframeinitialtuningpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup
 * @{
 * @addtogroup AirframeInitialTuningPage
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

#include "airframeinitialtuningpage.h"
#include "ui_airframeinitialtuningpage.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include "vehicletemplateexportdialog.h"
#include "utils/pathutils.h"

AirframeInitialTuningPage::AirframeInitialTuningPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::AirframeInitialTuningPage)
{
    ui->setupUi(this);
}

AirframeInitialTuningPage::~AirframeInitialTuningPage()
{
    delete ui;
}

void AirframeInitialTuningPage::initializePage()
{
    const char *path;

    switch (getWizard()->getVehicleType()) {
    case VehicleConfigurationSource::VEHICLE_FIXEDWING:
        path = VehicleTemplateExportDialog::EXPORT_FIXEDWING_NAME;
        break;
    case VehicleConfigurationSource::VEHICLE_MULTI:
        path = VehicleTemplateExportDialog::EXPORT_MULTI_NAME;
        break;
    case VehicleConfigurationSource::VEHICLE_HELI:
        path = VehicleTemplateExportDialog::EXPORT_HELI_NAME;
        break;
    case VehicleConfigurationSource::VEHICLE_SURFACE:
        path = VehicleTemplateExportDialog::EXPORT_SURFACE_NAME;
        break;
    default:
        path = NULL;
        break;
    }
    ui->selectorWidget->setTemplateInfo(path, getWizard()->getVehicleType(), getWizard()->getVehicleSubType());
}

bool AirframeInitialTuningPage::validatePage()
{
    QJsonObject *templ = ui->selectorWidget->selectedTemplate();

    if (getWizard()->getVehicleTemplate() != NULL) {
        delete getWizard()->getVehicleTemplate();
    }
    getWizard()->setVehicleTemplate(templ != NULL ? new QJsonObject(*templ) : NULL);

    return true;
}

bool AirframeInitialTuningPage::isComplete() const
{
    return true;
}
