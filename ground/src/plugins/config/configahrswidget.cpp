/**
 ******************************************************************************
 *
 * @file       configahrswidget.h
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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
#include "configahrswidget.h"

#include "math.h"
#include <QDebug>
#include <QTimer>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

#define sign(x) ((x < 0) ? -1 : 1)

const double ConfigAHRSWidget::maxVarValue = 0.1;
const int ConfigAHRSWidget::calibrationDelay = 7; // Time to wait for the AHRS to do its calibration

ConfigAHRSWidget::ConfigAHRSWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_ahrs = new Ui_AHRSWidget();
    m_ahrs->setupUi(this);

    // Initialization of the Paper plane widget
    m_ahrs->sixPointsHelp->setScene(new QGraphicsScene(this));

    paperplane = new QGraphicsSvgItem();
    paperplane->setSharedRenderer(new QSvgRenderer());
    paperplane->renderer()->load(QString(":/configgadget/images/paper-plane.svg"));
    paperplane->setElementId("plane-horizontal");
    m_ahrs->sixPointsHelp->scene()->addItem(paperplane);
    m_ahrs->sixPointsHelp->setSceneRect(paperplane->boundingRect());

    // Initialization of the AHRS bargraph graph

    m_ahrs->ahrsBargraph->setScene(new QGraphicsScene(this));

    QSvgRenderer *renderer = new QSvgRenderer();
    ahrsbargraph = new QGraphicsSvgItem();
    renderer->load(QString(":/configgadget/images/ahrs-calib.svg"));
    ahrsbargraph->setSharedRenderer(renderer);
    ahrsbargraph->setElementId("background");
    ahrsbargraph->setObjectName("background");
    m_ahrs->ahrsBargraph->scene()->addItem(ahrsbargraph);
    m_ahrs->ahrsBargraph->setSceneRect(ahrsbargraph->boundingRect());

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
    m_ahrs->ahrsBargraph->scene()->addItem(accel_x);
    accel_x->setPos(startX, startY);
    accel_x->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("accel_y");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("accel_y"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    accel_y = new QGraphicsSvgItem();
    accel_y->setSharedRenderer(renderer);
    accel_y->setElementId("accel_y");
    m_ahrs->ahrsBargraph->scene()->addItem(accel_y);
    accel_y->setPos(startX,startY);
    accel_y->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("accel_z");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("accel_z"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    accel_z = new QGraphicsSvgItem();
    accel_z->setSharedRenderer(renderer);
    accel_z->setElementId("accel_z");
    m_ahrs->ahrsBargraph->scene()->addItem(accel_z);
    accel_z->setPos(startX,startY);
    accel_z->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("gyro_x");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("gyro_x"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    gyro_x = new QGraphicsSvgItem();
    gyro_x->setSharedRenderer(renderer);
    gyro_x->setElementId("gyro_x");
    m_ahrs->ahrsBargraph->scene()->addItem(gyro_x);
    gyro_x->setPos(startX,startY);
    gyro_x->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("gyro_y");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("gyro_y"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    gyro_y = new QGraphicsSvgItem();
    gyro_y->setSharedRenderer(renderer);
    gyro_y->setElementId("gyro_y");
    m_ahrs->ahrsBargraph->scene()->addItem(gyro_y);
    gyro_y->setPos(startX,startY);
    gyro_y->setTransform(QTransform::fromScale(1,0),true);


    lineMatrix = renderer->matrixForElement("gyro_z");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("gyro_z"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    gyro_z = new QGraphicsSvgItem();
    gyro_z->setSharedRenderer(renderer);
    gyro_z->setElementId("gyro_z");
    m_ahrs->ahrsBargraph->scene()->addItem(gyro_z);
    gyro_z->setPos(startX,startY);
    gyro_z->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("mag_x");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("mag_x"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    mag_x = new QGraphicsSvgItem();
    mag_x->setSharedRenderer(renderer);
    mag_x->setElementId("mag_x");
    m_ahrs->ahrsBargraph->scene()->addItem(mag_x);
    mag_x->setPos(startX,startY);
    mag_x->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("mag_y");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("mag_y"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    mag_y = new QGraphicsSvgItem();
    mag_y->setSharedRenderer(renderer);
    mag_y->setElementId("mag_y");
    m_ahrs->ahrsBargraph->scene()->addItem(mag_y);
    mag_y->setPos(startX,startY);
    mag_y->setTransform(QTransform::fromScale(1,0),true);

    lineMatrix = renderer->matrixForElement("mag_z");
    rect = lineMatrix.mapRect(renderer->boundsOnElement("mag_z"));
    startX = rect.x();
    startY = rect.y()+ rect.height();
    mag_z = new QGraphicsSvgItem();
    mag_z->setSharedRenderer(renderer);
    mag_z->setElementId("mag_z");
    m_ahrs->ahrsBargraph->scene()->addItem(mag_z);
    mag_z->setPos(startX,startY);
    mag_z->setTransform(QTransform::fromScale(1,0),true);

    position = -1;

    // Fill the dropdown menus:
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSSettings")));
    UAVObjectField *field = obj->getField(QString("Algorithm"));
    m_ahrs->algorithm->addItems(field->getOptions());

    // Register for Home Location state changes
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("HomeLocation")));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this , SLOT(enableHomeLocSave(UAVObject*)));

    // Connect the signals
    connect(m_ahrs->ahrsCalibStart, SIGNAL(clicked()), this, SLOT(launchAHRSCalibration()));
    connect(m_ahrs->ahrsSettingsRequest, SIGNAL(clicked()), this, SLOT(ahrsSettingsRequest()));
    /*
    connect(m_ahrs->algorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(ahrsSettingsSave()));
    connect(m_ahrs->indoorFlight, SIGNAL(stateChanged(int)), this, SLOT(homeLocationSave()));
    connect(m_ahrs->homeLocation, SIGNAL(clicked()), this, SLOT(homeLocationSaveSD()));
    */
    connect(m_ahrs->ahrsSettingsSaveRAM, SIGNAL(clicked()), this, SLOT(ahrsSettingsSaveRAM()));
    connect(m_ahrs->ahrsSettingsSaveSD, SIGNAL(clicked()), this, SLOT(ahrsSettingsSaveSD()));
    connect(m_ahrs->sixPointsStart, SIGNAL(clicked()), this, SLOT(sixPointCalibrationMode()));
    connect(m_ahrs->sixPointsSave, SIGNAL(clicked()), this, SLOT(savePositionData()));
    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(ahrsSettingsRequest()));


}

