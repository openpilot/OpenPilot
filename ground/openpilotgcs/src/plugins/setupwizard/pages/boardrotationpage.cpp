/**
 ******************************************************************************
 *
 * @file       boardrotationpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup
 * @{
 * @addtogroup BoardRotationPage
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


#include "boardrotationpage.h"
#include "ui_boardrotationpage.h"


BoardRotationPage::BoardRotationPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::BoardRotationPage)
{
    ui->setupUi(this);
    m_board3DView = new BoardRotation3DView(ui->verticalLayoutWidget_4);
    m_board3DView->setBoardFilename(QString("%%DATAPATH%%models/boards/CC3D/CC3D.3ds"));
    ui->viewsLayout->addWidget(m_board3DView);
    
    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/setupwizard/resources/vehicle_bg.svg"));
    m_vehicleItem = new QGraphicsSvgItem();
    m_vehicleItem->setSharedRenderer(renderer);
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addItem(m_vehicleItem);
    ui->vehicleView->setScene(scene);
}

BoardRotationPage::~BoardRotationPage()
{
    delete ui;
}

bool BoardRotationPage::validatePage()
{
    boardRotation rotation;

    rotation.m_rollDegree = (float)ui->rollBias->value();
    rotation.m_pitchDegree = (float)ui->pitchBias->value();
    rotation.m_yawDegree = (float)ui->yawBias->value();

    getWizard()->setBoardRotation(rotation);
    return true;
}

void BoardRotationPage::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    if (m_vehicleItem) {
        ui->vehicleView->setSceneRect(m_vehicleItem->boundingRect());
        ui->vehicleView->fitInView(m_vehicleItem, Qt::KeepAspectRatio);
    }

    return;
}

