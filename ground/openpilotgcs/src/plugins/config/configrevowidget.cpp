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
#define SIX_POINT_CAL_ACCEL

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
    m_ui->sixPointsHelp->setScene(new QGraphicsScene(this));

    paperplane = new QGraphicsSvgItem();
    paperplane->setSharedRenderer(new QSvgRenderer());
    paperplane->renderer()->load(QString(":/configgadget/images/paper-plane.svg"));
    paperplane->setElementId("plane-horizontal");
    m_ui->sixPointsHelp->scene()->addItem(paperplane);
    m_ui->sixPointsHelp->setSceneRect(paperplane->boundingRect());

    // Initialization of the Revo sensor noise bargraph graph
    m_ui->sensorsBargraph->setScene(new QGraphicsScene(this));

    QSvgRenderer *renderer = new QSvgRenderer();
    sensorsBargraph = new QGraphicsSvgItem();
    renderer->load(QString(":/configgadget/images/ahrs-calib.svg"));
    sensorsBargraph->setSharedRenderer(renderer);
    sensorsBargraph->setElementId("background");
    sensorsBargraph->setObjectName("background");
    m_ui->sensorsBargraph->scene()->addItem(sensorsBargraph);
    m_ui->sensorsBargraph->setSceneRect(sensorsBargraph->boundingRect());

    // Initialize the 9 bargraph values:

    QMatrix lineMatrix = renderer->matrixForElement("accel_x");
    QRectF rect  = lineMatrix.mapRect(renderer->boundsOnElement("accel_x"));
    qreal startX = rect.x();
    qreal startY = rect.y() + rect.height();
    // maxBarHeight will be used for scaling it later.
    maxBarHeight = rect.height();
    // Then once we have the initial location, we can put it
    // into a QGraphicsSvgItem which we will display at the same
    // place: we do this so that the heading scale can be clipped to
    // the compass dial region.
    accel_x = new QGraphicsSvgItem();
    accel_x->setSharedRenderer(renderer);
    accel_x->setElementId("accel_x");
    m_ui->sensorsBargraph->scene()->addItem(accel_x);
    accel_x->setPos(startX, startY);
    accel_x->setTransform(QTransform::fromScale(1, 0), true);

    lineMatrix = renderer->matrixForElement("accel_y");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("accel_y"));
    startX     = rect.x();
    startY     = rect.y() + rect.height();
    accel_y    = new QGraphicsSvgItem();
    accel_y->setSharedRenderer(renderer);
    accel_y->setElementId("accel_y");
    m_ui->sensorsBargraph->scene()->addItem(accel_y);
    accel_y->setPos(startX, startY);
    accel_y->setTransform(QTransform::fromScale(1, 0), true);

    lineMatrix = renderer->matrixForElement("accel_z");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("accel_z"));
    startX     = rect.x();
    startY     = rect.y() + rect.height();
    accel_z    = new QGraphicsSvgItem();
    accel_z->setSharedRenderer(renderer);
    accel_z->setElementId("accel_z");
    m_ui->sensorsBargraph->scene()->addItem(accel_z);
    accel_z->setPos(startX, startY);
    accel_z->setTransform(QTransform::fromScale(1, 0), true);

    lineMatrix = renderer->matrixForElement("gyro_x");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("gyro_x"));
    startX     = rect.x();
    startY     = rect.y() + rect.height();
    gyro_x     = new QGraphicsSvgItem();
    gyro_x->setSharedRenderer(renderer);
    gyro_x->setElementId("gyro_x");
    m_ui->sensorsBargraph->scene()->addItem(gyro_x);
    gyro_x->setPos(startX, startY);
    gyro_x->setTransform(QTransform::fromScale(1, 0), true);

    lineMatrix = renderer->matrixForElement("gyro_y");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("gyro_y"));
    startX     = rect.x();
    startY     = rect.y() + rect.height();
    gyro_y     = new QGraphicsSvgItem();
    gyro_y->setSharedRenderer(renderer);
    gyro_y->setElementId("gyro_y");
    m_ui->sensorsBargraph->scene()->addItem(gyro_y);
    gyro_y->setPos(startX, startY);
    gyro_y->setTransform(QTransform::fromScale(1, 0), true);


    lineMatrix = renderer->matrixForElement("gyro_z");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("gyro_z"));
    startX     = rect.x();
    startY     = rect.y() + rect.height();
    gyro_z     = new QGraphicsSvgItem();
    gyro_z->setSharedRenderer(renderer);
    gyro_z->setElementId("gyro_z");
    m_ui->sensorsBargraph->scene()->addItem(gyro_z);
    gyro_z->setPos(startX, startY);
    gyro_z->setTransform(QTransform::fromScale(1, 0), true);

    lineMatrix = renderer->matrixForElement("mag_x");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("mag_x"));
    startX     = rect.x();
    startY     = rect.y() + rect.height();
    mag_x = new QGraphicsSvgItem();
    mag_x->setSharedRenderer(renderer);
    mag_x->setElementId("mag_x");
    m_ui->sensorsBargraph->scene()->addItem(mag_x);
    mag_x->setPos(startX, startY);
    mag_x->setTransform(QTransform::fromScale(1, 0), true);

    lineMatrix = renderer->matrixForElement("mag_y");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("mag_y"));
    startX     = rect.x();
    startY     = rect.y() + rect.height();
    mag_y = new QGraphicsSvgItem();
    mag_y->setSharedRenderer(renderer);
    mag_y->setElementId("mag_y");
    m_ui->sensorsBargraph->scene()->addItem(mag_y);
    mag_y->setPos(startX, startY);
    mag_y->setTransform(QTransform::fromScale(1, 0), true);

    lineMatrix = renderer->matrixForElement("mag_z");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("mag_z"));
    startX     = rect.x();
    startY     = rect.y() + rect.height();
    mag_z = new QGraphicsSvgItem();
    mag_z->setSharedRenderer(renderer);
    mag_z->setElementId("mag_z");
    m_ui->sensorsBargraph->scene()->addItem(mag_z);
    mag_z->setPos(startX, startY);
    mag_z->setTransform(QTransform::fromScale(1, 0), true);

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
    m_thermalCalibrationModel = new ThermalCalibrationModel(this);

    connect(m_ui->ThermalBiasStart, SIGNAL(clicked()),m_thermalCalibrationModel, SLOT(btnStart()));
    connect(m_ui->ThermalBiasEnd, SIGNAL(clicked()),m_thermalCalibrationModel, SLOT(btnEnd()));
    connect(m_ui->ThermalBiasCancel, SIGNAL(clicked()),m_thermalCalibrationModel, SLOT(btnAbort()));

    connect(m_thermalCalibrationModel, SIGNAL(startEnabledChanged(bool)),m_ui->ThermalBiasStart, SLOT(setEnabled(bool)));
    connect(m_thermalCalibrationModel, SIGNAL(endEnabledChanged(bool)),m_ui->ThermalBiasEnd, SLOT(setEnabled(bool)));
    connect(m_thermalCalibrationModel, SIGNAL(cancelEnabledChanged(bool)),m_ui->ThermalBiasCancel, SLOT(setEnabled(bool)));

    connect(m_thermalCalibrationModel, SIGNAL(temperatureChanged(QString)),m_ui->label_thermalGradient, SLOT(setText(QString)));
    connect(m_thermalCalibrationModel, SIGNAL(temperatureGradientChanged(QString)),m_ui->label_thermalGradient1, SLOT(setText(QString)));
    m_thermalCalibrationModel->init();


    // Connect the signals
    connect(m_ui->accelBiasStart, SIGNAL(clicked()), this, SLOT(doStartAccelGyroBiasCalibration()));
    connect(m_ui->sixPointsStart, SIGNAL(clicked()), this, SLOT(doStartSixPointCalibration()));
    connect(m_ui->sixPointsSave, SIGNAL(clicked()), this, SLOT(savePositionData()));
    connect(m_ui->noiseMeasurementStart, SIGNAL(clicked()), this, SLOT(doStartNoiseMeasurement()));

    connect(m_ui->hlClearButton, SIGNAL(clicked()), this, SLOT(clearHomeLocation()));

    addUAVObjectToWidgetRelation("RevoSettings", "FusionAlgorithm", m_ui->FusionAlgorithm);

    addUAVObjectToWidgetRelation("AttitudeSettings", "BoardRotation", m_ui->rollRotation, AttitudeSettings::BOARDROTATION_ROLL);
    addUAVObjectToWidgetRelation("AttitudeSettings", "BoardRotation", m_ui->pitchRotation, AttitudeSettings::BOARDROTATION_PITCH);
    addUAVObjectToWidgetRelation("AttitudeSettings", "BoardRotation", m_ui->yawRotation, AttitudeSettings::BOARDROTATION_YAW);
    addUAVObjectToWidgetRelation("AttitudeSettings", "AccelTau", m_ui->accelTau);

    populateWidgets();
    refreshWidgetsValues();
    m_ui->tabWidget->setCurrentIndex(0);
}

