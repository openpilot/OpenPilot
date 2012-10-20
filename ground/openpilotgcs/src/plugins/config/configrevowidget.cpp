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
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QThread>
#include <QErrorMessage>
#include <iostream>
#include <QDesktopServices>
#include <QUrl>
#include <revocalibration.h>
#include <accels.h>
#include <gyros.h>
#include <magnetometer.h>

#define GRAVITY 9.81f
#include "assertions.h"
#include "calibration.h"

#define sign(x) ((x < 0) ? -1 : 1)

const double ConfigRevoWidget::maxVarValue = 0.1;

// *****************

class Thread : public QThread
{
public:
    static void usleep(unsigned long usecs)
    {
        QThread::usleep(usecs);
    }
};

// *****************

ConfigRevoWidget::ConfigRevoWidget(QWidget *parent) :
    ConfigTaskWidget(parent),
    collectingData(false),
    m_ui(new Ui_RevoSensorsWidget()),
    position(-1)
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
    m_ui->ahrsBargraph->setScene(new QGraphicsScene(this));

    QSvgRenderer *renderer = new QSvgRenderer();
    ahrsbargraph = new QGraphicsSvgItem();
    renderer->load(QString(":/configgadget/images/ahrs-calib.svg"));
    ahrsbargraph->setSharedRenderer(renderer);
    ahrsbargraph->setElementId("background");
    ahrsbargraph->setObjectName("background");
    m_ui->ahrsBargraph->scene()->addItem(ahrsbargraph);
    m_ui->ahrsBargraph->setSceneRect(ahrsbargraph->boundingRect());

    // Initialize the 9 bargraph values:

    QMatrix lineMatrix = renderer->matrixForElement("accel_x");
    QRectF rect = lineMatrix.mapRect(renderer->boundsOnElement("accel_x"));
    qreal startX = rect.x();
    qreal startY = rect.y()+ rect.height();
    // maxBarHeight will be used for scaling it later.
    maxBarHeight = rect.height();
    // Then once we have the initial location, we can put it
    // into a QGraphicsSvgItem which we will display at the same
    // place: we do this so that the heading scale can be clipped to
    // the compass dial region.
    accel_x = new QGraphicsSvgItem();
    accel_x->setSharedRenderer(renderer);
    accel_x->setElementId("accel_x");
    m_ui->ahrsBargraph->scene()->addItem(accel_x);
    accel_x->setPos(startX, startY);
    accel_x->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("accel_y");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("accel_y"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    accel_y = new QGraphicsSvgItem();
    accel_y->setSharedRenderer(renderer);
    accel_y->setElementId("accel_y");
    m_ui->ahrsBargraph->scene()->addItem(accel_y);
    accel_y->setPos(startX,startY);
    accel_y->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("accel_z");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("accel_z"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    accel_z = new QGraphicsSvgItem();
    accel_z->setSharedRenderer(renderer);
    accel_z->setElementId("accel_z");
    m_ui->ahrsBargraph->scene()->addItem(accel_z);
    accel_z->setPos(startX,startY);
    accel_z->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("gyro_x");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("gyro_x"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    gyro_x = new QGraphicsSvgItem();
    gyro_x->setSharedRenderer(renderer);
    gyro_x->setElementId("gyro_x");
    m_ui->ahrsBargraph->scene()->addItem(gyro_x);
    gyro_x->setPos(startX,startY);
    gyro_x->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("gyro_y");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("gyro_y"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    gyro_y = new QGraphicsSvgItem();
    gyro_y->setSharedRenderer(renderer);
    gyro_y->setElementId("gyro_y");
    m_ui->ahrsBargraph->scene()->addItem(gyro_y);
    gyro_y->setPos(startX,startY);
    gyro_y->setTransform(QTransform::fromScale(1,0),true);


    lineMatrix = renderer->matrixForElement("gyro_z");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("gyro_z"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    gyro_z = new QGraphicsSvgItem();
    gyro_z->setSharedRenderer(renderer);
    gyro_z->setElementId("gyro_z");
    m_ui->ahrsBargraph->scene()->addItem(gyro_z);
    gyro_z->setPos(startX,startY);
    gyro_z->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("mag_x");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("mag_x"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    mag_x = new QGraphicsSvgItem();
    mag_x->setSharedRenderer(renderer);
    mag_x->setElementId("mag_x");
    m_ui->ahrsBargraph->scene()->addItem(mag_x);
    mag_x->setPos(startX,startY);
    mag_x->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("mag_y");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("mag_y"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    mag_y = new QGraphicsSvgItem();
    mag_y->setSharedRenderer(renderer);
    mag_y->setElementId("mag_y");
    m_ui->ahrsBargraph->scene()->addItem(mag_y);
    mag_y->setPos(startX,startY);
    mag_y->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("mag_z");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("mag_z"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    mag_z = new QGraphicsSvgItem();
    mag_z->setSharedRenderer(renderer);
    mag_z->setElementId("mag_z");
    m_ui->ahrsBargraph->scene()->addItem(mag_z);
    mag_z->setPos(startX,startY);
    mag_z->setTransform(QTransform::fromScale(1,0),true);

    // Connect the signals
    connect(m_ui->accelBiasStart, SIGNAL(clicked()), this, SLOT(launchAccelBiasCalibration()));

    RevoCalibration * revoCalibration = RevoCalibration::GetInstance(getObjectManager());
    Q_ASSERT(revoCalibration);
    connect(revoCalibration, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshValues()));

    connect(m_ui->ahrsSettingsSaveRAM, SIGNAL(clicked()), this, SLOT(SettingsToRAM()));
    connect(m_ui->ahrsSettingsSaveSD, SIGNAL(clicked()), this, SLOT(SettingsToFlash()));
    connect(m_ui->sixPointsStart, SIGNAL(clicked()), this, SLOT(sixPointCalibrationMode()));
    connect(m_ui->sixPointsSave, SIGNAL(clicked()), this, SLOT(savePositionData()));

    // Leave this timer permanently connected.  The timer itself is started and stopped.
    connect(&progressBarTimer, SIGNAL(timeout()), this, SLOT(incrementProgress()));

    // Order is important: 1st request the settings (it will also enable the controls)
    // then explicitely disable them. They will be re-enabled right afterwards by the
    // configgadgetwidget if the autopilot is actually connected.
    refreshValues();
    // when the AHRS Widget is instanciated, the autopilot is always connected // enableControls(false);
    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(onAutopilotConnect()));
    connect(parent, SIGNAL(autopilotDisconnected()), this, SLOT(onAutopilotDisconnect()));

    // Connect the help button
    connect(m_ui->ahrsHelp, SIGNAL(clicked()), this, SLOT(openHelp()));
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
    // the result is usually a ahrsbargraph that is way too small.
    m_ui->ahrsBargraph->fitInView(ahrsbargraph, Qt::KeepAspectRatio);
    m_ui->sixPointsHelp->fitInView(paperplane,Qt::KeepAspectRatio);
}

void ConfigRevoWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    m_ui->ahrsBargraph->fitInView(ahrsbargraph, Qt::KeepAspectRatio);
    m_ui->sixPointsHelp->fitInView(paperplane,Qt::KeepAspectRatio);
}


void ConfigRevoWidget::enableControls(bool enable)
{
    //m_ui->ahrsSettingsSaveRAM->setEnabled(enable);
    m_ui->ahrsSettingsSaveSD->setEnabled(enable);
}

/**
  Starts an accelerometer bias calibration.
  */
void ConfigRevoWidget::launchAccelBiasCalibration()
{
    m_ui->accelBiasStart->setEnabled(false);
    m_ui->accelBiasProgress->setValue(0);

    RevoCalibration * revoCalibration = RevoCalibration::GetInstance(getObjectManager());
    Q_ASSERT(revoCalibration);
    RevoCalibration::DataFields revoCalibrationData = revoCalibration->getData();
    revoCalibrationData.BiasCorrectedRaw = RevoCalibration::BIASCORRECTEDRAW_FALSE;
    revoCalibration->setData(revoCalibrationData);
    revoCalibration->updated();

    accel_accum_x.clear();
    accel_accum_y.clear();
    accel_accum_z.clear();

    /* Need to get as many AttitudeRaw updates as possible */
    Accels * accels = Accels::GetInstance(getObjectManager());
    Q_ASSERT(accels);
    initialMdata = accels->getMetadata();
    UAVObject::Metadata mdata = initialMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    accels->setMetadata(mdata);

    // Now connect to the accels and mag updates, gather for 100 samples
    collectingData = true;
    connect(accels, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(accelBiasattitudeRawUpdated(UAVObject*)));
}

/**
  Updates the accel bias raw values
  */
void ConfigRevoWidget::accelBiasattitudeRawUpdated(UAVObject *obj)
{
    Q_UNUSED(obj);

    Accels * accels = Accels::GetInstance(getObjectManager());
    Q_ASSERT(accels);
    Accels::DataFields accelsData = accels->getData();

    // This is necessary to prevent a race condition on disconnect signal and another update
    if (collectingData == true) {
        accel_accum_x.append(accelsData.x);
        accel_accum_y.append(accelsData.y);
        accel_accum_z.append(accelsData.z);
    }

    m_ui->accelBiasProgress->setValue(m_ui->accelBiasProgress->value()+1);

    if(accel_accum_x.size() >= 100 && collectingData == true) {
        collectingData = false;
        disconnect(accels,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(accelBiasattitudeRawUpdated(UAVObject*)));
        m_ui->accelBiasStart->setEnabled(true);

        RevoCalibration * revoCalibration = RevoCalibration::GetInstance(getObjectManager());
        Q_ASSERT(revoCalibration);
        RevoCalibration::DataFields revoCalibrationData = revoCalibration->getData();
        revoCalibrationData.BiasCorrectedRaw = RevoCalibration::BIASCORRECTEDRAW_TRUE;

        revoCalibrationData.accel_bias[RevoCalibration::ACCEL_BIAS_X] -= listMean(accel_accum_x);
        revoCalibrationData.accel_bias[RevoCalibration::ACCEL_BIAS_Y] -= listMean(accel_accum_y);
        revoCalibrationData.accel_bias[RevoCalibration::ACCEL_BIAS_Z] -= GRAVITY + listMean(accel_accum_z);

        revoCalibration->setData(revoCalibrationData);
        revoCalibration->updated();

        accels->setMetadata(initialMdata);
    }
}

/**
  Increment progress bar for noise measurements (not really based on feedback)
  */
void ConfigRevoWidget::incrementProgress()
{
    m_ui->calibProgress->setValue(m_ui->calibProgress->value()+1);
    if (m_ui->calibProgress->value() >= m_ui->calibProgress->maximum()) {
        progressBarTimer.stop();

        RevoCalibration * revoCalibration = RevoCalibration::GetInstance(getObjectManager());
        Q_ASSERT(revoCalibration);
        disconnect(revoCalibration, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(noiseMeasured()));
        collectingData = false;

        QErrorMessage err(this);
        err.showMessage("Noise measurement timed out.  State undetermined.  Please power cycle.");
        err.exec();
    }
}

void ConfigRevoWidget::sensorsUpdated(UAVObject * obj)
{
    QMutexLocker lock(&attitudeRawUpdateLock);

       qDebug() << "Data";
    // This is necessary to prevent a race condition on disconnect signal and another update
    if (collectingData == true) {
        qDebug() << "Collecting";
        if( obj->getObjID() == Accels::OBJID ) {
            qDebug() << "Accels";
            Accels * accels = Accels::GetInstance(getObjectManager());
            Q_ASSERT(accels);
            Accels::DataFields accelsData = accels->getData();
            accel_accum_x.append(accelsData.x);
            accel_accum_y.append(accelsData.y);
            accel_accum_z.append(accelsData.z);
        } else if( obj->getObjID() == Magnetometer::OBJID ) {
            qDebug() << "Mag";
            Magnetometer * mag = Magnetometer::GetInstance(getObjectManager());
            Q_ASSERT(mag);
            Magnetometer::DataFields magData = mag->getData();
            mag_accum_x.append(magData.x);
            mag_accum_y.append(magData.y);
            mag_accum_z.append(magData.z);
        } else {
            Q_ASSERT(0);
        }
    }

    if(accel_accum_x.size() >= 20 && mag_accum_x.size() >= 20 && collectingData == true) {
        collectingData = false;

        Accels * accels = Accels::GetInstance(getObjectManager());
        Q_ASSERT(accels);
        Magnetometer * mag = Magnetometer::GetInstance(getObjectManager());
        Q_ASSERT(mag);
        disconnect(accels,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(sensorsUpdated(UAVObject*)));
        disconnect(mag,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(sensorsUpdated(UAVObject*)));

        m_ui->sixPointsSave->setEnabled(true);

        accel_data_x[position] = listMean(accel_accum_x);
        accel_data_y[position] = listMean(accel_accum_y);
        accel_data_z[position] = listMean(accel_accum_z);
        mag_data_x[position] = listMean(mag_accum_x);
        mag_data_y[position] = listMean(mag_accum_y);
        mag_data_z[position] = listMean(mag_accum_z);

        position = (position + 1) % 6;
        if(position == 1) {
            m_ui->sixPointCalibInstructions->append("Place with left side down and click save position...");
            displayPlane("plane-left");
        }
        if(position == 2) {
            m_ui->sixPointCalibInstructions->append("Place upside down and click save position...");
            displayPlane("plane-flip");
        }
        if(position == 3) {
            m_ui->sixPointCalibInstructions->append("Place with right side down and click save position...");
            displayPlane("plane-right");
        }
        if(position == 4) {
            m_ui->sixPointCalibInstructions->append("Place with nose up and click save position...");
            displayPlane("plane-up");
        }
        if(position == 5) {
            m_ui->sixPointCalibInstructions->append("Place with nose down and click save position...");
            displayPlane("plane-down");
        }
        if(position == 0) {
            computeScaleBias();
            m_ui->sixPointsStart->setEnabled(true);
            m_ui->sixPointsSave->setEnabled(false);

            /* Cleanup original settings */
            accels->setMetadata(initialMdata);
            mag->setMetadata(initialMdata);
        }
    }
}

/**
  * Saves the data from the aircraft in one of six positions
  */
void ConfigRevoWidget::savePositionData()
{    
    QMutexLocker lock(&attitudeRawUpdateLock);
    m_ui->sixPointsSave->setEnabled(false);

    accel_accum_x.clear();
    accel_accum_y.clear();
    accel_accum_z.clear();
    mag_accum_x.clear();
    mag_accum_y.clear();
    mag_accum_z.clear();
    gyro_accum_x.clear();
    gyro_accum_y.clear();
    gyro_accum_z.clear();

    collectingData = true;

    Accels * accels = Accels::GetInstance(getObjectManager());
    Q_ASSERT(accels);
    Magnetometer * mag = Magnetometer::GetInstance(getObjectManager());
    Q_ASSERT(mag);

    connect(accels, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(sensorsUpdated(UAVObject*)));
    connect(mag, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(sensorsUpdated(UAVObject*)));

    m_ui->sixPointCalibInstructions->append("Hold...");
}

int LinearEquationsSolving(int nDim, double* pfMatr, double* pfVect, double* pfSolution)
{
 double fMaxElem;
 double fAcc;

 int i , j, k, m;

 for(k=0; k<(nDim-1); k++) // base row of matrix
 {
   // search of line with max element
   fMaxElem = fabs( pfMatr[k*nDim + k] );
   m = k;
   for(i=k+1; i<nDim; i++)
   {
     if(fMaxElem < fabs(pfMatr[i*nDim + k]) )
     {
       fMaxElem = pfMatr[i*nDim + k];
       m = i;
     }
   }

   // permutation of base line (index k) and max element line(index m)
   if(m != k)
   {
     for(i=k; i<nDim; i++)
     {
       fAcc               = pfMatr[k*nDim + i];
       pfMatr[k*nDim + i] = pfMatr[m*nDim + i];
       pfMatr[m*nDim + i] = fAcc;
     }
     fAcc = pfVect[k];
     pfVect[k] = pfVect[m];
     pfVect[m] = fAcc;
   }

   if( pfMatr[k*nDim + k] == 0.) return 0; // needs improvement !!!

   // triangulation of matrix with coefficients
   for(j=(k+1); j<nDim; j++) // current row of matrix
   {
     fAcc = - pfMatr[j*nDim + k] / pfMatr[k*nDim + k];
     for(i=k; i<nDim; i++)
     {
       pfMatr[j*nDim + i] = pfMatr[j*nDim + i] + fAcc*pfMatr[k*nDim + i];
     }
     pfVect[j] = pfVect[j] + fAcc*pfVect[k]; // free member recalculation
   }
 }

 for(k=(nDim-1); k>=0; k--)
 {
   pfSolution[k] = pfVect[k];
   for(i=(k+1); i<nDim; i++)
   {
     pfSolution[k] -= (pfMatr[k*nDim + i]*pfSolution[i]);
   }
   pfSolution[k] = pfSolution[k] / pfMatr[k*nDim + k];
 }

 return 1;
}


int SixPointInConstFieldCal( double ConstMag, double x[6], double y[6], double z[6], double S[3], double b[3] )
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
 for (i=0;i<5;i++){
     A[i][0] = 2.0 * (x[i+1] - x[i]);
     A[i][1] = y[i+1]*y[i+1] - y[i]*y[i];
     A[i][2] = 2.0 * (y[i+1] - y[i]);
     A[i][3] = z[i+1]*z[i+1] - z[i]*z[i];
     A[i][4] = 2.0 * (z[i+1] - z[i]);
     f[i]    = x[i]*x[i] - x[i+1]*x[i+1];
 }

 // solve for c0=bx/Sx, c1=Sy^2/Sx^2; c2=Sy*by/Sx^2, c3=Sz^2/Sx^2, c4=Sz*bz/Sx^2
 if (  !LinearEquationsSolving( 5, (double *)A, f, c) ) return 0;

 // use one magnitude equation and c's to find Sx - doesn't matter which - all give the same answer
 xp = x[0]; yp = y[0]; zp = z[0];
 Sx = sqrt(ConstMag*ConstMag / (xp*xp + 2*c[0]*xp + c[0]*c[0] + c[1]*yp*yp + 2*c[2]*yp + c[2]*c[2]/c[1] + c[3]*zp*zp + 2*c[4]*zp + c[4]*c[4]/c[3]));

 S[0] = Sx;
 b[0] = Sx*c[0];
 S[1] = sqrt(c[1]*Sx*Sx);
 b[1] = c[2]*Sx*Sx/S[1];
 S[2] = sqrt(c[3]*Sx*Sx);
 b[2] = c[4]*Sx*Sx/S[2];

 return 1;
}

void ConfigRevoWidget::computeScaleBias()
{
   double S[3], b[3];
   RevoCalibration * revoCalibration = RevoCalibration::GetInstance(getObjectManager());
   Q_ASSERT(revoCalibration);
   RevoCalibration::DataFields revoCalibrationData = revoCalibration->getData();

   // Calibration accel
   SixPointInConstFieldCal( GRAVITY, accel_data_x, accel_data_y, accel_data_z, S, b);
   revoCalibrationData.accel_scale[RevoCalibration::ACCEL_SCALE_X] = fabs(S[0]);
   revoCalibrationData.accel_scale[RevoCalibration::ACCEL_SCALE_Y] = fabs(S[1]);
   revoCalibrationData.accel_scale[RevoCalibration::ACCEL_SCALE_Z] = fabs(S[2]);

   revoCalibrationData.accel_bias[RevoCalibration::ACCEL_BIAS_X] = -sign(S[0]) * b[0];
   revoCalibrationData.accel_bias[RevoCalibration::ACCEL_BIAS_Y] = -sign(S[1]) * b[1];
   revoCalibrationData.accel_bias[RevoCalibration::ACCEL_BIAS_Z] = -sign(S[2]) * b[2];

   // Calibration mag
   SixPointInConstFieldCal( 1000, mag_data_x, mag_data_y, mag_data_z, S, b);
   revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_X] = fabs(S[0]);
   revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Y] = fabs(S[1]);
   revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Z] = fabs(S[2]);

   revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X] = -sign(S[0]) * b[0];
   revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y] = -sign(S[1]) * b[1];
   revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z] = -sign(S[2]) * b[2];

   revoCalibration->setData(revoCalibrationData);

   position = -1; //set to run again
   m_ui->sixPointCalibInstructions->append("Computed accel and mag scale and bias...");

}