ConfigAHRSWidget::~ConfigAHRSWidget()
{
   // Do nothing
}


void ConfigAHRSWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    m_ahrs->ahrsBargraph->fitInView(ahrsbargraph, Qt::KeepAspectRatio);
    m_ahrs->sixPointsHelp->fitInView(paperplane,Qt::KeepAspectRatio);
}

/**
  Launches the AHRS sensors calibration
  */
void ConfigAHRSWidget::launchAHRSCalibration()
{
    m_ahrs->calibInstructions->setText("Estimating sensor variance...");
    m_ahrs->ahrsCalibStart->setEnabled(false);

    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSCalibration")));
    UAVObjectField *field = obj->getField(QString("measure_var"));
    field->setValue("MEASURE");
    obj->updated();

    QTimer::singleShot(calibrationDelay*1000, this, SLOT(calibPhase2()));
    m_ahrs->calibProgress->setRange(0,calibrationDelay);
    phaseCounter = 0;
    progressBarIndex = 0;
    connect(&progressBarTimer, SIGNAL(timeout()), this, SLOT(incrementProgress()));
    progressBarTimer.start(1000);
}

/**
  Increment progress bar
  */
void ConfigAHRSWidget::incrementProgress()
{
    m_ahrs->calibProgress->setValue(progressBarIndex++);
    if (progressBarIndex > m_ahrs->calibProgress->maximum()) {
        progressBarTimer.stop();
        progressBarIndex = 0;
    }
}


/**
  Callback once calibration is done on the board.

  Currently we don't have a way to tell if calibration is finished, so we
  have to use a timer.

  calibPhase2 is also connected to the AHRSCalibration object update signal.


  */
void ConfigAHRSWidget::calibPhase2()
{
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSCalibration")));
    UAVObjectField *field = obj->getField(QString("measure_var"));

    //  This is a bit weird, but it is because we are expecting an update from the
      // OP board with the correct calibration values, and those only arrive on the object update
      // which comes back from the board, and not the first object update signal which is in fast
      // the object update we did ourselves... Clear ?
      switch (phaseCounter) {
      case 0:
          phaseCounter++;
          m_ahrs->calibInstructions->setText("Getting results...");
          connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(calibPhase2()));
          //  We need to echo back the results of calibration before changing to set mode
          obj->requestUpdate();
          break;
      case 1:  // This is the update with the right values (coming from the board)
          disconnect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(calibPhase2()));
          // Now update size of all the graphs
          drawVariancesGraph();
          saveAHRSCalibration();
          m_ahrs->calibInstructions->setText(QString("Calibration saved."));
          m_ahrs->ahrsCalibStart->setEnabled(true);
          break;
      }
}