ConfigRevoWidget::~ConfigRevoWidget()
{
    // Do nothing
}


void ConfigRevoWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a sensorsBargraph that is way too small.
    m_ui->sensorsBargraph->fitInView(sensorsBargraph, Qt::KeepAspectRatio);
    m_ui->sixPointsHelp->fitInView(paperplane, Qt::KeepAspectRatio);
}

void ConfigRevoWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    m_ui->sensorsBargraph->fitInView(sensorsBargraph, Qt::KeepAspectRatio);
    m_ui->sixPointsHelp->fitInView(paperplane, Qt::KeepAspectRatio);
}

/**
 * Starts an accelerometer bias calibration.
 */
void ConfigRevoWidget::doStartAccelGyroBiasCalibration()
{
    // Store and reset board rotation before calibration starts
    isBoardRotationStored = false;
    storeAndClearBoardRotation();

    m_ui->accelBiasStart->setEnabled(false);
    m_ui->accelBiasProgress->setValue(0);

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

    accel_accum_x.clear();
    accel_accum_y.clear();
    accel_accum_z.clear();
    gyro_accum_x.clear();
    gyro_accum_y.clear();
    gyro_accum_z.clear();

    UAVObject::Metadata mdata;

    /* Need to get as many accel updates as possible */
    AccelState *accelState = AccelState::GetInstance(getObjectManager());
    Q_ASSERT(accelState);
    initialAccelStateMdata = accelState->getMetadata();
    mdata = initialAccelStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    accelState->setMetadata(mdata);

    GyroState *gyroState = GyroState::GetInstance(getObjectManager());
    Q_ASSERT(gyroState);
    initialGyroStateMdata = gyroState->getMetadata();
    mdata = initialGyroStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    gyroState->setMetadata(mdata);

    // Now connect to the accels and mag updates, gather for 100 samples
    collectingData = true;
    connect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetAccelGyroBiasData(UAVObject *)));
    connect(gyroState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetAccelGyroBiasData(UAVObject *)));
}

