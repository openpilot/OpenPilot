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
#include <ekfconfiguration.h>
#include <revocalibration.h>
#include <accelgyrosettings.h>
#include <homelocation.h>
#include <accelstate.h>
#include <gyrostate.h>
#include <magstate.h>

#include "assertions.h"
#include "calibration.h"
#include "calibration/calibrationutils.h"
#define sign(x) ((x < 0) ? -1 : 1)

// Uncomment this to enable 6 point calibration on the accels
#define SAMPLE_SIZE 40
const double ConfigRevoWidget::maxVarValue = 0.1;

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
    collectingData(false),
    position(-1),
    isBoardRotationStored(false)
{
    m_ui->setupUi(this);

    // Initialization of the Paper plane widget
    m_ui->calibrationVisualHelp->setScene(new QGraphicsScene(this));
    displayVisualHelp("snow");
    // Must set up the UI (above) before setting up the UAVO mappings or refreshWidgetValues
    // will be dealing with some null pointers
    addUAVObject("RevoCalibration");
    addUAVObject("EKFConfiguration");
    addUAVObject("HomeLocation");
    addUAVObject("AttitudeSettings");
    addUAVObject("RevoSettings");
    addUAVObject("AccelGyroSettings");
    autoLoadWidgets();

    // connect the thermalCalibration model to UI
    m_thermalCalibrationModel = new OpenPilot::ThermalCalibrationModel(this);

    connect(m_ui->ThermalBiasStart, SIGNAL(clicked()), m_thermalCalibrationModel, SLOT(btnStart()));
    connect(m_ui->ThermalBiasEnd, SIGNAL(clicked()), m_thermalCalibrationModel, SLOT(btnEnd()));
    connect(m_ui->ThermalBiasCancel, SIGNAL(clicked()), m_thermalCalibrationModel, SLOT(btnAbort()));

    connect(m_thermalCalibrationModel, SIGNAL(startEnabledChanged(bool)), m_ui->ThermalBiasStart, SLOT(setEnabled(bool)));
    connect(m_thermalCalibrationModel, SIGNAL(endEnabledChanged(bool)), m_ui->ThermalBiasEnd, SLOT(setEnabled(bool)));
    connect(m_thermalCalibrationModel, SIGNAL(cancelEnabledChanged(bool)), m_ui->ThermalBiasCancel, SLOT(setEnabled(bool)));

    connect(m_thermalCalibrationModel, SIGNAL(instructionsChanged(QString)), m_ui->label_thermalDescription, SLOT(setText(QString)));
    connect(m_thermalCalibrationModel, SIGNAL(temperatureChanged(QString)), m_ui->textTemperature, SLOT(setText(QString)));
    connect(m_thermalCalibrationModel, SIGNAL(temperatureGradientChanged(QString)), m_ui->textThermalGradient, SLOT(setText(QString)));
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
    connect(m_sixPointCalibrationModel, SIGNAL(displayInstructions(QString, bool)), this, SLOT(displayInstructions(QString, bool)));
    connect(m_sixPointCalibrationModel, SIGNAL(displayVisualHelp(QString)), this, SLOT(displayVisualHelp(QString)));
    connect(m_sixPointCalibrationModel, SIGNAL(savePositionEnabledChanged(bool)), this->m_ui->sixPointsSave, SLOT(setEnabled(bool)));

    // Connect the signals
    // gyro zero calibration
    connect(m_ui->gyroBiasStart, SIGNAL(clicked()), this, SLOT(gyroCalibrationStart()));

    // level calibration
    connect(m_ui->boardLevelStart, SIGNAL(clicked()), this, SLOT(levelCalibrationStart()));
    connect(m_ui->boardLevelSavePos, SIGNAL(clicked()), this, SLOT(levelCalibrationSavePosition()));


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
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a sensorsBargraph that is way too small.
    m_ui->calibrationVisualHelp->fitInView(m_ui->calibrationVisualHelp->scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);
    m_thermalCalibrationModel->init();
}

void ConfigRevoWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_ui->calibrationVisualHelp->fitInView(m_ui->calibrationVisualHelp->scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);
}