/**
  Saves the AHRS sensors calibration (to RAM and SD)
  */
void ConfigAHRSWidget::saveAHRSCalibration()
{
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSCalibration")));
    UAVObjectField *field = obj->getField(QString("measure_var"));
    field->setValue("SET");
    obj->updated();
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);

}

void ConfigAHRSWidget::attitudeRawUpdated(UAVObject * obj)
{
    QMutexLocker lock(&attitudeRawUpdateLock);
    UAVObjectField *accel_field = obj->getField(QString("accels_filtered"));
    UAVObjectField *gyro_field = obj->getField(QString("gyros_filtered"));
    UAVObjectField *mag_field = obj->getField(QString("magnetometers"));

    Q_ASSERT(gyro_field != 0 && accel_field != 0 & mag_field != 0);

    // This is necessary to prevent a race condition on disconnect signal and another update
    if (collectingData == true) {
        accel_accum_x.append(accel_field->getValue(0).toDouble());
        accel_accum_y.append(accel_field->getValue(1).toDouble());
        accel_accum_z.append(accel_field->getValue(2).toDouble());
        // Note gyros actually (-y,-x,-z) but since we consistent here no prob
        mag_accum_x.append(mag_field->getValue(0).toDouble());
        mag_accum_y.append(mag_field->getValue(1).toDouble());
        mag_accum_z.append(mag_field->getValue(2).toDouble());
        gyro_accum_x.append(gyro_field->getValue(0).toDouble());
        gyro_accum_y.append(gyro_field->getValue(1).toDouble());
        gyro_accum_z.append(gyro_field->getValue(2).toDouble());
    }

    if(accel_accum_x.size() >= 20 && collectingData == true) {
        collectingData = false;
        disconnect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(attitudeRawUpdated(UAVObject*)));
        m_ahrs->sixPointsSave->setEnabled(true);

        accel_data_x[position] = listMean(accel_accum_x);
        accel_data_y[position] = listMean(accel_accum_y);
        accel_data_z[position] = listMean(accel_accum_z);
        mag_data_x[position] = listMean(mag_accum_x);
        mag_data_y[position] = listMean(mag_accum_y);
        mag_data_z[position] = listMean(mag_accum_z);

        position = (position + 1) % 6;
        if(position == 1) {
            m_ahrs->sixPointCalibInstructions->append("Place with left side down and click save position...");
            displayPlane("plane-left");
        }
        if(position == 2) {
            m_ahrs->sixPointCalibInstructions->append("Place upside down and click save position...");
            displayPlane("plane-flip");
        }
        if(position == 3) {
            m_ahrs->sixPointCalibInstructions->append("Place with right side down and click save position...");
            displayPlane("plane-right");
        }
        if(position == 4) {
            m_ahrs->sixPointCalibInstructions->append("Place with nose up and click save position...");
            displayPlane("plane-up");
        }
        if(position == 5) {
            m_ahrs->sixPointCalibInstructions->append("Place with nose down and click save position...");
            displayPlane("plane-down");
        }
        if(position == 0) {
            computeScaleBias();
            m_ahrs->sixPointsStart->setEnabled(true);
            m_ahrs->sixPointsSave->setEnabled(false);
            saveAHRSCalibration(); // Saves the result to SD.

            /* Cleanup original settings */
            getObjectManager()->getObject(QString("AttitudeRaw"))->setMetadata(initialMdata);
        }
    }
}

double ConfigAHRSWidget::listMean(QList<double> list)
{
    double accum = 0;
    for(int i = 0; i < list.size(); i++)
        accum += list[i];
    return accum / list.size();
}

/**
  * Saves the data from the aircraft in one of six positions
  */
void ConfigAHRSWidget::savePositionData()
{    
    QMutexLocker lock(&attitudeRawUpdateLock);
    m_ahrs->sixPointsSave->setEnabled(false);

    accel_accum_x.clear();
    accel_accum_y.clear();
    accel_accum_z.clear();
    mag_accum_x.clear();
    mag_accum_y.clear();
    mag_accum_z.clear();

    collectingData = true;
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AttitudeRaw")));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(attitudeRawUpdated(UAVObject*)));

    m_ahrs->sixPointCalibInstructions->append("Hold...");
}