/**
   Updates the accel bias raw values
 */
void ConfigRevoWidget::doGetAccelGyroBiasData(UAVObject *obj)
{
    QMutexLocker lock(&sensorsUpdateLock);

    Q_UNUSED(lock);

    switch (obj->getObjID()) {
    case AccelState::OBJID:
    {
        AccelState *accelState = AccelState::GetInstance(getObjectManager());
        Q_ASSERT(accelState);
        AccelState::DataFields accelStateData = accelState->getData();

        accel_accum_x.append(accelStateData.x);
        accel_accum_y.append(accelStateData.y);
        accel_accum_z.append(accelStateData.z);
        break;
    }
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
    double p1 = (double)accel_accum_x.size() / (double)NOISE_SAMPLES;
    double p2 = (double)accel_accum_y.size() / (double)NOISE_SAMPLES;
    m_ui->accelBiasProgress->setValue(((p1 < p2) ? p1 : p2) * 100);

    if (accel_accum_x.size() >= NOISE_SAMPLES &&
        gyro_accum_y.size() >= NOISE_SAMPLES &&
        collectingData == true) {
        collectingData = false;

        AccelState *accelState = AccelState::GetInstance(getObjectManager());
        GyroState *gyroState   = GyroState::GetInstance(getObjectManager());

        disconnect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetAccelGyroBiasData(UAVObject *)));
        disconnect(gyroState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetAccelGyroBiasData(UAVObject *)));

        m_ui->accelBiasStart->setEnabled(true);

        RevoCalibration *revoCalibration     = RevoCalibration::GetInstance(getObjectManager());
        Q_ASSERT(revoCalibration);
        AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(getObjectManager());
        Q_ASSERT(accelGyroSettings);


        RevoCalibration::DataFields revoCalibrationData     = revoCalibration->getData();
        AccelGyroSettings::DataFields accelGyroSettingsData = accelGyroSettings->getData();

        revoCalibrationData.BiasCorrectedRaw = RevoCalibration::BIASCORRECTEDRAW_TRUE;

        // Update the biases based on collected data
        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X] += listMean(accel_accum_x);
        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y] += listMean(accel_accum_y);
        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z] += (listMean(accel_accum_z) + GRAVITY);
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

        accelState->setMetadata(initialAccelStateMdata);
        gyroState->setMetadata(initialGyroStateMdata);

        // Recall saved board rotation
        recallBoardRotation();
    }
}


