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
    m_multiPic = new QGraphicsSvgItem();
    m_multiPic->setSharedRenderer(renderer);
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addItem(m_multiPic);
    ui->typeGraphicsView->setScene(scene);

    setupMultiTypesCombo();

    // Default to Quad X since it is the most common setup
    ui->typeCombo->setCurrentIndex(1);
    connect(ui->typeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateImageAndDescription()));
    ui->typeGraphicsView->setSceneRect(m_multiPic->boundingRect());
    ui->typeGraphicsView->fitInView(m_multiPic, Qt::KeepAspectRatio);
}

MultiPage::~MultiPage()
{
    delete ui;
}

void MultiPage::initializePage()
{
    updateAvailableTypes();
    updateImageAndDescription();
}

bool MultiPage::validatePage()
{
    SetupWizard::VEHICLE_SUB_TYPE type = (SetupWizard::VEHICLE_SUB_TYPE) ui->typeCombo->itemData(ui->typeCombo->currentIndex()).toInt();
    getWizard()->setVehicleSubType(type);
    return true;
}

void MultiPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if(m_multiPic) {
        ui->typeGraphicsView->setSceneRect(m_multiPic->boundingRect());
        ui->typeGraphicsView->fitInView(m_multiPic, Qt::KeepAspectRatio);
    }
}

void MultiPage::setupMultiTypesCombo()
{
    ui->typeCombo->addItem(tr("Tricopter"), SetupWizard::MULTI_ROTOR_TRI_Y);
    m_descriptions << tr("The Tricopter uses three motors and one servo. The servo is used to give yaw authority to the rear motor. "
                         "The front motors are rotating in opposite directions. The Tricopter is known for its sweeping yaw movement and "
                         "it is very well suited for FPV since the front rotors are spread wide apart.");

    ui->typeCombo->addItem(tr("Quadcopter X"), SetupWizard::MULTI_ROTOR_QUAD_X);
    m_descriptions << tr("The X Quadcopter uses four motors and is the most common multi rotor configuration. Two of the motors rotate clockwise "
                         "and two counter clockwise. The motors positioned diagonal to each other rotate in the same direction. "
                         "This setup is perfect for sport flying and is also commonly used for FPV platforms.");

    ui->typeCombo->addItem(tr("Quadcopter +"), SetupWizard::MULTI_ROTOR_QUAD_PLUS);
    m_descriptions << tr("The Plus(+) Quadcopter uses four motors and is similar to the X Quadcopter but the forward direction is offset by 45 degrees. "
                         "The motors front and rear rotate in clockwise and the motors right and left rotate counter-clockwise. "
                         "This setup was one of the first to be used and is still used for sport flying. This configuration is not that well suited "
                         "for FPV since the fore rotor tend to be in the way of the camera.");

    ui->typeCombo->addItem(tr("Hexacopter"), SetupWizard::MULTI_ROTOR_HEXA);
    m_descriptions << tr("Hexacopter");

    ui->typeCombo->addItem(tr("Hexacopter Coax (Y6)"), SetupWizard::MULTI_ROTOR_HEXA_COAX_Y);
    m_descriptions << tr("Hexacopter Coax (Y6)");

    ui->typeCombo->addItem(tr("Hexacopter X"), SetupWizard::MULTI_ROTOR_HEXA_H);
    m_descriptions << tr("Hexacopter H");

    // Fredrik Arvidsson(m_thread) 2012-08-26 Disable Octos until further notice
    /*
    ui->typeCombo->addItem(tr("Octocopter"), SetupWizard::MULTI_ROTOR_OCTO);
    m_descriptions << tr("Octocopter");

    ui->typeCombo->addItem(tr("Octocopter Coax X"), SetupWizard::MULTI_ROTOR_OCTO_COAX_X);
    m_descriptions << tr("Octocopter Coax X");

    ui->typeCombo->addItem(tr("Octocopter Coax +"), SetupWizard::MULTI_ROTOR_OCTO_COAX_PLUS);
    m_descriptions << tr("Octocopter Coax +");

    ui->typeCombo->addItem(tr("Octocopter V"), SetupWizard::MULTI_ROTOR_OCTO_V);
    m_descriptions << tr("Octocopter V");
    */
}

void MultiPage::updateAvailableTypes()
{
    /*
    QVariant enable = (getWizard()->getInputType() == SetupWizard::INPUT_PWM) ? QVariant(0) : QVariant(1 | 32);
    ui->typeCombo->model()->setData(ui->typeCombo->model()->index(6, 0), enable, Qt::UserRole - 1);
    ui->typeCombo->model()->setData(ui->typeCombo->model()->index(7, 0), enable, Qt::UserRole - 1);
    ui->typeCombo->model()->setData(ui->typeCombo->model()->index(8, 0), enable, Qt::UserRole - 1);
    ui->typeCombo->model()->setData(ui->typeCombo->model()->index(9, 0), enable, Qt::UserRole - 1);
    */
}

void MultiPage::updateImageAndDescription()
{
    SetupWizard::VEHICLE_SUB_TYPE type = (SetupWizard::VEHICLE_SUB_TYPE) ui->typeCombo->itemData(ui->typeCombo->currentIndex()).toInt();
    QString elementId = "";
    QString description = m_descriptions.at(ui->typeCombo->currentIndex());
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
    m_multiPic->setElementId(elementId);
    ui->typeGraphicsView->setSceneRect(m_multiPic->boundingRect());
    ui->typeGraphicsView->fitInView(m_multiPic, Qt::KeepAspectRatio);

    ui->typeDescription->setText(description);
}