//*****************************************************************

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

void ConfigAHRSWidget::computeScaleBias()
{
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSCalibration")));
    UAVObjectField *field;
    double S[3], b[3];

    SixPointInConstFieldCal( 9.81, accel_data_x, accel_data_y, accel_data_z, S, b);

    field = obj->getField(QString("gyro_bias"));
    field->setDouble(listMean(gyro_accum_x),0);
    field->setDouble(listMean(gyro_accum_y),1);
    field->setDouble(listMean(gyro_accum_z),2);

    field = obj->getField(QString("accel_scale"));
    field->setDouble(sign(S[0]) * S[0],0);
    field->setDouble(sign(S[1]) * S[1],1);
    field->setDouble(-sign(S[2]) * S[2],2);

    field = obj->getField(QString("accel_bias"));
    field->setDouble(sign(S[0]) * b[0],0);
    field->setDouble(sign(S[1]) * b[1],1);
    field->setDouble(-sign(S[2]) * b[2],2);

    SixPointInConstFieldCal( 1000, mag_data_x, mag_data_y, mag_data_z, S, b);
    field = obj->getField(QString("mag_scale"));
    field->setDouble(sign(S[0]) * S[0],0);
    field->setDouble(sign(S[1]) * S[1],1);
    field->setDouble(sign(S[2]) * S[2],2);

    field = obj->getField(QString("mag_bias"));
    field->setDouble(sign(S[0]) * b[0], 0);
    field->setDouble(sign(S[1]) * b[1], 1);
    field->setDouble(sign(S[2]) * b[2], 2);

    // Go ahead and fix gyro scale here
    field = obj->getField(QString("gyro_bias"));
    double a = field->getDouble(0);
    field->setDouble(-sign(a) * a, 0);
    a = field->getDouble(1);
    field->setDouble(sign(a) * a, 1);
    a = field->getDouble(2);
    field->setDouble(-sign(a) * a, 2);

    field = obj->getField(QString("gyro_bias"));
    a = listMean(gyro_accum_x);
    field->setDouble(sign(a)*a,0);
    a = listMean(gyro_accum_y);
    field->setDouble(-sign(a)*a,1);
    a = listMean(gyro_accum_z);
    field->setDouble(sign(a)*a,2);

    obj->updated();

    position = -1; //set to run again
    m_ahrs->sixPointCalibInstructions->append("Computed accel and mag scale and bias...");

}

/**
  Six point calibration mode
  */
void ConfigAHRSWidget::sixPointCalibrationMode()
{
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSCalibration")));

    // set accels to unity gain
    UAVObjectField *field = obj->getField(QString("accel_scale"));
    field->setDouble(1,0);
    field->setDouble(1,1);
    field->setDouble(1,2);

    field = obj->getField(QString("accel_bias"));
    field->setDouble(0,0);
    field->setDouble(0,1);
    field->setDouble(0,2);

    field = obj->getField(QString("gyro_bias"));
    field->setDouble(0,0);
    field->setDouble(0,1);
    field->setDouble(0,2);

    field = obj->getField(QString("mag_scale"));
    field->setDouble(1,0);
    field->setDouble(1,1);
    field->setDouble(1,2);

    field = obj->getField(QString("mag_bias"));
    field->setDouble(0,0);
    field->setDouble(0,1);
    field->setDouble(0,2);

    obj->updated();

    gyro_accum_x.clear();
    gyro_accum_y.clear();
    gyro_accum_z.clear();

    /* Need to get as many AttitudeRaw updates as possible */
    obj = getObjectManager()->getObject(QString("AttitudeRaw"));
    initialMdata = obj->getMetadata();
    UAVObject::Metadata mdata = initialMdata;
    mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    mdata.flightTelemetryUpdatePeriod = 50;
    obj->setMetadata(mdata);

    /* Show instructions and enable controls */
    m_ahrs->sixPointCalibInstructions->clear();
    m_ahrs->sixPointCalibInstructions->append("Place horizontally and click save position...");
    displayPlane("plane-horizontal");
    m_ahrs->sixPointsStart->setEnabled(false);
    m_ahrs->sixPointsSave->setEnabled(true);
    position = 0;

}

/**
  Rotate the paper plane
  */
