/**
 ******************************************************************************
 *
 * @file       escvehiclepage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup [Group]
 * @{
 * @addtogroup ESCVehiclePage
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

#include "escvehiclepage.h"
#include "ui_escvehiclepage.h"
#include "cfg_vehicletypes/vehicleconfig.h"
#include "systemsettings.h"

ESCVehiclePage::ESCVehiclePage(ESCWizard *wizard, QWidget *parent) :
    AbstractWizardPage<ESCWizard>(wizard, parent),
    ui(new Ui::ESCVehiclePage)
{
    ui->setupUi(this);
    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/configgadget/images/multirotor-shapes.svg"));
    m_multiPic = new QGraphicsSvgItem();
    m_multiPic->setSharedRenderer(renderer);
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addItem(m_multiPic);
    ui->typeGraphicsView->setScene(scene);

    //setupVehicleImage(SystemSettings::AIRFRAMETYPE_QUADX);
}

ESCVehiclePage::~ESCVehiclePage()
{
    delete ui;
}

void ESCVehiclePage::initializePage()
{
    //GUIConfigDataUnion configData;

    SystemSettings * systemSettings = SystemSettings::GetInstance(getWizard()->getUAVObjectManager());
    Q_ASSERT(systemSettings);
    SystemSettings::DataFields systemSettingsData = systemSettings->getData();

    for(int i = 0; i < (int)(SystemSettings::GUICONFIGDATA_NUMELEM); i++) {
        //configData.UAVObject[i] = systemSettingsData.;
    }

    setupVehicleImage(systemSettingsData.AirframeType);
}

void ESCVehiclePage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if(m_multiPic) {
        ui->typeGraphicsView->setSceneRect(m_multiPic->boundingRect());
        ui->typeGraphicsView->fitInView(m_multiPic, Qt::KeepAspectRatio);
    }
}

void ESCVehiclePage::setupVehicleImage(quint8 type)
{
    QString elementId;
    switch(type)
    {
        case SystemSettings::AIRFRAMETYPE_TRI:
            elementId = "tri";
            break;
        case SystemSettings::AIRFRAMETYPE_QUADX:
            elementId = "quad-x";
            break;
        case SystemSettings::AIRFRAMETYPE_QUADP:
            elementId = "quad-plus";
            break;
        case SystemSettings::AIRFRAMETYPE_HEXA:
            elementId = "quad-hexa";
            break;
        case SystemSettings::AIRFRAMETYPE_HEXACOAX:
            elementId = "hexa-coax";
            break;
        case SystemSettings::AIRFRAMETYPE_HEXAX:
            elementId = "quad-hexa-H";
            break;
        case SystemSettings::AIRFRAMETYPE_OCTO:
            elementId = "quad-octo";
            break;
        case SystemSettings::AIRFRAMETYPE_OCTOCOAXX:
            elementId = "octo-coax-X";
            break;
        case SystemSettings::AIRFRAMETYPE_OCTOCOAXP:
            elementId = "octo-coax-P";
            break;
        case SystemSettings::AIRFRAMETYPE_OCTOV:
            elementId = "quad-octo-v";
            break;
        default:
            elementId = "";
            break;
    }
    m_multiPic->setElementId(elementId);
    ui->typeGraphicsView->setSceneRect(m_multiPic->boundingRect());
    ui->typeGraphicsView->fitInView(m_multiPic, Qt::KeepAspectRatio);
}
