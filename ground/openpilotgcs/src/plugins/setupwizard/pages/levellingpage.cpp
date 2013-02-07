/**
 ******************************************************************************
 *
 * @file       levellingpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup LevellingPage
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
#include "levellingpage.h"
#include "ui_levellingpage.h"
#include "setupwizard.h"

LevellingPage::LevellingPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::LevellingPage),  m_levellingUtil(0)
{
    ui->setupUi(this);
    connect(ui->levelButton, SIGNAL(clicked()), this, SLOT(performLevelling()));
}

LevellingPage::~LevellingPage()
{
    if(m_levellingUtil) {
        delete m_levellingUtil;
    }
    delete ui;
}

bool LevellingPage::validatePage()
{
    return true;
}

bool LevellingPage::isComplete() const
{
    //const_cast<LevellingPage *>(this)->getWizard()->isLevellingPerformed() &&
    return ui->levelButton->isEnabled();
}

void LevellingPage::enableButtons(bool enable)
{
    ui->levelButton->setEnabled(enable);
    getWizard()->button(QWizard::NextButton)->setEnabled(enable);
    getWizard()->button(QWizard::CancelButton)->setEnabled(enable);
    getWizard()->button(QWizard::BackButton)->setEnabled(enable);
    getWizard()->button(QWizard::CustomButton1)->setEnabled(enable);
    QApplication::processEvents();
}

void LevellingPage::performLevelling()
{
    if(!getWizard()->getConnectionManager()->isConnected()) {
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


    if(!m_levellingUtil)
    {
        m_levellingUtil = new LevellingUtil(BIAS_CYCLES, BIAS_RATE);
    }

    connect(m_levellingUtil, SIGNAL(progress(long,long)), this, SLOT(levellingProgress(long,long)));
    connect(m_levellingUtil, SIGNAL(done(accelGyroBias)), this, SLOT(levellingDone(accelGyroBias)));
    connect(m_levellingUtil, SIGNAL(timeout(QString)), this, SLOT(levellingTimeout(QString)));

    m_levellingUtil->start();
}

void LevellingPage::levellingProgress(long current, long total)
{
    if(ui->levellinProgressBar->maximum() != (int)total) {
        ui->levellinProgressBar->setMaximum((int)total);
    }
    if(ui->levellinProgressBar->value() != (int)current) {
        ui->levellinProgressBar->setValue((int)current);
    }
}

void LevellingPage::levellingDone(accelGyroBias bias)
{
    stopLevelling();
    getWizard()->setLevellingBias(bias);
    emit completeChanged();
}

void LevellingPage::levellingTimeout(QString message)
{
    stopLevelling();

    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void LevellingPage::stopLevelling()
{
    if(m_levellingUtil)
    {
        disconnect(m_levellingUtil, SIGNAL(progress(long,long)), this, SLOT(levellingProgress(long,long)));
        disconnect(m_levellingUtil, SIGNAL(done(accelGyroBias)), this, SLOT(levellingDone(accelGyroBias)));
        disconnect(m_levellingUtil, SIGNAL(timeout(QString)), this, SLOT(levellingTimeout(QString)));
        ui->progressLabel->setText(QString(tr("<font color='green'>Done!</font>")));
        enableButtons(true);
    }
}
