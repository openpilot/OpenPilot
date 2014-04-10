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

#define GRAVITY 9.81f
#include "assertions.h"
#include "calibration.h"

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
    calibratingMag(false),
    calibratingAccel(false),
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

    // Connect the signals
    // gyro zero calibration
    connect(m_ui->gyroBiasStart, SIGNAL(clicked()), this, SLOT(gyroCalibrationStart()));

    // level calibration
    connect(m_ui->boardLevelStart, SIGNAL(clicked()), this, SLOT(levelCalibrationStart()));
    connect(m_ui->boardLevelSavePos, SIGNAL(clicked()), this, SLOT(levelCalibrationSavePosition()));

    // six point calibrations
    connect(m_ui->sixPointsStartAccel, SIGNAL(clicked()), this, SLOT(sixPointCalibrationAccelStart()));
    connect(m_ui->sixPointsStartMag, SIGNAL(clicked()), this, SLOT(sixPointCalibrationMagStart()));
    connect(m_ui->sixPointsSave, SIGNAL(clicked()), this, SLOT(sixPointCalibrationSavePositionData()));

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

        GyroState *gyroState   = GyroState::GetInstance(getObjectManager());

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
        accelGyroSettingsData.gyro_bias[AccelGyroSettings::GYRO_BIAS_X]   += listMean(gyro_accum_x);
        accelGyroSettingsData.gyro_bias[AccelGyroSettings::GYRO_BIAS_Y]   += listMean(gyro_accum_y);
        accelGyroSettingsData.gyro_bias[AccelGyroSettings::GYRO_BIAS_Z]   += listMean(gyro_accum_z);

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
    rot_data_roll = 0;
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

void ConfigRevoWidget::levelCalibrationSavePosition(){
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
        switch(position){
        case 1:
            rot_data_pitch = listMean(rot_accum_pitch);
            rot_data_roll = listMean(rot_accum_roll);

            displayInstructions("Leave horizontally, rotate 180° along yaw axis and click save position...", true);
            displayVisualHelp("plane-horizontal-rotated");

            disableAllCalibrations();

            m_ui->boardLevelSavePos->setEnabled(true);
            break;
        case 2:
            rot_data_pitch += listMean(rot_accum_pitch);
            rot_data_pitch /= 2;
            rot_data_roll += listMean(rot_accum_roll);
            rot_data_roll /= 2;
            attitudeState->setMetadata(initialAttitudeStateMdata);
            levelCalibrationCompute();
            enableAllCalibrations();
            break;
        }

    }
}
void ConfigRevoWidget::levelCalibrationCompute(){
    enableAllCalibrations();

    AttitudeSettings *attitudeSettings = AttitudeSettings::GetInstance(getObjectManager());
    Q_ASSERT(attitudeSettings);
    AttitudeSettings::DataFields attitudeSettingsData = attitudeSettings->getData();

    // Update the biases based on collected data
    // "rotate" the board in the opposite direction as the calculated offset
    attitudeSettingsData.BoardRotation[AttitudeSettings::BOARDROTATION_PITCH] -= rot_data_pitch;
    attitudeSettingsData.BoardRotation[AttitudeSettings::BOARDROTATION_ROLL] -= rot_data_roll;

    attitudeSettings->setData(attitudeSettingsData);
    attitudeSettings->updated();


}

/********** Six point calibration **************/

void ConfigRevoWidget::sixPointCalibrationMagStart(){
    sixPointCalibrationStart(false, true);
}
void ConfigRevoWidget::sixPointCalibrationAccelStart(){
    sixPointCalibrationStart(true, false);
}

/**
 * Called by the "Start" button.  Sets up the meta data and enables the
 * buttons to perform six point calibration of the magnetometer (optionally
 * accel) to compute the scale and bias of this sensor based on the current
 * home location magnetic strength.
 */