int LinearEquationsSolving(int nDim, double *pfMatr, double *pfVect, double *pfSolution)
{
    double fMaxElem;
    double fAcc;

    int i, j, k, m;

    for (k = 0; k < (nDim - 1); k++) { // base row of matrix
        // search of line with max element
        fMaxElem = fabs(pfMatr[k * nDim + k]);
        m = k;
        for (i = k + 1; i < nDim; i++) {
            if (fMaxElem < fabs(pfMatr[i * nDim + k])) {
                fMaxElem = pfMatr[i * nDim + k];
                m = i;
            }
        }

        // permutation of base line (index k) and max element line(index m)
        if (m != k) {
            for (i = k; i < nDim; i++) {
                fAcc = pfMatr[k * nDim + i];
                pfMatr[k * nDim + i] = pfMatr[m * nDim + i];
                pfMatr[m * nDim + i] = fAcc;
            }
            fAcc = pfVect[k];
            pfVect[k] = pfVect[m];
            pfVect[m] = fAcc;
        }

        if (pfMatr[k * nDim + k] == 0.) {
            return 0; // needs improvement !!!
        }
        // triangulation of matrix with coefficients
        for (j = (k + 1); j < nDim; j++) { // current row of matrix
            fAcc = -pfMatr[j * nDim + k] / pfMatr[k * nDim + k];
            for (i = k; i < nDim; i++) {
                pfMatr[j * nDim + i] = pfMatr[j * nDim + i] + fAcc * pfMatr[k * nDim + i];
            }
            pfVect[j] = pfVect[j] + fAcc * pfVect[k]; // free member recalculation
        }
    }

    for (k = (nDim - 1); k >= 0; k--) {
        pfSolution[k] = pfVect[k];
        for (i = (k + 1); i < nDim; i++) {
            pfSolution[k] -= (pfMatr[k * nDim + i] * pfSolution[i]);
        }
        pfSolution[k] = pfSolution[k] / pfMatr[k * nDim + k];
    }

    return 1;
}


int SixPointInConstFieldCal(double ConstMag, double x[6], double y[6], double z[6], double S[3], double b[3])
{
    int i;
    double A[5][5];
    double f[5], c[5];
    double xp, yp, zp, Sx;

    // Fill in matrix A -
    // write six difference-in-magnitude equations of the form
    // Sx^2(x2^2-x1^2) + 2*Sx*bx*(x2-x1) + Sy^2(y2^2-y1^2) + 2*Sy*by*(y2-y1) + Sz^2(z2^2-z1^2) + 2*Sz*bz*(z2-z1) = 0
    // or in other words
    // 2*Sx*bx*(x2-x1)/Sx^2  + Sy^2(y2^2-y1^2)/Sx^2  + 2*Sy*by*(y2-y1)/Sx^2  + Sz^2(z2^2-z1^2)/Sx^2  + 2*Sz*bz*(z2-z1)/Sx^2  = (x1^2-x2^2)
    for (i = 0; i < 5; i++) {
        A[i][0] = 2.0 * (x[i + 1] - x[i]);
        A[i][1] = y[i + 1] * y[i + 1] - y[i] * y[i];
        A[i][2] = 2.0 * (y[i + 1] - y[i]);
        A[i][3] = z[i + 1] * z[i + 1] - z[i] * z[i];
        A[i][4] = 2.0 * (z[i + 1] - z[i]);
        f[i]    = x[i] * x[i] - x[i + 1] * x[i + 1];
    }

    // solve for c0=bx/Sx, c1=Sy^2/Sx^2; c2=Sy*by/Sx^2, c3=Sz^2/Sx^2, c4=Sz*bz/Sx^2
    if (!LinearEquationsSolving(5, (double *)A, f, c)) {
        return 0;
    }

    // use one magnitude equation and c's to find Sx - doesn't matter which - all give the same answer
    xp   = x[0]; yp = y[0]; zp = z[0];
    Sx   = sqrt(ConstMag * ConstMag / (xp * xp + 2 * c[0] * xp + c[0] * c[0] + c[1] * yp * yp + 2 * c[2] * yp + c[2] * c[2] / c[1] + c[3] * zp * zp + 2 * c[4] * zp + c[4] * c[4] / c[3]));

    S[0] = Sx;
    b[0] = Sx * c[0];
    S[1] = sqrt(c[1] * Sx * Sx);
    b[1] = c[2] * Sx * Sx / S[1];
    S[2] = sqrt(c[3] * Sx * Sx);
    b[2] = c[4] * Sx * Sx / S[2];

    return 1;
}

/********** Functions for six point calibration **************/

/**
 * Called by the "Start" button.  Sets up the meta data and enables the
 * buttons to perform six point calibration of the magnetometer (optionally
 * accel) to compute the scale and bias of this sensor based on the current
 * home location magnetic strength.
 */