void ConfigAHRSWidget::displayPlane(QString elementID)
{
    paperplane->setElementId(elementID);
    m_ahrs->sixPointsHelp->setSceneRect(paperplane->boundingRect());
    m_ahrs->sixPointsHelp->fitInView(paperplane,Qt::KeepAspectRatio);

}


/**
  Draws the sensor variances bargraph
  */
void ConfigAHRSWidget::drawVariancesGraph()
{
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSCalibration")));
    // Now update size of all the graphs
    // I have not found a way to do this elegantly...
    UAVObjectField *field = obj->getField(QString("accel_var"));
    // The expected range is from 1E-6 to 1E-1
    double steps = 6; // 6 bars on the graph
    float accel_x_var = -1/steps*(1+steps+log10(field->getValue(0).toFloat()));
    accel_x->setTransform(QTransform::fromScale(1,accel_x_var),false);
    float accel_y_var = -1/steps*(1+steps+log10(field->getValue(1).toFloat()));
    accel_y->setTransform(QTransform::fromScale(1,accel_y_var),false);
    float accel_z_var = -1/steps*(1+steps+log10(field->getValue(2).toFloat()));
    accel_z->setTransform(QTransform::fromScale(1,accel_z_var),false);

    field = obj->getField(QString("gyro_var"));
    float gyro_x_var = -1/steps*(1+steps+log10(field->getValue(0).toFloat()));
    gyro_x->setTransform(QTransform::fromScale(1,gyro_x_var),false);
    float gyro_y_var = -1/steps*(1+steps+log10(field->getValue(1).toFloat()));
    gyro_y->setTransform(QTransform::fromScale(1,gyro_y_var),false);
    float gyro_z_var = -1/steps*(1+steps+log10(field->getValue(2).toFloat()));
    gyro_z->setTransform(QTransform::fromScale(1,gyro_z_var),false);

    field = obj->getField(QString("mag_var"));
    float mag_x_var = -1/steps*(1+steps+log10(field->getValue(0).toFloat()));
    mag_x->setTransform(QTransform::fromScale(1,mag_x_var),false);
    float mag_y_var = -1/steps*(1+steps+log10(field->getValue(1).toFloat()));
    mag_y->setTransform(QTransform::fromScale(1,mag_y_var),false);
    float mag_z_var = -1/steps*(1+steps+log10(field->getValue(2).toFloat()));
    mag_z->setTransform(QTransform::fromScale(1,mag_z_var),false);

}

/**
  Request current settings from the AHRS
  */
void ConfigAHRSWidget::ahrsSettingsRequest()
{

    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSSettings")));
    obj->requestUpdate();
    UAVObjectField *field = obj->getField(QString("Algorithm"));
    if (field)
        m_ahrs->algorithm->setCurrentIndex(m_ahrs->algorithm->findText(field->getValue().toString()));
    drawVariancesGraph();

    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("HomeLocation")));
    field = obj->getField(QString("Set"));
    if (field)
        m_ahrs->homeLocationSet->setEnabled(field->getValue().toBool());

    m_ahrs->ahrsCalibStart->setEnabled(true);
    m_ahrs->sixPointsStart->setEnabled(true);
    m_ahrs->calibInstructions->setText(QString("Press \"Start\" above to calibrate."));

}

/**
  Enables/disables the Home Location saving button depending on whether the
  home location is set-able
  */
void ConfigAHRSWidget::enableHomeLocSave(UAVObject * obj)
{
    UAVObjectField *field = obj->getField(QString("Set"));
    if (field) {
        m_ahrs->homeLocationSet->setEnabled(field->getValue().toBool());
    }
}


/**
  Save current settings to RAM
  */
void ConfigAHRSWidget::ahrsSettingsSaveRAM()
{
    UAVDataObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSSettings")));
    UAVObjectField *field = obj->getField(QString("Algorithm"));
    field->setValue(m_ahrs->algorithm->currentText());
    obj->updated();
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("HomeLocation")));
    field = obj->getField(QString("Set"));
    if (m_ahrs->homeLocationSet->isChecked())
        field->setValue(QString("TRUE"));
    else
        field->setValue(QString("FALSE"));
    obj->updated();

}

/**
Save AHRS Settings and home location to SD
  */
void ConfigAHRSWidget::ahrsSettingsSaveSD()
{
    ahrsSettingsSaveRAM();
    UAVDataObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("HomeLocation")));
    saveObjectToSD(obj);
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSSettings")));
    saveObjectToSD(obj);

}


/**
  @}
  @}
  */