/**
  Six point calibration mode
  */
void ConfigRevoWidget::sixPointCalibrationMode()
{
    RevoCalibration * revoCalibration = RevoCalibration::GetInstance(getObjectManager());
    Q_ASSERT(revoCalibration);
    RevoCalibration::DataFields revoCalibrationData = revoCalibration->getData();

    // Calibration accel
    revoCalibrationData.accel_scale[RevoCalibration::ACCEL_SCALE_X] = 1;
    revoCalibrationData.accel_scale[RevoCalibration::ACCEL_SCALE_Y] = 1;
    revoCalibrationData.accel_scale[RevoCalibration::ACCEL_SCALE_Z] = 1;
    revoCalibrationData.accel_bias[RevoCalibration::ACCEL_BIAS_X] = 0;
    revoCalibrationData.accel_bias[RevoCalibration::ACCEL_BIAS_Y] = 0;
    revoCalibrationData.accel_bias[RevoCalibration::ACCEL_BIAS_Z] = 0;

    // Calibration mag
    revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_X] = 1;
    revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Y] = 1;
    revoCalibrationData.mag_scale[RevoCalibration::MAG_SCALE_Z] = 1;
    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X] = 0;
    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y] = 0;
    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z] = 0;

    revoCalibration->setData(revoCalibrationData);

   Thread::usleep(100000);

   gyro_accum_x.clear();
   gyro_accum_y.clear();
   gyro_accum_z.clear();
   accel_accum_x.clear();
   accel_accum_y.clear();
   accel_accum_z.clear();
   mag_accum_x.clear();
   mag_accum_y.clear();
   mag_accum_z.clear();

   /* Need to get as many accel and mag updates as possible */
   Accels * accels = Accels::GetInstance(getObjectManager());
   Q_ASSERT(accels);
   Magnetometer * mag = Magnetometer::GetInstance(getObjectManager());
   Q_ASSERT(mag);

   initialMdata = accels->getMetadata();
   UAVObject::Metadata mdata = initialMdata;
   UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
   mdata.flightTelemetryUpdatePeriod = 100;
   accels->setMetadata(mdata);

   mdata = mag->getMetadata();
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
   qDebug() << "Starting";
}