/******* gyro bias zero ******/
void ConfigRevoWidget::gyroCalibrationStart()
{
    // Store and reset board rotation before calibration starts
    isBoardRotationStored = false;
    storeAndClearBoardRotation();

    disableAllCalibrations();
    m_ui->gyroBiasProgress->setValue(0);

    RevoCalibration *revoCalibration = RevoCalibration::GetInstance(getObjectManager());
    Q_ASSERT(revoCalibration);
    RevoCalibration::DataFields revoCalibrationData = revoCalibration->getData();
    revoCalibrationData.BiasCorrectedRaw = RevoCalibration::BIASCORRECTEDRAW_FALSE;
    revoCalibration->setData(revoCalibrationData);
    revoCalibration->updated();

    // Disable gyro bias correction while calibrating
    AttitudeSettings *attitudeSettings = AttitudeSettings::GetInstance(getObjectManager());
    Q_ASSERT(attitudeSettings);
    AttitudeSettings::DataFields attitudeSettingsData = attitudeSettings->getData();
    attitudeSettingsData.BiasCorrectGyro = AttitudeSettings::BIASCORRECTGYRO_FALSE;
    attitudeSettings->setData(attitudeSettingsData);
    attitudeSettings->updated();

    gyro_accum_x.clear();
    gyro_accum_y.clear();
    gyro_accum_z.clear();

    UAVObject::Metadata mdata;

    GyroState *gyroState = GyroState::GetInstance(getObjectManager());
    Q_ASSERT(gyroState);
    initialGyroStateMdata = gyroState->getMetadata();
    mdata = initialGyroStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    gyroState->setMetadata(mdata);

    // Now connect to the accels and mag updates, gather for 100 samples
    collectingData = true;
    connect(gyroState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(gyroBiasCalibrationGetSample(UAVObject *)));
}

/**
   Updates the gyro bias raw values
 */
void ConfigRevoWidget::gyroBiasCalibrationGetSample(UAVObject *obj)
{
    QMutexLocker lock(&sensorsUpdateLock);

    Q_UNUSED(lock);

    switch (obj->getObjID()) {
    case GyroState::OBJID:
    {
        GyroState *gyroState = GyroState::GetInstance(getObjectManager());
        Q_ASSERT(gyroState);
        GyroState::DataFields gyroStateData = gyroState->getData();

        gyro_accum_x.append(gyroStateData.x);
        gyro_accum_y.append(gyroStateData.y);
        gyro_accum_z.append(gyroStateData.z);
        break;
    }
    default:
        Q_ASSERT(0);
    }

    // Work out the progress based on whichever has less
    double p1 = (double)gyro_accum_x.size() / (double)NOISE_SAMPLES;
    double p2 = (double)gyro_accum_y.size() / (double)NOISE_SAMPLES;
    m_ui->gyroBiasProgress->setValue(((p1 < p2) ? p1 : p2) * 50);

    if (gyro_accum_y.size() >= 2 * NOISE_SAMPLES &&
        collectingData == true) {
        collectingData = false;

        GyroState *gyroState = GyroState::GetInstance(getObjectManager());

        disconnect(gyroState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(levelCalibrationGetSample(UAVObject *)));

        enableAllCalibrations();

        RevoCalibration *revoCalibration     = RevoCalibration::GetInstance(getObjectManager());
        Q_ASSERT(revoCalibration);
        AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(getObjectManager());
        Q_ASSERT(accelGyroSettings);

        RevoCalibration::DataFields revoCalibrationData     = revoCalibration->getData();
        AccelGyroSettings::DataFields accelGyroSettingsData = accelGyroSettings->getData();

        revoCalibrationData.BiasCorrectedRaw = RevoCalibration::BIASCORRECTEDRAW_TRUE;

        // Update biases based on collected data
        accelGyroSettingsData.gyro_bias[AccelGyroSettings::GYRO_BIAS_X] += OpenPilot::CalibrationUtils::listMean(gyro_accum_x);
        accelGyroSettingsData.gyro_bias[AccelGyroSettings::GYRO_BIAS_Y] += OpenPilot::CalibrationUtils::listMean(gyro_accum_y);
        accelGyroSettingsData.gyro_bias[AccelGyroSettings::GYRO_BIAS_Z] += OpenPilot::CalibrationUtils::listMean(gyro_accum_z);

        revoCalibration->setData(revoCalibrationData);
        revoCalibration->updated();
        accelGyroSettings->setData(accelGyroSettingsData);
        accelGyroSettings->updated();

        AttitudeSettings *attitudeSettings = AttitudeSettings::GetInstance(getObjectManager());
        Q_ASSERT(attitudeSettings);
        AttitudeSettings::DataFields attitudeSettingsData = attitudeSettings->getData();
        attitudeSettingsData.BiasCorrectGyro = AttitudeSettings::BIASCORRECTGYRO_TRUE;
        attitudeSettings->setData(attitudeSettingsData);
        attitudeSettings->updated();

        gyroState->setMetadata(initialGyroStateMdata);

        // Recall saved board rotation
        recallBoardRotation();
    }
}


/******* Level calibration *******/
/**
 * Starts an accelerometer bias calibration.
 */
