/**
 ******************************************************************************
 *
 * @file       biascalibrationpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup BiasCalibrationPage
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

#include <QMessageBox>
#include <QDebug>
#include "biascalibrationpage.h"
#include "ui_biascalibrationpage.h"
#include "setupwizard.h"

BiasCalibrationPage::BiasCalibrationPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::BiasCalibrationPage), m_calibrationUtil(0)
{
    ui->setupUi(this);
    connect(ui->levelButton, SIGNAL(clicked()), this, SLOT(performCalibration()));
}

BiasCalibrationPage::~BiasCalibrationPage()
{
    if (m_calibrationUtil) {
        delete m_calibrationUtil;
    }
    delete ui;
}

bool BiasCalibrationPage::validatePage()
{
    return true;
}

bool BiasCalibrationPage::isComplete() const
{
    return ui->levelButton->isEnabled();
}

void BiasCalibrationPage::enableButtons(bool enable)
{
    ui->levelButton->setEnabled(enable);
    getWizard()->button(QWizard::NextButton)->setEnabled(enable);
    getWizard()->button(QWizard::CancelButton)->setEnabled(enable);
    getWizard()->button(QWizard::BackButton)->setEnabled(enable);
    getWizard()->button(QWizard::CustomButton1)->setEnabled(enable);
    QApplication::processEvents();
}

void BiasCalibrationPage::performCalibration()
{
    if (!getWizard()->getConnectionManager()->isConnected()) {
        QMessageBox msgBox;
        msgBox.setText(tr("An OpenPilot controller must be connected to your computer to perform bias "
                          "calculations.\nPlease connect your OpenPilot controller to your computer and try again."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    enableButtons(false);
    ui->progressLabel->setText(QString(tr("Retrieving data...")));


    if (!m_calibrationUtil) {
        m_calibrationUtil = new BiasCalibrationUtil(BIAS_CYCLES, BIAS_RATE);
    }

    connect(m_calibrationUtil, SIGNAL(progress(long, long)), this, SLOT(calibrationProgress(long, long)));
    connect(m_calibrationUtil, SIGNAL(done(accelGyroBias)), this, SLOT(calibrationDone(accelGyroBias)));
    connect(m_calibrationUtil, SIGNAL(timeout(QString)), this, SLOT(calibrationTimeout(QString)));

    m_calibrationUtil->start();
}

void BiasCalibrationPage::calibrationProgress(long current, long total)
{
    if (ui->levellinProgressBar->maximum() != (int)total) {
        ui->levellinProgressBar->setMaximum((int)total);
    }
    if (ui->levellinProgressBar->value() != (int)current) {
        ui->levellinProgressBar->setValue((int)current);
    }
}

void BiasCalibrationPage::calibrationDone(accelGyroBias bias)
{
    stopCalibration();
    getWizard()->setLevellingBias(bias);
    emit completeChanged();
}

void BiasCalibrationPage::calibrationTimeout(QString message)
{
    stopCalibration();

    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void BiasCalibrationPage::stopCalibration()
{
    if (m_calibrationUtil) {
        disconnect(m_calibrationUtil, SIGNAL(progress(long, long)), this, SLOT(calibrationProgress(long, long)));
        disconnect(m_calibrationUtil, SIGNAL(done(accelGyroBias)), this, SLOT(calibrationDone(accelGyroBias)));
        disconnect(m_calibrationUtil, SIGNAL(timeout(QString)), this, SLOT(calibrationTimeout(QString)));
        ui->progressLabel->setText(QString(tr("<font color='green'>Done!</font>")));
        enableButtons(true);
    }
}
