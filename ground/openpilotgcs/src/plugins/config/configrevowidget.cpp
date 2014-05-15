/**
 ******************************************************************************
 *
 * @file       ConfigRevoWidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#include "configrevowidget.h"

#include "math.h"
#include <QDebug>
#include <QTimer>
#include <QStringList>
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QThread>
#include <QErrorMessage>
#include <iostream>
#include <QDesktopServices>
#include <QUrl>
#include <attitudestate.h>
#include <attitudesettings.h>
#include <revocalibration.h>
#include <accelgyrosettings.h>
#include <homelocation.h>
#include <accelstate.h>

#include <magstate.h>

#include "assertions.h"
#include "calibration.h"
#include "calibration/calibrationutils.h"
#define sign(x) ((x < 0) ? -1 : 1)

// Uncomment this to enable 6 point calibration on the accels
#define NOISE_SAMPLES 50


// *****************

class Thread : public QThread {
public:
    static void usleep(unsigned long usecs)
    {
        QThread::usleep(usecs);
    }
};

// *****************

ConfigRevoWidget::ConfigRevoWidget(QWidget *parent) :
    ConfigTaskWidget(parent),
    m_ui(new Ui_RevoSensorsWidget()),
    isBoardRotationStored(false)
{
    m_ui->setupUi(this);

    // Initialization of the Paper plane widget
    m_ui->calibrationVisualHelp->setScene(new QGraphicsScene(this));
    m_ui->calibrationVisualHelp->setRenderHint(QPainter::HighQualityAntialiasing, true);
    m_ui->calibrationVisualHelp->setRenderHint(QPainter::SmoothPixmapTransform, true);
    displayVisualHelp("empty");

    // Must set up the UI (above) before setting up the UAVO mappings or refreshWidgetValues
    // will be dealing with some null pointers
    addUAVObject("RevoCalibration");
    addUAVObject("HomeLocation");
    addUAVObject("AttitudeSettings");
    addUAVObject("RevoSettings");
    addUAVObject("AccelGyroSettings");
    autoLoadWidgets();

    // connect the thermalCalibration model to UI
    m_thermalCalibrationModel = new OpenPilot::ThermalCalibrationModel(this);
    m_thermalCalibrationModel->init();

    connect(m_ui->ThermalBiasStart, SIGNAL(clicked()), m_thermalCalibrationModel, SLOT(btnStart()));
    connect(m_ui->ThermalBiasEnd, SIGNAL(clicked()), m_thermalCalibrationModel, SLOT(btnEnd()));
    connect(m_ui->ThermalBiasCancel, SIGNAL(clicked()), m_thermalCalibrationModel, SLOT(btnAbort()));

    connect(m_thermalCalibrationModel, SIGNAL(startEnabledChanged(bool)), m_ui->ThermalBiasStart, SLOT(setEnabled(bool)));
    connect(m_thermalCalibrationModel, SIGNAL(endEnabledChanged(bool)), m_ui->ThermalBiasEnd, SLOT(setEnabled(bool)));
    connect(m_thermalCalibrationModel, SIGNAL(cancelEnabledChanged(bool)), m_ui->ThermalBiasCancel, SLOT(setEnabled(bool)));

    connect(m_thermalCalibrationModel, SIGNAL(displayInstructions(QString, WizardModel::MessageType)),
            this, SLOT(displayInstructions(QString, WizardModel::MessageType)));
//    connect(m_thermalCalibrationModel, SIGNAL(temperatureChanged(QString)), m_ui->textTemperature, SLOT(setText(QString)));
//    connect(m_thermalCalibrationModel, SIGNAL(temperatureGradientChanged(QString)), m_ui->textThermalGradient, SLOT(setText(QString)));
    connect(m_thermalCalibrationModel, SIGNAL(progressChanged(int)), m_ui->thermalBiasProgress, SLOT(setValue(int)));
    // note: init for m_thermalCalibrationModel is done in showEvent to prevent cases wiht "Start" button not enabled due to some itming issue.

    m_sixPointCalibrationModel = new OpenPilot::SixPointCalibrationModel(this);
    // six point calibrations
    connect(m_ui->sixPointsStartAccel, SIGNAL(clicked()), m_sixPointCalibrationModel, SLOT(accelStart()));
    connect(m_ui->sixPointsStartMag, SIGNAL(clicked()), m_sixPointCalibrationModel, SLOT(magStart()));
    connect(m_ui->sixPointsSave, SIGNAL(clicked()), m_sixPointCalibrationModel, SLOT(savePositionData()));

    connect(m_sixPointCalibrationModel, SIGNAL(disableAllCalibrations()), this, SLOT(disableAllCalibrations()));
    connect(m_sixPointCalibrationModel, SIGNAL(enableAllCalibrations()), this, SLOT(enableAllCalibrations()));
    connect(m_sixPointCalibrationModel, SIGNAL(storeAndClearBoardRotation()), this, SLOT(storeAndClearBoardRotation()));
    connect(m_sixPointCalibrationModel, SIGNAL(recallBoardRotation()), this, SLOT(recallBoardRotation()));
    connect(m_sixPointCalibrationModel, SIGNAL(displayInstructions(QString, WizardModel::MessageType, bool)),
            this, SLOT(displayInstructions(QString, WizardModel::MessageType, bool)));
    connect(m_sixPointCalibrationModel, SIGNAL(displayVisualHelp(QString)), this, SLOT(displayVisualHelp(QString)));
    connect(m_sixPointCalibrationModel, SIGNAL(savePositionEnabledChanged(bool)), this->m_ui->sixPointsSave, SLOT(setEnabled(bool)));

    m_levelCalibrationModel = new OpenPilot::LevelCalibrationModel(this);
    // level calibration
    connect(m_ui->boardLevelStart, SIGNAL(clicked()), m_levelCalibrationModel, SLOT(start()));
    connect(m_ui->boardLevelSavePos, SIGNAL(clicked()), m_levelCalibrationModel, SLOT(savePosition()));

    connect(m_levelCalibrationModel, SIGNAL(disableAllCalibrations()), this, SLOT(disableAllCalibrations()));
    connect(m_levelCalibrationModel, SIGNAL(enableAllCalibrations()), this, SLOT(enableAllCalibrations()));
    connect(m_levelCalibrationModel, SIGNAL(displayInstructions(QString, WizardModel::MessageType, bool)),
            this, SLOT(displayInstructions(QString, WizardModel::MessageType, bool)));
    connect(m_levelCalibrationModel, SIGNAL(displayVisualHelp(QString)), this, SLOT(displayVisualHelp(QString)));
    connect(m_levelCalibrationModel, SIGNAL(savePositionEnabledChanged(bool)), this->m_ui->boardLevelSavePos, SLOT(setEnabled(bool)));
    connect(m_levelCalibrationModel, SIGNAL(progressChanged(int)), this->m_ui->boardLevelProgress, SLOT(setValue(int)));

    // Connect the signals
    // gyro zero calibration
    m_gyroBiasCalibrationModel = new OpenPilot::GyroBiasCalibrationModel(this);
    connect(m_ui->gyroBiasStart, SIGNAL(clicked()), m_gyroBiasCalibrationModel, SLOT(start()));

    connect(m_gyroBiasCalibrationModel, SIGNAL(progressChanged(int)), this->m_ui->gyroBiasProgress, SLOT(setValue(int)));

    connect(m_gyroBiasCalibrationModel, SIGNAL(disableAllCalibrations()), this, SLOT(disableAllCalibrations()));
    connect(m_gyroBiasCalibrationModel, SIGNAL(enableAllCalibrations()), this, SLOT(enableAllCalibrations()));
    connect(m_gyroBiasCalibrationModel, SIGNAL(storeAndClearBoardRotation()), this, SLOT(storeAndClearBoardRotation()));
    connect(m_gyroBiasCalibrationModel, SIGNAL(recallBoardRotation()), this, SLOT(recallBoardRotation()));
    connect(m_gyroBiasCalibrationModel, SIGNAL(displayInstructions(QString, WizardModel::MessageType, bool)),
            this, SLOT(displayInstructions(QString, WizardModel::MessageType, bool)));
    connect(m_gyroBiasCalibrationModel, SIGNAL(displayVisualHelp(QString)), this, SLOT(displayVisualHelp(QString)));


    connect(m_ui->hlClearButton, SIGNAL(clicked()), this, SLOT(clearHomeLocation()));

    addWidgetBinding("RevoSettings", "FusionAlgorithm", m_ui->FusionAlgorithm);

    addWidgetBinding("AttitudeSettings", "BoardRotation", m_ui->rollRotation, AttitudeSettings::BOARDROTATION_ROLL);
    addWidgetBinding("AttitudeSettings", "BoardRotation", m_ui->pitchRotation, AttitudeSettings::BOARDROTATION_PITCH);
    addWidgetBinding("AttitudeSettings", "BoardRotation", m_ui->yawRotation, AttitudeSettings::BOARDROTATION_YAW);
    addWidgetBinding("AttitudeSettings", "AccelTau", m_ui->accelTau);

    populateWidgets();
    refreshWidgetsValues();
    m_ui->tabWidget->setCurrentIndex(0);
    enableAllCalibrations();
}

ConfigRevoWidget::~ConfigRevoWidget()
{
    // Do nothing
}

void ConfigRevoWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    updateVisualHelp();
}

void ConfigRevoWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateVisualHelp();
}

void ConfigRevoWidget::updateVisualHelp()
{
    m_ui->calibrationVisualHelp->fitInView(m_ui->calibrationVisualHelp->scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);
}

void ConfigRevoWidget::storeAndClearBoardRotation()
{
    if (!isBoardRotationStored) {
        // Store current board rotation
        isBoardRotationStored = true;
        AttitudeSettings *attitudeSettings = AttitudeSettings::GetInstance(getObjectManager());
        Q_ASSERT(attitudeSettings);
        AttitudeSettings::DataFields data  = attitudeSettings->getData();
        storedBoardRotation[AttitudeSettings::BOARDROTATION_YAW]   = data.BoardRotation[AttitudeSettings::BOARDROTATION_YAW];
        storedBoardRotation[AttitudeSettings::BOARDROTATION_ROLL]  = data.BoardRotation[AttitudeSettings::BOARDROTATION_ROLL];
        storedBoardRotation[AttitudeSettings::BOARDROTATION_PITCH] = data.BoardRotation[AttitudeSettings::BOARDROTATION_PITCH];

        // Set board rotation to no rotation
        data.BoardRotation[AttitudeSettings::BOARDROTATION_YAW]    = 0;
        data.BoardRotation[AttitudeSettings::BOARDROTATION_ROLL]   = 0;
        data.BoardRotation[AttitudeSettings::BOARDROTATION_PITCH]  = 0;
        attitudeSettings->setData(data);
    }
}

void ConfigRevoWidget::recallBoardRotation()
{
    if (isBoardRotationStored) {
        // Recall current board rotation
        isBoardRotationStored = false;

        AttitudeSettings *attitudeSettings = AttitudeSettings::GetInstance(getObjectManager());
        Q_ASSERT(attitudeSettings);
        AttitudeSettings::DataFields data  = attitudeSettings->getData();
        data.BoardRotation[AttitudeSettings::BOARDROTATION_YAW]   = storedBoardRotation[AttitudeSettings::BOARDROTATION_YAW];
        data.BoardRotation[AttitudeSettings::BOARDROTATION_ROLL]  = storedBoardRotation[AttitudeSettings::BOARDROTATION_ROLL];
        data.BoardRotation[AttitudeSettings::BOARDROTATION_PITCH] = storedBoardRotation[AttitudeSettings::BOARDROTATION_PITCH];
        attitudeSettings->setData(data);
    }
}

/**
   Show the selected visual aid
 */
