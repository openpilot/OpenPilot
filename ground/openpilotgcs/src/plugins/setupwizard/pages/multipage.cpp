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
#include "ui_multipage.h"
#include "setupwizard.h"

MultiPage::MultiPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::MultiPage)
{
    ui->setupUi(this);    

    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/configgadget/images/multirotor-shapes.svg"));
    multiPic = new QGraphicsSvgItem();
    multiPic->setSharedRenderer(renderer);
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addItem(multiPic);
    ui->typeGraphicsView->setScene(scene);

    setupMultiTypesCombo();

    // Default to Quad X since it is the most common setup
    ui->typeCombo->setCurrentIndex(1);
    connect(ui->typeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateImageAndDescription()));
}

MultiPage::~MultiPage()
{
    delete ui;
}

void MultiPage::initializePage()
{
    updateImageAndDescription();
}

void MultiPage::setupMultiTypesCombo()
{
    ui->typeCombo->addItem("Tricopter", SetupWizard::MULTI_ROTOR_TRI_Y);
    ui->typeCombo->addItem("Quadcopter X", SetupWizard::MULTI_ROTOR_QUAD_X);
    ui->typeCombo->addItem("Quadcopter +", SetupWizard::MULTI_ROTOR_QUAD_PLUS);
    ui->typeCombo->addItem("Hexacopter", SetupWizard::MULTI_ROTOR_HEXA);
    ui->typeCombo->addItem("Hexacopter Coax", SetupWizard::MULTI_ROTOR_HEXA_COAX_Y);
    ui->typeCombo->addItem("Hexacopter H", SetupWizard::MULTI_ROTOR_HEXA_H);
    ui->typeCombo->addItem("Octocopter", SetupWizard::MULTI_ROTOR_OCTO);
    ui->typeCombo->addItem("Octocopter Coax X", SetupWizard::MULTI_ROTOR_OCTO_COAX_X);
    ui->typeCombo->addItem("Octocopter Coax +", SetupWizard::MULTI_ROTOR_OCTO_COAX_PLUS);
    ui->typeCombo->addItem("Octocopter V", SetupWizard::MULTI_ROTOR_OCTO_V);
}

void MultiPage::updateImageAndDescription()
{
    SetupWizard::MULTI_ROTOR_SUB_TYPE type = (SetupWizard::MULTI_ROTOR_SUB_TYPE) ui->typeCombo->itemData(ui->typeCombo->currentIndex()).toInt();
    QString elementId = "";
    QString description = "Descriptive text with information about ";
    description.append(ui->typeCombo->currentText());
    description.append(" multirotors.");
    switch(type)
    {
        case SetupWizard::MULTI_ROTOR_TRI_Y:
            elementId = "tri";
            break;
        case SetupWizard::MULTI_ROTOR_QUAD_X:
            elementId = "quad-x";
            break;
        case SetupWizard::MULTI_ROTOR_QUAD_PLUS:
            elementId = "quad-plus";
            break;
        case SetupWizard::MULTI_ROTOR_HEXA:
            elementId = "quad-hexa";
            break;
        case SetupWizard::MULTI_ROTOR_HEXA_COAX_Y:
            elementId = "hexa-coax";
            break;
        case SetupWizard::MULTI_ROTOR_HEXA_H:
            elementId = "quad-hexa-H";
            break;
        case SetupWizard::MULTI_ROTOR_OCTO:
            elementId = "quad-octo";
            break;
        case SetupWizard::MULTI_ROTOR_OCTO_COAX_X:
            elementId = "octo-coax-X";
            break;
        case SetupWizard::MULTI_ROTOR_OCTO_COAX_PLUS:
            elementId = "octo-coax-P";
            break;
        case SetupWizard::MULTI_ROTOR_OCTO_V:
            elementId = "quad-octo-v";
            break;
        default:
            elementId = "";
            break;
    }
    multiPic->setElementId(elementId);
    ui->typeGraphicsView->setSceneRect(multiPic->boundingRect());
    ui->typeGraphicsView->fitInView(multiPic, Qt::KeepAspectRatio);

    ui->typeDescription->setText(description);
}