void ConfigRevoWidget::sixPointCalibrationStart(bool calibrateAccel, bool calibrateMag)
{
    calibratingAccel = calibrateAccel;
    calibratingMag = calibrateMag;
    // Store and reset board rotation before calibration starts
    isBoardRotationStored = false;
    storeAndClearBoardRotation();

    RevoCalibration *revoCalibration     = RevoCalibration::GetInstance(getObjectManager());
    HomeLocation *homeLocation = HomeLocation::GetInstance(getObjectManager());
    AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(getObjectManager());

    Q_ASSERT(revoCalibration);
    Q_ASSERT(homeLocation);
    RevoCalibration::DataFields revoCalibrationData     = revoCalibration->getData();
    savedSettings.revoCalibration = revoCalibration->getData();
    HomeLocation::DataFields homeLocationData = homeLocation->getData();
    AccelGyroSettings::DataFields accelGyroSettingsData = accelGyroSettings->getData();
    savedSettings.accelGyroSettings = accelGyroSettings->getData();

    // check if Homelocation is set
    if (!homeLocationData.Set) {
        QMessageBox msgBox;
        msgBox.setInformativeText(tr("<p>HomeLocation not SET.</p><p>Please set your HomeLocation and try again. Aborting calibration!</p>"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        return;
    }

    // Calibration accel
    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X] = 1;
    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y] = 1;
    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z] = 1;
    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X]   = 0;
    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y]   = 0;
    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z]   = 0;

    accel_accum_x.clear();
    accel_accum_y.clear();
    accel_accum_z.clear();

    // Calibration mag
    // Reset the transformation matrix to identity
    for(int i = 0; i < RevoCalibration::MAG_TRANSFORM_R2C2; i++){
        revoCalibrationData.mag_transform[i] = 0;
    }
    revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0] = 1;
    revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1] = 1;
    revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C2] = 1;
    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X]   = 0;
    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y]   = 0;
    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z]   = 0;

    // Disable adaptive mag nulling
    initialMagCorrectionRate = revoCalibrationData.MagBiasNullingRate;
    revoCalibrationData.MagBiasNullingRate = 0;

    revoCalibration->setData(revoCalibrationData);
    accelGyroSettings->setData(accelGyroSettingsData);

    Thread::usleep(100000);

    gyro_accum_x.clear();
    gyro_accum_y.clear();
    gyro_accum_z.clear();
    mag_accum_x.clear();
    mag_accum_y.clear();
    mag_accum_z.clear();

    UAVObject::Metadata mdata;

    /* Need to get as many accel updates as possible */
    AccelState *accelState = AccelState::GetInstance(getObjectManager());
    Q_ASSERT(accelState);

    initialAccelStateMdata = accelState->getMetadata();
    mdata = initialAccelStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    accelState->setMetadata(mdata);

    /* Need to get as many mag updates as possible */
    MagState *mag = MagState::GetInstance(getObjectManager());
    Q_ASSERT(mag);
    initialMagStateMdata = mag->getMetadata();
    mdata = initialMagStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    mag->setMetadata(mdata);

    /* Show instructions and enable controls */
    displayInstructions("Place horizontally and click save position...", true);
    displayVisualHelp("plane-horizontal");
    disableAllCalibrations();
    m_ui->sixPointsSave->setEnabled(true);
    position = 0;
}

/**
 * Saves the data from the aircraft in one of six positions.
 * This is called when they click "save position" and starts
 * averaging data for this position.
 */
void ConfigRevoWidget::sixPointCalibrationSavePositionData()
{
    QMutexLocker lock(&sensorsUpdateLock);

    m_ui->sixPointsSave->setEnabled(false);

    accel_accum_x.clear();
    accel_accum_y.clear();
    accel_accum_z.clear();
    mag_accum_x.clear();
    mag_accum_y.clear();
    mag_accum_z.clear();

    collectingData = true;

    AccelState *accelState = AccelState::GetInstance(getObjectManager());
    Q_ASSERT(accelState);
    MagState *mag = MagState::GetInstance(getObjectManager());
    Q_ASSERT(mag);

    connect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(sixPointCalibrationGetSample(UAVObject *)));
    connect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(sixPointCalibrationGetSample(UAVObject *)));

    displayInstructions("Hold...");
}

/**
 * Grab a sample of mag (optionally accel) data while in this position and
 * store it for averaging.  When sufficient points are collected advance
 * to the next position (give message to user) or compute the scale and bias
 */
void ConfigRevoWidget::sixPointCalibrationGetSample(UAVObject *obj)
{
    QMutexLocker lock(&sensorsUpdateLock);

    // This is necessary to prevent a race condition on disconnect signal and another update
    if (collectingData == true) {
        if (obj->getObjID() == AccelState::OBJID) {
            AccelState *accelState = AccelState::GetInstance(getObjectManager());
            Q_ASSERT(accelState);
            AccelState::DataFields accelStateData = accelState->getData();
            accel_accum_x.append(accelStateData.x);
            accel_accum_y.append(accelStateData.y);
            accel_accum_z.append(accelStateData.z);

        } else if (obj->getObjID() == MagState::OBJID) {
            MagState *mag = MagState::GetInstance(getObjectManager());
            Q_ASSERT(mag);
            MagState::DataFields magData = mag->getData();
            mag_accum_x.append(magData.x);
            mag_accum_y.append(magData.y);
            mag_accum_z.append(magData.z);
        } else {
            Q_ASSERT(0);
        }
    }

    if (accel_accum_x.size() >= SAMPLE_SIZE && mag_accum_x.size() >= SAMPLE_SIZE && collectingData == true) {

        collectingData = false;

        m_ui->sixPointsSave->setEnabled(true);

        // Store the mean for this position for the accel
        AccelState *accelState = AccelState::GetInstance(getObjectManager());
        Q_ASSERT(accelState);
        disconnect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(sixPointCalibrationGetSample(UAVObject *)));
        accel_data_x[position] = listMean(accel_accum_x);
        accel_data_y[position] = listMean(accel_accum_y);
        accel_data_z[position] = listMean(accel_accum_z);

        // Store the mean for this position for the mag
        MagState *mag = MagState::GetInstance(getObjectManager());
        Q_ASSERT(mag);
        disconnect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(sixPointCalibrationGetSample(UAVObject *)));
        mag_data_x[position] = listMean(mag_accum_x);
        mag_data_y[position] = listMean(mag_accum_y);
        mag_data_z[position] = listMean(mag_accum_z);

        position = (position + 1) % 6;
        if (position == 1) {
            displayInstructions("Place with left side down and click save position...");
            displayVisualHelp("plane-left");
        }
        if (position == 2) {
            displayInstructions("Place upside down and click save position...");
            displayVisualHelp("plane-flip");
        }
        if (position == 3) {
            displayInstructions("Place with right side down and click save position...");
            displayVisualHelp("plane-right");
        }
        if (position == 4) {
            displayInstructions("Place with nose up and click save position...");
            displayVisualHelp("plane-up");
        }
        if (position == 5) {
            displayInstructions("Place with nose down and click save position...");
            displayVisualHelp("plane-down");
        }
        if (position == 0) {
            sixPointCalibrationCompute(calibratingMag, calibratingAccel);
            m_ui->sixPointsSave->setEnabled(false);

            enableAllCalibrations();

            /* Cleanup original settings */
            accelState->setMetadata(initialAccelStateMdata);

            mag->setMetadata(initialMagStateMdata);

            // Recall saved board rotation
            recallBoardRotation();
        }
    }
}