void ConfigRevoWidget::displayVisualHelp(QString elementID)
{
    m_ui->calibrationVisualHelp->scene()->clear();
    QPixmap pixmap = QPixmap(":/configgadget/images/calibration/" + elementID + ".png");
    m_ui->calibrationVisualHelp->scene()->addPixmap(pixmap);
    m_ui->calibrationVisualHelp->setSceneRect(pixmap.rect());
    updateVisualHelp();
}

void ConfigRevoWidget::displayInstructions(QString text, WizardModel::MessageType type, bool clear)
{
    if (clear || text.isEmpty()) {
        m_ui->calibrationInstructions->clear();
    }
    if (!text.isNull()) {
        switch(type) {
        case WizardModel::Error:
            text = QString("<font color='red'>%1</font>").arg(text);
            break;
        case WizardModel::Notice:
            text = QString("<font color='blue'>%1</font>").arg(text);
            break;
        case WizardModel::Info:
        default:
            break;
        }
        m_ui->calibrationInstructions->append(text);
    }
}

/********** UI Functions *************/

/**
 * Called by the ConfigTaskWidget parent when RevoCalibration is updated
 * to update the UI
 */
void ConfigRevoWidget::refreshWidgetsValues(UAVObject *object)
{
    ConfigTaskWidget::refreshWidgetsValues(object);


    m_ui->calibInstructions->setText(QString("Press \"Start\" above to calibrate."));

    m_ui->isSetCheckBox->setEnabled(false);

    HomeLocation *homeLocation = HomeLocation::GetInstance(getObjectManager());
    Q_ASSERT(homeLocation);
    HomeLocation::DataFields homeLocationData = homeLocation->getData();

    QString beStr = QString("%1:%2:%3").arg(QString::number(homeLocationData.Be[0]), QString::number(homeLocationData.Be[1]), QString::number(homeLocationData.Be[2]));
    m_ui->beBox->setText(beStr);
}

