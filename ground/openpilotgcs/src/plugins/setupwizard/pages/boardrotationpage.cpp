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


#include <QtDebug>
#include "boardrotationpage.h"
#include "ui_boardrotationpage.h"

#ifndef SHARE_PATH
#ifdef Q_OS_MAC
    #define SHARE_PATH "/../Resources"
#else
    #define SHARE_PATH "/../share/openpilotgcs"
#endif /* Q_OS_MAC */
#endif /* SHARE_PATH */


BoardRotationPage::BoardRotationPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent), m_board3DView(NULL),
    m_prevRoll(0), m_prevPitch(0), m_prevYaw(0),
    ui(new Ui::BoardRotationPage)
{
    ui->setupUi(this);

    QDir directory(QCoreApplication::applicationDirPath() + SHARE_PATH + QString("/models/boards/"));
    QString fileName = directory.absolutePath() + QDir::separator();
    switch (getWizard()->getControllerType()) {
    case VehicleConfigurationSource::CONTROLLER_REVO:
    case VehicleConfigurationSource::CONTROLLER_NANO:
        fileName += QString("Revolution/Revolution.3DS");
        break;
    case VehicleConfigurationSource::CONTROLLER_CC:
    case VehicleConfigurationSource::CONTROLLER_CC3D:
    default:
        fileName += QString("CC3D/CC3D.3ds");
        break;
    }

    m_board3DView = new BoardRotation3DView(this, fileName);
    m_board3DView->setGeometry(QRect(12, 185, 750, 387));

    connect(ui->rollBias, SIGNAL(valueChanged(int)), this, SLOT(onRollChanged()));
    connect(ui->pitchBias, SIGNAL(valueChanged(int)), this, SLOT(onPitchChanged()));
    connect(ui->yawBias, SIGNAL(valueChanged(int)), this, SLOT(onYawChanged()));
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

void BoardRotationPage::onRollChanged()
{
    int changed(ui->rollBias->value() - m_prevRoll);
    if ((changed > 1) || (changed < -1)) {
        m_board3DView->rollRotation(-m_prevRoll);
        m_board3DView->rollRotation(ui->rollBias->value());
    } else {
        m_board3DView->rollRotation(changed);
    }

    m_prevRoll = ui->rollBias->value();
}

void BoardRotationPage::onPitchChanged()
{
    int changed(ui->pitchBias->value() - m_prevPitch);
    if ((changed > 1) || (changed < -1)) {
        m_board3DView->pitchRotation(-m_prevPitch);
        m_board3DView->pitchRotation(ui->pitchBias->value());
    } else {
        m_board3DView->pitchRotation(changed);
    }

    m_prevPitch = ui->pitchBias->value();
}

void BoardRotationPage::onYawChanged()
{
    int changed(ui->yawBias->value() - m_prevYaw);
    if ((changed > 1) || (changed < -1)) {
        m_board3DView->yawRotation(-m_prevYaw);
        m_board3DView->yawRotation(ui->yawBias->value());
    } else {
        m_board3DView->yawRotation(changed);
    }

    m_prevYaw = ui->yawBias->value();
}