void ConfigRevoWidget::doStartSixPointCalibration()
{
    // Store and reset board rotation before calibration starts
    isBoardRotationStored = false;
    storeAndClearBoardRotation();

    RevoCalibration *revoCalibration     = RevoCalibration::GetInstance(getObjectManager());
    HomeLocation *homeLocation = HomeLocation::GetInstance(getObjectManager());
    AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(getObjectManager());

    Q_ASSERT(revoCalibration);
    Q_ASSERT(homeLocation);
    RevoCalibration::DataFields revoCalibrationData     = revoCalibration->getData();
    HomeLocation::DataFields homeLocationData = homeLocation->getData();
    AccelGyroSettings::DataFields accelGyroSettingsData = accelGyroSettings->getData();

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

#ifdef SIX_POINT_CAL_ACCEL
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
#endif

    // Calibration mag
    revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_X] = 1;
    revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Y] = 1;
    revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Z] = 1;
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

#ifdef SIX_POINT_CAL_ACCEL
    /* Need to get as many accel updates as possible */
    AccelState *accelState = AccelState::GetInstance(getObjectManager());
    Q_ASSERT(accelState);

    initialAccelStateMdata = accelState->getMetadata();
    mdata = initialAccelStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    accelState->setMetadata(mdata);
#endif

    /* Need to get as many mag updates as possible */
    MagState *mag = MagState::GetInstance(getObjectManager());
    Q_ASSERT(mag);
    initialMagStateMdata = mag->getMetadata();
    mdata = initialMagStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    mag->setMetadata(mdata);

    /* Show instructions and enable controls */
    m_ui->sixPointCalibInstructions->clear();
    m_ui->sixPointCalibInstructions->append("Place horizontally and click save position...");
    displayPlane("plane-horizontal");
    m_ui->sixPointsStart->setEnabled(false);
    m_ui->sixPointsSave->setEnabled(true);
    position = 0;
}

/**
 * Saves the data from the aircraft in one of six positions.
 * This is called when they click "save position" and starts
 * averaging data for this position.
 */
void ConfigRevoWidget::savePositionData()
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

    connect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetSixPointCalibrationMeasurement(UAVObject *)));
    connect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetSixPointCalibrationMeasurement(UAVObject *)));

    m_ui->sixPointCalibInstructions->append("Hold...");
}

/**
 * Grab a sample of mag (optionally accel) data while in this position and
 * store it for averaging.  When sufficient points are collected advance
 * to the next position (give message to user) or compute the scale and bias
 */
void ConfigRevoWidget::doGetSixPointCalibrationMeasurement(UAVObject *obj)
{
    QMutexLocker lock(&sensorsUpdateLock);

    // This is necessary to prevent a race condition on disconnect signal and another update
    if (collectingData == true) {
        if (obj->getObjID() == AccelState::OBJID) {
#ifdef SIX_POINT_CAL_ACCEL
            AccelState *accelState = AccelState::GetInstance(getObjectManager());
            Q_ASSERT(accelState);
            AccelState::DataFields accelStateData = accelState->getData();
            accel_accum_x.append(accelStateData.x);
            accel_accum_y.append(accelStateData.y);
            accel_accum_z.append(accelStateData.z);
#endif
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

#ifdef SIX_POINT_CAL_ACCEL
    if (accel_accum_x.size() >= 20 && mag_accum_x.size() >= 20 && collectingData == true) {
#else
    if (mag_accum_x.size() >= 20 && collectingData == true) {
#endif
        collectingData = false;

        m_ui->sixPointsSave->setEnabled(true);

#ifdef SIX_POINT_CAL_ACCEL
        // Store the mean for this position for the accel
        AccelState *accelState = AccelState::GetInstance(getObjectManager());
        Q_ASSERT(accelState);
        disconnect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetSixPointCalibrationMeasurement(UAVObject *)));
        accel_data_x[position] = listMean(accel_accum_x);
        accel_data_y[position] = listMean(accel_accum_y);
        accel_data_z[position] = listMean(accel_accum_z);
#endif

        // Store the mean for this position for the mag
        MagState *mag = MagState::GetInstance(getObjectManager());
        Q_ASSERT(mag);
        disconnect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetSixPointCalibrationMeasurement(UAVObject *)));
        mag_data_x[position] = listMean(mag_accum_x);
        mag_data_y[position] = listMean(mag_accum_y);
        mag_data_z[position] = listMean(mag_accum_z);

        position = (position + 1) % 6;
        if (position == 1) {
            m_ui->sixPointCalibInstructions->append("Place with left side down and click save position...");
            displayPlane("plane-left");
        }
        if (position == 2) {
            m_ui->sixPointCalibInstructions->append("Place upside down and click save position...");
            displayPlane("plane-flip");
        }
        if (position == 3) {
            m_ui->sixPointCalibInstructions->append("Place with right side down and click save position...");
            displayPlane("plane-right");
        }
        if (position == 4) {
            m_ui->sixPointCalibInstructions->append("Place with nose up and click save position...");
            displayPlane("plane-up");
        }
        if (position == 5) {
            m_ui->sixPointCalibInstructions->append("Place with nose down and click save position...");
            displayPlane("plane-down");
        }
        if (position == 0) {
            computeScaleBias();
            m_ui->sixPointsStart->setEnabled(true);
            m_ui->sixPointsSave->setEnabled(false);

            /* Cleanup original settings */
#ifdef SIX_POINT_CAL_ACCEL
            accelState->setMetadata(initialAccelStateMdata);
#endif
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
void ConfigRevoWidget::computeScaleBias()
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

#ifdef SIX_POINT_CAL_ACCEL
    // Calibration accel
    SixPointInConstFieldCal(homeLocationData.g_e, accel_data_x, accel_data_y, accel_data_z, S, b);
    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X] = fabs(S[0]);
    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y] = fabs(S[1]);
    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z] = fabs(S[2]);

    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X]   = -sign(S[0]) * b[0];
    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y]   = -sign(S[1]) * b[1];
    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z]   = -sign(S[2]) * b[2];