/**
  Rotate the paper plane
  */
void ConfigRevoWidget::displayPlane(QString elementID)
{
    paperplane->setElementId(elementID);
    m_ui->sixPointsHelp->setSceneRect(paperplane->boundingRect());
    m_ui->sixPointsHelp->fitInView(paperplane,Qt::KeepAspectRatio);

}


/**
  Draws the sensor variances bargraph
  */
void ConfigRevoWidget::drawVariancesGraph()
{
    RevoCalibration * revoCalibration = RevoCalibration::GetInstance(getObjectManager());
    Q_ASSERT(revoCalibration);
    RevoCalibration::DataFields revoCalibrationData = revoCalibration->getData();

    // The expected range is from 1E-6 to 1E-1
    double steps = 6; // 6 bars on the graph
    float accel_x_var = -1/steps*(1+steps+log10(revoCalibrationData.accel_var[RevoCalibration::ACCEL_VAR_X]));
    accel_x->setTransform(QTransform::fromScale(1,accel_x_var),false);
    float accel_y_var = -1/steps*(1+steps+log10(revoCalibrationData.accel_var[RevoCalibration::ACCEL_VAR_Y]));
    accel_y->setTransform(QTransform::fromScale(1,accel_y_var),false);
    float accel_z_var = -1/steps*(1+steps+log10(revoCalibrationData.accel_var[RevoCalibration::ACCEL_VAR_Z]));
    accel_z->setTransform(QTransform::fromScale(1,accel_z_var),false);

    float gyro_x_var = -1/steps*(1+steps+log10(revoCalibrationData.gyro_var[RevoCalibration::GYRO_VAR_X]));
    gyro_x->setTransform(QTransform::fromScale(1,gyro_x_var),false);
    float gyro_y_var = -1/steps*(1+steps+log10(revoCalibrationData.gyro_var[RevoCalibration::GYRO_VAR_Y]));
    gyro_y->setTransform(QTransform::fromScale(1,gyro_y_var),false);
    float gyro_z_var = -1/steps*(1+steps+log10(revoCalibrationData.gyro_var[RevoCalibration::GYRO_VAR_Z]));
    gyro_z->setTransform(QTransform::fromScale(1,gyro_z_var),false);

    // Scale by 1e-3 because mag vars are much higher.
    float mag_x_var = -1/steps*(1+steps+log10(1e-3*revoCalibrationData.mag_var[RevoCalibration::MAG_VAR_X]));
    mag_x->setTransform(QTransform::fromScale(1,mag_x_var),false);
    float mag_y_var = -1/steps*(1+steps+log10(1e-3*revoCalibrationData.mag_var[RevoCalibration::MAG_VAR_Y]));
    mag_y->setTransform(QTransform::fromScale(1,mag_y_var),false);
    float mag_z_var = -1/steps*(1+steps+log10(1e-3*revoCalibrationData.mag_var[RevoCalibration::MAG_VAR_Z]));
    mag_z->setTransform(QTransform::fromScale(1,mag_z_var),false);
}

/**
  Request current settings from the AHRS
  */
void ConfigRevoWidget::refreshValues()
{
    drawVariancesGraph();

    m_ui->ahrsCalibStart->setEnabled(true);
    m_ui->sixPointsStart->setEnabled(true);
    m_ui->accelBiasStart->setEnabled(true);
    m_ui->startDriftCalib->setEnabled(true);

    m_ui->calibInstructions->setText(QString("Press \"Start\" above to calibrate."));
}


/**
  Save current settings to RAM
  */
void ConfigRevoWidget::SettingsToRAM()
{
    RevoCalibration * revoCalibration = RevoCalibration::GetInstance(getObjectManager());
    Q_ASSERT(revoCalibration);
    revoCalibration->updated();
}

/**
Save Revo calibration settings to flash
  */
void ConfigRevoWidget::SettingsToFlash()
{
    SettingsToRAM();

    RevoCalibration * revoCalibration = RevoCalibration::GetInstance(getObjectManager());
    Q_ASSERT(revoCalibration);
    saveObjectToSD(revoCalibration);
}

void ConfigRevoWidget::openHelp()
{
    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/display/Doc/Revo+Configuration", QUrl::StrictMode) );
}

/**
  @}
  @}
  */