void ConfigRevoWidget::clearHomeLocation()
{
    HomeLocation *homeLocation = HomeLocation::GetInstance(getObjectManager());

    Q_ASSERT(homeLocation);
    HomeLocation::DataFields homeLocationData;
    homeLocationData.Latitude  = 0;
    homeLocationData.Longitude = 0;
    homeLocationData.Altitude  = 0;
    homeLocationData.Be[0]     = 0;
    homeLocationData.Be[1]     = 0;
    homeLocationData.Be[2]     = 0;
    homeLocationData.g_e = 9.81f;
    homeLocationData.Set = HomeLocation::SET_FALSE;
    homeLocation->setData(homeLocationData);
}

void ConfigRevoWidget::disableAllCalibrations()
{
    m_ui->sixPointsStartAccel->setEnabled(false);
    m_ui->sixPointsStartMag->setEnabled(false);
    m_ui->boardLevelStart->setEnabled(false);
    m_ui->gyroBiasStart->setEnabled(false);
    m_ui->ThermalBiasStart->setEnabled(false);
}

void ConfigRevoWidget::enableAllCalibrations()
{
    m_ui->sixPointsStartAccel->setEnabled(true);
    m_ui->sixPointsStartMag->setEnabled(true);
    m_ui->boardLevelStart->setEnabled(true);
    m_ui->gyroBiasStart->setEnabled(true);
    m_ui->ThermalBiasStart->setEnabled(true);
}