#endif

    // Calibration mag
    Be_length = sqrt(pow(homeLocationData.Be[0], 2) + pow(homeLocationData.Be[1], 2) + pow(homeLocationData.Be[2], 2));
    SixPointInConstFieldCal(Be_length, mag_data_x, mag_data_y, mag_data_z, S, b);
    revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_X] = fabs(S[0]);
    revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Y] = fabs(S[1]);
    revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Z] = fabs(S[2]);

    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X]   = -sign(S[0]) * b[0];
    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y]   = -sign(S[1]) * b[1];
    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z]   = -sign(S[2]) * b[2];

    // Restore the previous setting
    revoCalibrationData.MagBiasNullingRate = initialMagCorrectionRate;

#ifdef SIX_POINT_CAL_ACCEL
    bool good_calibration = true;

    // Check the mag calibration is good
    good_calibration &= revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_X] ==
                        revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_X];
    good_calibration &= revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Y] ==
                        revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Y];
    good_calibration &= revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Z] ==
                        revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Z];
    good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X] ==
                        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X];
    good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y] ==
                        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y];
    good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z] ==
                        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z];

    // Check the accel calibration is good
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
    if (good_calibration) {
        revoCalibration->setData(revoCalibrationData);
        accelGyroSettings->setData(accelGyroSettingsData);
        m_ui->sixPointCalibInstructions->append("Computed accel and mag scale and bias...");
    } else {
        revoCalibrationData   = revoCalibration->getData();
        accelGyroSettingsData = accelGyroSettings->getData();
        m_ui->sixPointCalibInstructions->append("Bad calibration. Please repeat.");
    }
#else // ifdef SIX_POINT_CAL_ACCEL
    bool good_calibration = true;
    good_calibration &= revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_X] ==
                        revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_X];
    good_calibration &= revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Y] ==
                        revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Y];
    good_calibration &= revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Z] ==
                        revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Z];
    good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X] ==
                        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X];
    good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y] ==
                        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y];
    good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z] ==
                        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z];
    if (good_calibration) {
        revoCalibration->setData(revoCalibrationData);
        accelGyroSettings->setData(accelGyroSettingsData);
        m_ui->sixPointCalibInstructions->append("Computed mag scale and bias...");
    } else {
        revoCalibrationData   = revoCalibration->getData();
        accelGyroSettingsData = accelGyroSettings->getData();
        m_ui->sixPointCalibInstructions->append("Bad calibration. Please repeat.");
    }
#endif // ifdef SIX_POINT_CAL_ACCEL

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
   Rotate the paper plane
 */
void ConfigRevoWidget::displayPlane(QString elementID)
{
    paperplane->setElementId(elementID);
    m_ui->sixPointsHelp->setSceneRect(paperplane->boundingRect());
    m_ui->sixPointsHelp->fitInView(paperplane, Qt::KeepAspectRatio);
}

/*********** Noise measurement functions **************/
/**
 * Connect sensor updates and timeout for measuring the noise
 */
