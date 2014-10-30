/**
 ******************************************************************************
 *
 * @file       EscCalibrationPage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup
 * @{
 * @addtogroup EscCalibrationPage
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


#include "esccalibrationpage.h"
#include "ui_esccalibrationpage.h"
#include "setupwizard.h"
#include "mixersettings.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "vehicleconfigurationhelper.h"
#include "actuatorsettings.h"

#include <QThread>

EscCalibrationPage::EscCalibrationPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::EscCalibrationPage), m_isCalibrating(false)
{
    ui->setupUi(this);

    ui->outputHigh->setEnabled(false);
    ui->outputLow->setEnabled(true);
    ui->outputLevel->setEnabled(true);
    ui->outputLevel->setText(QString(tr("%1 µs")).arg(OFF_PWM_OUTPUT_PULSE_LENGTH_MICROSECONDS));

    connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));

    connect(ui->securityCheckBox1, SIGNAL(toggled(bool)), this, SLOT(securityCheckBoxesToggled()));
    connect(ui->securityCheckBox2, SIGNAL(toggled(bool)), this, SLOT(securityCheckBoxesToggled()));
    connect(ui->securityCheckBox3, SIGNAL(toggled(bool)), this, SLOT(securityCheckBoxesToggled()));
}

EscCalibrationPage::~EscCalibrationPage()
{
    delete ui;
}

bool EscCalibrationPage::validatePage()
{
    return true;
}

void EscCalibrationPage::enableButtons(bool enable)
{
    getWizard()->button(QWizard::NextButton)->setEnabled(enable);
    getWizard()->button(QWizard::CancelButton)->setEnabled(enable);
    getWizard()->button(QWizard::BackButton)->setEnabled(enable);
    getWizard()->button(QWizard::CustomButton1)->setEnabled(enable);
    ui->securityCheckBox1->setEnabled(enable);
    ui->securityCheckBox2->setEnabled(enable);
    ui->securityCheckBox3->setEnabled(enable);
    QApplication::processEvents();
}

void EscCalibrationPage::resetAllSecurityCheckboxes()
{
    ui->securityCheckBox1->setChecked(false);
    ui->securityCheckBox2->setChecked(false);
    ui->securityCheckBox3->setChecked(false);
}

void EscCalibrationPage::startButtonClicked()
{
    if (!m_isCalibrating) {
        m_isCalibrating = true;
        ui->startButton->setEnabled(false);
        enableButtons(false);
        ui->outputHigh->setEnabled(true);
        ui->outputLow->setEnabled(false);
        ui->nonconnectedLabel->setEnabled(false);
        ui->connectedLabel->setEnabled(true);
        ui->outputLevel->setText(QString(tr("%1 µs")).arg(HIGH_PWM_OUTPUT_PULSE_LENGTH_MICROSECONDS));
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectManager *uavoManager = pm->getObject<UAVObjectManager>();
        Q_ASSERT(uavoManager);
        MixerSettings *mSettings = MixerSettings::GetInstance(uavoManager);
        Q_ASSERT(mSettings);
        QString mixerTypePattern = "Mixer%1Type";
        for (quint32 i = 0; i < ActuatorSettings::CHANNELADDR_NUMELEM; i++) {
            UAVObjectField *field = mSettings->getField(mixerTypePattern.arg(i + 1));
            Q_ASSERT(field);
            if (field->getValue().toString() == field->getOptions().at(VehicleConfigurationHelper::MIXER_TYPE_MOTOR)) {
                OutputCalibrationUtil *output = new OutputCalibrationUtil();
                output->startChannelOutput(i, OFF_PWM_OUTPUT_PULSE_LENGTH_MICROSECONDS);
                output->setChannelOutputValue(HIGH_PWM_OUTPUT_PULSE_LENGTH_MICROSECONDS);
                m_outputs << output;
            }
        }
        ui->stopButton->setEnabled(true);
    }
}

void EscCalibrationPage::stopButtonClicked()
{
    if (m_isCalibrating) {
        ui->stopButton->setEnabled(false);
        ui->outputHigh->setEnabled(false);

        // Set to low pwm out
        foreach(OutputCalibrationUtil * output, m_outputs) {
            output->setChannelOutputValue(LOW_PWM_OUTPUT_PULSE_LENGTH_MICROSECONDS);
        }
        ui->outputLevel->setText(QString(tr("%1 µs")).arg(LOW_PWM_OUTPUT_PULSE_LENGTH_MICROSECONDS));
        QApplication::processEvents();
        QThread::msleep(2000);

        // Ramp down to off pwm out
        for (int i = LOW_PWM_OUTPUT_PULSE_LENGTH_MICROSECONDS; i >= OFF_PWM_OUTPUT_PULSE_LENGTH_MICROSECONDS; i -= 10) {
            foreach(OutputCalibrationUtil * output, m_outputs) {
                output->setChannelOutputValue(i);
            }
            ui->outputLevel->setText(QString(tr("%1 µs")).arg(i));
            QApplication::processEvents();
            QThread::msleep(200);
        }

        // Stop output
        foreach(OutputCalibrationUtil * output, m_outputs) {
            output->stopChannelOutput();
            delete output;
        }
        ui->outputLevel->setText(QString(tr("%1 µs")).arg(OFF_PWM_OUTPUT_PULSE_LENGTH_MICROSECONDS));
        ui->outputHigh->setEnabled(false);
        ui->outputLow->setEnabled(true);
        ui->nonconnectedLabel->setEnabled(true);
        ui->connectedLabel->setEnabled(false);
        m_outputs.clear();
        m_isCalibrating = false;
        resetAllSecurityCheckboxes();
        enableButtons(true);
    }
}

void EscCalibrationPage::securityCheckBoxesToggled()
{
    ui->startButton->setEnabled(ui->securityCheckBox1->isChecked() &&
                                ui->securityCheckBox2->isChecked() &&
                                ui->securityCheckBox3->isChecked());
}


void EscCalibrationPage::initializePage()
{
    resetAllSecurityCheckboxes();
}
