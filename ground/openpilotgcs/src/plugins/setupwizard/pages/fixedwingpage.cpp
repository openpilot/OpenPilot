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

#include "fixedwingpage.h"
#include "ui_fixedwingpage.h"
#include "setupwizard.h"

FixedWingPage::FixedWingPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::FixedWingPage)
{
    ui->setupUi(this);
    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/configgadget/images/fixedwing-shapes.svg"));
    m_fixedwingPic = new QGraphicsSvgItem();
    m_fixedwingPic->setSharedRenderer(renderer);
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addItem(m_fixedwingPic);
    ui->typeGraphicsView->setScene(scene);

    setupFixedWingTypesCombo();

    // Default to Aileron setup
    ui->typeCombo->setCurrentIndex(0);
    connect(ui->typeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateImageAndDescription()));
    ui->typeGraphicsView->setSceneRect(m_fixedwingPic->boundingRect());
    ui->typeGraphicsView->fitInView(m_fixedwingPic, Qt::KeepAspectRatio);

}

FixedWingPage::~FixedWingPage()
{
    delete ui;
}

void FixedWingPage::initializePage()
{
    updateAvailableTypes();
    updateImageAndDescription();
}

bool FixedWingPage::validatePage()
{
    SetupWizard::VEHICLE_SUB_TYPE type = (SetupWizard::VEHICLE_SUB_TYPE)ui->typeCombo->itemData(ui->typeCombo->currentIndex()).toInt();

    getWizard()->setVehicleSubType(type);
    return true;
}

void FixedWingPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_fixedwingPic) {
        ui->typeGraphicsView->setSceneRect(m_fixedwingPic->boundingRect());
        ui->typeGraphicsView->fitInView(m_fixedwingPic, Qt::KeepAspectRatio);
    }
}

void FixedWingPage::setupFixedWingTypesCombo()
{
    ui->typeCombo->addItem(tr("Aileron, Elevator, Rudder"), SetupWizard::FIXED_WING_AILERON);
    m_descriptions << tr("This setup currently expects a traditional 4 channel setup including two ailerons (not connected by Y adapter), an elevator and a rudder. ");

    ui->typeCombo->addItem(tr("Elevon"), SetupWizard::FIXED_WING_ELEVON);
    m_descriptions << tr("This setup currently expects a flying-wing setup, an elevon plus rudder setup is not yet supported. Setup should include only two elevons, and should explicitly not include a rudder.");
}

void FixedWingPage::updateAvailableTypes()
{
}

void FixedWingPage::updateImageAndDescription()
{


    SetupWizard::VEHICLE_SUB_TYPE type = (SetupWizard::VEHICLE_SUB_TYPE)ui->typeCombo->itemData(ui->typeCombo->currentIndex()).toInt();
    QString elementId   = "";
    QString description = m_descriptions.at(ui->typeCombo->currentIndex());

    switch (type) {
    case SetupWizard::FIXED_WING_AILERON:
        elementId = "aileron";
        break;
    case SetupWizard::FIXED_WING_ELEVON:
        elementId = "elevon";
        break;
//    case SetupWizard::FIXED_WING_VTAIL:
//        elementId = "vtail";
//        break;
    default:
        elementId = "";
        break;
    }
    m_fixedwingPic->setElementId(elementId);
    ui->typeGraphicsView->setSceneRect(m_fixedwingPic->boundingRect());
    ui->typeGraphicsView->fitInView(m_fixedwingPic, Qt::KeepAspectRatio);

    ui->typeDescription->setText(description);

}