void ConfigRevoWidget::doStartNoiseMeasurement()
{
    QMutexLocker lock(&sensorsUpdateLock);

    // Store and reset board rotation before calibration starts
    isBoardRotationStored = false;
    storeAndClearBoardRotation();

    Q_UNUSED(lock);

    RevoCalibration *revoCalibration = RevoCalibration::GetInstance(getObjectManager());
    HomeLocation *homeLocation = HomeLocation::GetInstance(getObjectManager());
    Q_ASSERT(revoCalibration);
    Q_ASSERT(homeLocation);
    RevoCalibration::DataFields revoCalibrationData = revoCalibration->getData();
    HomeLocation::DataFields homeLocationData = homeLocation->getData();

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

    accel_accum_x.clear();
    accel_accum_y.clear();
    accel_accum_z.clear();
    gyro_accum_x.clear();
    gyro_accum_y.clear();
    gyro_accum_z.clear();
    mag_accum_x.clear();
    mag_accum_y.clear();
    mag_accum_z.clear();

    /* Need to get as many accel, mag and gyro updates as possible */
    AccelState *accelState = AccelState::GetInstance(getObjectManager());
    Q_ASSERT(accelState);
    GyroState *gyroState   = GyroState::GetInstance(getObjectManager());
    Q_ASSERT(gyroState);
    MagState *mag = MagState::GetInstance(getObjectManager());
    Q_ASSERT(mag);

    UAVObject::Metadata mdata;

    initialAccelStateMdata = accelState->getMetadata();
    mdata = initialAccelStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    accelState->setMetadata(mdata);

    initialGyroStateMdata = gyroState->getMetadata();
    mdata = initialGyroStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    gyroState->setMetadata(mdata);

    initialMagStateMdata = mag->getMetadata();
    mdata = initialMagStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    mag->setMetadata(mdata);

    /* Connect for updates */
    connect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetNoiseSample(UAVObject *)));
    connect(gyroState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetNoiseSample(UAVObject *)));
    connect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetNoiseSample(UAVObject *)));
}

/**
 * Called when any of the sensors are updated.  Stores the sample for measuring the
 * variance at the end
 */
void ConfigRevoWidget::doGetNoiseSample(UAVObject *obj)
{
    QMutexLocker lock(&sensorsUpdateLock);

    Q_UNUSED(lock);

    Q_ASSERT(obj);

    switch (obj->getObjID()) {
    case GyroState::OBJID:
    {
        GyroState *gyroState = GyroState::GetInstance(getObjectManager());
        Q_ASSERT(gyroState);
        GyroState::DataFields gyroData = gyroState->getData();
        gyro_accum_x.append(gyroData.x);
        gyro_accum_y.append(gyroData.y);
        gyro_accum_z.append(gyroData.z);
        break;
    }
    case AccelState::OBJID:
    {
        AccelState *accelState = AccelState::GetInstance(getObjectManager());
        Q_ASSERT(accelState);
        AccelState::DataFields accelStateData = accelState->getData();
        accel_accum_x.append(accelStateData.x);
        accel_accum_y.append(accelStateData.y);
        accel_accum_z.append(accelStateData.z);
        break;
    }
    case MagState::OBJID:
    {
        MagState *mags = MagState::GetInstance(getObjectManager());
        Q_ASSERT(mags);
        MagState::DataFields magData = mags->getData();
        mag_accum_x.append(magData.x);
        mag_accum_y.append(magData.y);
        mag_accum_z.append(magData.z);
        break;
    }
    default:
        Q_ASSERT(0);
    }

    float p1   = (float)mag_accum_x.length() / (float)NOISE_SAMPLES;
    float p2   = (float)gyro_accum_x.length() / (float)NOISE_SAMPLES;
    float p3   = (float)accel_accum_x.length() / (float)NOISE_SAMPLES;

    float prog = (p1 < p2) ? p1 : p2;
    prog = (prog < p3) ? prog : p3;

    m_ui->noiseMeasurementProgress->setValue(prog * 100);

    if (mag_accum_x.length() >= NOISE_SAMPLES &&
        gyro_accum_x.length() >= NOISE_SAMPLES &&
        accel_accum_x.length() >= NOISE_SAMPLES) {
        // No need to for more updates
        MagState *mags = MagState::GetInstance(getObjectManager());
        AccelState *accelState = AccelState::GetInstance(getObjectManager());
        GyroState *gyroState   = GyroState::GetInstance(getObjectManager());
        disconnect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetNoiseSample(UAVObject *)));
        disconnect(gyroState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetNoiseSample(UAVObject *)));
        disconnect(mags, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(doGetNoiseSample(UAVObject *)));

        EKFConfiguration *ekfConfiguration = EKFConfiguration::GetInstance(getObjectManager());
        Q_ASSERT(ekfConfiguration);
        if (ekfConfiguration) {
            EKFConfiguration::DataFields revoCalData = ekfConfiguration->getData();
            revoCalData.Q[EKFConfiguration::Q_ACCELX] = listVar(accel_accum_x);
            revoCalData.Q[EKFConfiguration::Q_ACCELY] = listVar(accel_accum_y);
            revoCalData.Q[EKFConfiguration::Q_ACCELZ] = listVar(accel_accum_z);
            revoCalData.Q[EKFConfiguration::Q_GYROX]  = listVar(gyro_accum_x);
            revoCalData.Q[EKFConfiguration::Q_GYROY]  = listVar(gyro_accum_y);
            revoCalData.Q[EKFConfiguration::Q_GYROZ]  = listVar(gyro_accum_z);
            revoCalData.R[EKFConfiguration::R_MAGX]   = listVar(mag_accum_x);
            revoCalData.R[EKFConfiguration::R_MAGY]   = listVar(mag_accum_y);
            revoCalData.R[EKFConfiguration::R_MAGZ]   = listVar(mag_accum_z);
            ekfConfiguration->setData(revoCalData);
        }

        // Recall saved board rotation
        recallBoardRotation();
    }
}