/**
 * Computes the scale and bias for the magnetomer and (compile option)
 * for the accel once all the data has been collected in 6 positions.
 */
void ConfigRevoWidget::sixPointCalibrationCompute(bool mag, bool accel)
{
    double S[3], b[3];
    double Be_length;
    AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(getObjectManager());
    RevoCalibration *revoCalibration     = RevoCalibration::GetInstance(getObjectManager());
    HomeLocation *homeLocation = HomeLocation::GetInstance(getObjectManager());

    Q_ASSERT(revoCalibration);
    Q_ASSERT(homeLocation);
    AccelGyroSettings::DataFields accelGyroSettingsData = accelGyroSettings->getData();
    RevoCalibration::DataFields revoCalibrationData     = revoCalibration->getData();
    HomeLocation::DataFields homeLocationData = homeLocation->getData();

    // Calibration accel
    if(accel) {
        OpenPilot::CalibrationUtils::SixPointInConstFieldCal(homeLocationData.g_e, accel_data_x, accel_data_y, accel_data_z, S, b);
        accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X] = fabs(S[0]);
        accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y] = fabs(S[1]);
        accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z] = fabs(S[2]);

        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X]   = -sign(S[0]) * b[0];
        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y]   = -sign(S[1]) * b[1];
        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z]   = -sign(S[2]) * b[2];
    }

    // Calibration mag
    if(mag){
        Be_length = sqrt(pow(homeLocationData.Be[0], 2) + pow(homeLocationData.Be[1], 2) + pow(homeLocationData.Be[2], 2));
        OpenPilot::CalibrationUtils::SixPointInConstFieldCal(Be_length, mag_data_x, mag_data_y, mag_data_z, S, b);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0] = fabs(S[0]);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1] = fabs(S[1]);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C2] = fabs(S[2]);

        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X]   = -sign(S[0]) * b[0];
        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y]   = -sign(S[1]) * b[1];
        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z]   = -sign(S[2]) * b[2];
    }
    // Restore the previous setting
    revoCalibrationData.MagBiasNullingRate = initialMagCorrectionRate;


    bool good_calibration = true;

    // Check the mag calibration is good
    if(mag){
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0];
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1];
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C2] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C2];
        good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X] ==
                            revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X];
        good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y] ==
                            revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y];
        good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z] ==
                            revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z];
    }
    // Check the accel calibration is good
    if(accel){
        good_calibration &= accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X] ==
                            accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X];
        good_calibration &= accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y] ==
                            accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y];
        good_calibration &= accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z] ==
                            accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z];
        good_calibration &= accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X] ==
                            accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X];
        good_calibration &= accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y] ==
                            accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y];
        good_calibration &= accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z] ==
                            accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z];
    }
    if (good_calibration) {
        if(mag){
            revoCalibration->setData(revoCalibrationData);
        } else {
            revoCalibration->setData(savedSettings.revoCalibration);
        }

        if(accel){
            accelGyroSettings->setData(accelGyroSettingsData);
        } else {
            accelGyroSettings->setData(savedSettings.accelGyroSettings);
        }
        displayInstructions("Computed sensor scale and bias...", true);
    } else {
        displayInstructions("Bad calibration. Please repeat.", true);
    }
    position = -1; // set to run again
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

void ConfigRevoWidget::displayInstructions(QString instructions, bool replace){
    if(replace || instructions.isNull()){
        m_ui->calibrationInstructions->clear();
    }
    if(!instructions.isNull()){
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