void ConfigRevoWidget::levelCalibrationStart()
{
    // Store and reset board rotation before calibration starts

    disableAllCalibrations();
    m_ui->boardLevelProgress->setValue(0);

    rot_data_pitch = 0;
    rot_data_roll  = 0;
    UAVObject::Metadata mdata;

    AttitudeState *attitudeState = AttitudeState::GetInstance(getObjectManager());
    Q_ASSERT(attitudeState);
    initialAttitudeStateMdata = attitudeState->getMetadata();
    mdata = initialAttitudeStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    attitudeState->setMetadata(mdata);

    /* Show instructions and enable controls */
    displayInstructions("Place horizontally and click save position...", true);
    displayVisualHelp("plane-horizontal");
    disableAllCalibrations();
    m_ui->boardLevelSavePos->setEnabled(true);
    position = 0;
}

void ConfigRevoWidget::levelCalibrationSavePosition()
{
    QMutexLocker lock(&sensorsUpdateLock);

    Q_UNUSED(lock);

    m_ui->boardLevelSavePos->setEnabled(false);

    rot_accum_pitch.clear();
    rot_accum_roll.clear();

    collectingData = true;

    AttitudeState *attitudeState = AttitudeState::GetInstance(getObjectManager());
    Q_ASSERT(attitudeState);
    connect(attitudeState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(levelCalibrationGetSample(UAVObject *)));

    displayInstructions("Hold...");
}

/**
   Updates the accel bias raw values
 */
void ConfigRevoWidget::levelCalibrationGetSample(UAVObject *obj)
{
    QMutexLocker lock(&sensorsUpdateLock);

    Q_UNUSED(lock);

    switch (obj->getObjID()) {
    case AttitudeState::OBJID:
    {
        AttitudeState *attitudeState = AttitudeState::GetInstance(getObjectManager());
        Q_ASSERT(attitudeState);
        AttitudeState::DataFields attitudeStateData = attitudeState->getData();
        rot_accum_roll.append(attitudeStateData.Roll);
        rot_accum_pitch.append(attitudeStateData.Pitch);
        break;
    }
    default:
        Q_ASSERT(0);
    }

    // Work out the progress based on whichever has less
    double p1 = (double)rot_accum_roll.size() / (double)NOISE_SAMPLES;
    m_ui->boardLevelProgress->setValue(p1 * 100);

    if (rot_accum_roll.size() >= NOISE_SAMPLES &&
        collectingData == true) {
        collectingData = false;

        AttitudeState *attitudeState = AttitudeState::GetInstance(getObjectManager());

        disconnect(attitudeState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(levelCalibrationGetSample(UAVObject *)));

        position++;
        switch (position) {
        case 1:
            rot_data_pitch = OpenPilot::CalibrationUtils::listMean(rot_accum_pitch);
            rot_data_roll  = OpenPilot::CalibrationUtils::listMean(rot_accum_roll);

            displayInstructions("Leave horizontally, rotate 180° along yaw axis and click save position...", true);
            displayVisualHelp("plane-horizontal-rotated");

            disableAllCalibrations();

            m_ui->boardLevelSavePos->setEnabled(true);
            break;
        case 2:
            rot_data_pitch += OpenPilot::CalibrationUtils::listMean(rot_accum_pitch);
            rot_data_pitch /= 2;
            rot_data_roll  += OpenPilot::CalibrationUtils::listMean(rot_accum_roll);
            rot_data_roll  /= 2;
            attitudeState->setMetadata(initialAttitudeStateMdata);
            levelCalibrationCompute();
            enableAllCalibrations();
            break;
        }
    }
}
void ConfigRevoWidget::levelCalibrationCompute()
{
    enableAllCalibrations();

    AttitudeSettings *attitudeSettings = AttitudeSettings::GetInstance(getObjectManager());
    Q_ASSERT(attitudeSettings);
    AttitudeSettings::DataFields attitudeSettingsData = attitudeSettings->getData();

    // Update the biases based on collected data
    // "rotate" the board in the opposite direction as the calculated offset
    attitudeSettingsData.BoardRotation[AttitudeSettings::BOARDROTATION_PITCH] -= rot_data_pitch;
    attitudeSettingsData.BoardRotation[AttitudeSettings::BOARDROTATION_ROLL]  -= rot_data_roll;

    attitudeSettings->setData(attitudeSettingsData);
    attitudeSettings->updated();
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
    m_ui->calibrationVisualHelp->fitInView(m_ui->calibrationVisualHelp->scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);
}

void ConfigRevoWidget::displayInstructions(QString instructions, bool replace)
{
    if (replace || instructions.isNull()) {
        m_ui->calibrationInstructions->clear();
    }
    if (!instructions.isNull()) {
        m_ui->calibrationInstructions->append(instructions);
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