/********** UI Functions *************/
/**
   Draws the sensor variances bargraph
 */
void ConfigRevoWidget::drawVariancesGraph()
{
    EKFConfiguration *ekfConfiguration = EKFConfiguration::GetInstance(getObjectManager());

    Q_ASSERT(ekfConfiguration);
    if (!ekfConfiguration) {
        return;
    }
    EKFConfiguration::DataFields ekfConfigurationData = ekfConfiguration->getData();

    // The expected range is from 1E-6 to 1E-1
    double steps = 6; // 6 bars on the graph
    float accel_x_var = -1 / steps * (1 + steps + log10(ekfConfigurationData.Q[EKFConfiguration::Q_ACCELX]));
    if (accel_x) {
        accel_x->setTransform(QTransform::fromScale(1, accel_x_var), false);
    }
    float accel_y_var = -1 / steps * (1 + steps + log10(ekfConfigurationData.Q[EKFConfiguration::Q_ACCELY]));
    if (accel_y) {
        accel_y->setTransform(QTransform::fromScale(1, accel_y_var), false);
    }
    float accel_z_var = -1 / steps * (1 + steps + log10(ekfConfigurationData.Q[EKFConfiguration::Q_ACCELZ]));
    if (accel_z) {
        accel_z->setTransform(QTransform::fromScale(1, accel_z_var), false);
    }

    float gyro_x_var = -1 / steps * (1 + steps + log10(ekfConfigurationData.Q[EKFConfiguration::Q_GYROX]));
    if (gyro_x) {
        gyro_x->setTransform(QTransform::fromScale(1, gyro_x_var), false);
    }
    float gyro_y_var = -1 / steps * (1 + steps + log10(ekfConfigurationData.Q[EKFConfiguration::Q_GYROY]));
    if (gyro_y) {
        gyro_y->setTransform(QTransform::fromScale(1, gyro_y_var), false);
    }
    float gyro_z_var = -1 / steps * (1 + steps + log10(ekfConfigurationData.Q[EKFConfiguration::Q_GYROZ]));
    if (gyro_z) {
        gyro_z->setTransform(QTransform::fromScale(1, gyro_z_var), false);
    }

    // Scale by 1e-3 because mag vars are much higher.
    float mag_x_var = -1 / steps * (1 + steps + log10(1e-3 * ekfConfigurationData.R[EKFConfiguration::R_MAGX]));
    if (mag_x) {
        mag_x->setTransform(QTransform::fromScale(1, mag_x_var), false);
    }
    float mag_y_var = -1 / steps * (1 + steps + log10(1e-3 * ekfConfigurationData.R[EKFConfiguration::R_MAGY]));
    if (mag_y) {
        mag_y->setTransform(QTransform::fromScale(1, mag_y_var), false);
    }
    float mag_z_var = -1 / steps * (1 + steps + log10(1e-3 * ekfConfigurationData.R[EKFConfiguration::R_MAGZ]));
    if (mag_z) {
        mag_z->setTransform(QTransform::fromScale(1, mag_z_var), false);
    }
}

/**
 * Called by the ConfigTaskWidget parent when RevoCalibration is updated
 * to update the UI
 */
void ConfigRevoWidget::refreshWidgetsValues(UAVObject *object)
{
    ConfigTaskWidget::refreshWidgetsValues(object);

    drawVariancesGraph();

    m_ui->noiseMeasurementStart->setEnabled(true);
    m_ui->sixPointsStart->setEnabled(true);
    m_ui->accelBiasStart->setEnabled(true);
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
