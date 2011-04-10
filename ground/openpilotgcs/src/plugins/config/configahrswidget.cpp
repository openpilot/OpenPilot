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
#include <QThread>
#include <iostream>
#include <Eigen/align-function.h>

#include "assertions.h"
#include "calibration.h"

#define sign(x) ((x < 0) ? -1 : 1)

const double ConfigAHRSWidget::maxVarValue = 0.1;
const int ConfigAHRSWidget::calibrationDelay = 7; // Time to wait for the AHRS to do its calibration

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

ConfigAHRSWidget::ConfigAHRSWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_ahrs = new Ui_AHRSWidget();
    m_ahrs->setupUi(this);

    collectingData = false;

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

    // Don't enable multi-point calibration until HomeLocation is set.
    m_ahrs->sixPointsStart->setEnabled(obj->getField("Set")->getValue().toBool());

    // Connect the signals
    connect(m_ahrs->ahrsCalibStart, SIGNAL(clicked()), this, SLOT(launchAHRSCalibration()));
    connect(m_ahrs->accelBiasStart, SIGNAL(clicked()), this, SLOT(launchAccelBiasCalibration()));
    connect(m_ahrs->ahrsSettingsRequest, SIGNAL(clicked()), this, SLOT(ahrsSettingsRequest()));
    /*
    connect(m_ahrs->algorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(ahrsSettingsSave()));
    connect(m_ahrs->indoorFlight, SIGNAL(stateChanged(int)), this, SLOT(homeLocationSave()));
    connect(m_ahrs->homeLocation, SIGNAL(clicked()), this, SLOT(homeLocationSaveSD()));
    */
    connect(m_ahrs->ahrsSettingsSaveRAM, SIGNAL(clicked()), this, SLOT(ahrsSettingsSaveRAM()));
    connect(m_ahrs->ahrsSettingsSaveSD, SIGNAL(clicked()), this, SLOT(ahrsSettingsSaveSD()));
    connect(m_ahrs->sixPointsStart, SIGNAL(clicked()), this, SLOT(multiPointCalibrationMode()));
    connect(m_ahrs->sixPointsSave, SIGNAL(clicked()), this, SLOT(savePositionData()));
    connect(m_ahrs->startDriftCalib, SIGNAL(clicked()),this, SLOT(launchGyroDriftCalibration()));
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

void ConfigAHRSWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    m_ahrs->ahrsBargraph->fitInView(ahrsbargraph, Qt::KeepAspectRatio);
    m_ahrs->sixPointsHelp->fitInView(paperplane,Qt::KeepAspectRatio);
}

/**
  Starts an accelerometer bias calibration.
  */
void ConfigAHRSWidget::launchAccelBiasCalibration()
{
    m_ahrs->accelBiasStart->setEnabled(false);
    m_ahrs->accelBiasProgress->setValue(0);

    // Setup the AHRS to give us the right data at the right rate:
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSSettings")));
    UAVObjectField* field = obj->getField(QString("BiasCorrectedRaw"));
    field->setValue("FALSE");
    obj->updated();

    accel_accum_x.clear();
    accel_accum_y.clear();
    accel_accum_z.clear();

    UAVDataObject* ahrsCalib = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSCalibration")));
//    ahrsCalib->getField("accel_bias")->setDouble(0,0);
//    ahrsCalib->getField("accel_bias")->setDouble(0,1);
//    ahrsCalib->getField("accel_bias")->setDouble(0,2);
//    ahrsCalib->updated();

    /* Need to get as many AttitudeRaw updates as possible */
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AttitudeRaw")));
    initialMdata = obj->getMetadata();
    UAVObject::Metadata mdata = initialMdata;
    mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    mdata.flightTelemetryUpdatePeriod = 100;
    obj->setMetadata(mdata);

    // Now connect to the attituderaw updates, gather for 100 samples
    collectingData = true;
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AttitudeRaw")));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(accelBiasattitudeRawUpdated(UAVObject*)));

}

/**
  Updates the accel bias raw values
  */
void ConfigAHRSWidget::accelBiasattitudeRawUpdated(UAVObject *obj)
{
	// TODO: THis one gets replaced with the multipoint calibratino below.
    UAVObjectField *accel_field = obj->getField(QString("accels"));
    Q_ASSERT(accel_field != 0);

    // This is necessary to prevent a race condition on disconnect signal and another update
    if (collectingData == true) {
        accel_accum_x.append(accel_field->getValue(0).toDouble());
        accel_accum_y.append(accel_field->getValue(1).toDouble());
        accel_accum_z.append(accel_field->getValue(2).toDouble());
    }

    m_ahrs->accelBiasProgress->setValue(m_ahrs->accelBiasProgress->value()+1);

    if(accel_accum_x.size() >= 100 && collectingData == true) {
        collectingData = false;
        disconnect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(accelBiasattitudeRawUpdated(UAVObject*)));
        m_ahrs->accelBiasStart->setEnabled(true);

        UAVDataObject* ahrsCalib = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSCalibration")));
        UAVObjectField* field = ahrsCalib->getField("accel_bias");
        double xBias = field->getDouble(0)- listMean(accel_accum_x);
        double yBias = field->getDouble(1) - listMean(accel_accum_y);
        double zBias = -9.81 + field->getDouble(2) - listMean(accel_accum_z);

        field->setDouble(xBias,0);
        field->setDouble(yBias,1);
        field->setDouble(zBias,2);

        ahrsCalib->updated();

        getObjectManager()->getObject(QString("AttitudeRaw"))->setMetadata(initialMdata);

        UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSSettings")));
        field = obj->getField(QString("BiasCorrectedRaw"));
        field->setValue("TRUE");
        obj->updated();

        saveAHRSCalibration();

    }

}


/**
  Starts a Gyro temperature drift calibration.
  */
void ConfigAHRSWidget::launchGyroDriftCalibration()
{
    if (!collectingData) {
        // m_ahrs->startDriftCalib->setEnabled(false);
        m_ahrs->startDriftCalib->setText("Stop");
        m_ahrs->accelBiasStart->setEnabled(false);
        m_ahrs->ahrsCalibStart->setEnabled(false);
        m_ahrs->sixPointsStart->setEnabled(false);

        // Setup the AHRS to give us the right data at the right rate:
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSSettings")));
        UAVObjectField* field = obj->getField(QString("BiasCorrectedRaw"));
        field->setValue("FALSE");
        obj->updated();

        /* Need to get as many AttitudeRaw updates as possible */
        obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AttitudeRaw")));
        initialMdata = obj->getMetadata();
        UAVObject::Metadata mdata = initialMdata;
        mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
        mdata.flightTelemetryUpdatePeriod = 100;
        obj->setMetadata(mdata);

        // Now connect to the attituderaw updates until we stop
        collectingData = true;
        obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("BaroAltitude")));
        field = obj->getField(QString("Temperature"));
        double temp = field->getValue().toDouble();
        m_ahrs->gyroTempSlider->setRange(temp*10,temp*10);
        m_ahrs->gyroMaxTemp->setText(QString::number(temp,'g',3));
        m_ahrs->gyroMinTemp->setText(QString::number(temp,'g',3));

        connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(driftCalibrationAttitudeRawUpdated(UAVObject*)));
    } else {
        // Stop all the gathering:
        collectingData = false;
        m_ahrs->startDriftCalib->setText("Start");
        m_ahrs->accelBiasStart->setEnabled(true);
        m_ahrs->ahrsCalibStart->setEnabled(true);
        m_ahrs->sixPointsStart->setEnabled(true);

        UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AttitudeRaw")));
        disconnect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(driftCalibrationAttitudeRawUpdated(UAVObject*)));

        getObjectManager()->getObject(QString("AttitudeRaw"))->setMetadata(initialMdata);

        obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSSettings")));
        UAVObjectField* field = obj->getField(QString("BiasCorrectedRaw"));
        field->setValue("TRUE");
        obj->updated();

        // TODO: Now compute the drift here
        computeGyroDrift();

    }
}

/**
  Updates the gyro drift calibration values in real time
  */
void ConfigAHRSWidget::driftCalibrationAttitudeRawUpdated(UAVObject* obj) {

    // This is necessary to prevent a race condition on disconnect signal and another update
    if (collectingData == true) {
        /**
          First of all, update the temperature user feedback
          This is not what we will use for our calculations, but it it easier for the
          user to have the real temperature rather than an obscure unit...
          */
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("BaroAltitude")));
        UAVObjectField *tempField = obj->getField(QString("Temperature"));
        Q_ASSERT(tempField != 0);
        double mbTemp = tempField->getValue().toDouble();
        if (mbTemp*10 < m_ahrs->gyroTempSlider->minimum()) {
            m_ahrs->gyroTempSlider->setMinimum(mbTemp*10);
            m_ahrs->gyroMinTemp->setText(QString::number(mbTemp,'g',3));
        } else if (mbTemp*10 > m_ahrs->gyroTempSlider->maximum()) {
             m_ahrs->gyroTempSlider->setMaximum(mbTemp*10);
             m_ahrs->gyroMaxTemp->setText(QString::number(mbTemp,'g',3));
        }
        m_ahrs->gyroTempSlider->setValue(mbTemp*10);
        // TODO:
        // - Add an indicator to show that we have a significant
        //   temperature difference in our gathered data (red/yellow/green)

        /**
          Now, append gyro values + gyro temp data into our buffers
          */
        // TODO:
        // - choose a storage type for this data
        // - Check it's not getting too big
        // - do the actual appending
        // - That's it, really...


    }
}

/**
  Computes gyro drift based on sampled data
  */
void ConfigAHRSWidget::computeGyroDrift() {
    // TODO

    // TODO: if this is not too computing-intensive, we could consider
    // calling this with a timer when data sampling is enabled, to get
    // a real-time view of the computed drift convergence and let the
    // user stop sampling when it becomes stable enough...
    //
    // Hint for whoever wants to implement that:
    // The formula I use for computing the temperature compensation factor from
    // two nicely filtered (downsampled) sample points is as follows:
    //
    // gyro_tempcompfactor == -(raw_gyro1 - raw_gyro2)/(gyro_temp1 - gyro_temp2)
    //
    // where raw_gyro1 and raw_gyro2 are gyroscope raw measurement values and
    // gyro_temp1 and gyro_temp2 are the measurements from the gyroscope internal
    // temperature sensors, each at two measure points T1 and T2
    // note that the X and Y gyroscopes share one temperature sensor while
    // Z has its own.
    //
    // the formula that calculates the AttitudeRav.gyros[X,Y,Z] values is
    // currently as follows:
    //
    // gyro = 180/Pi * ( ( ( raw_gyro + raw_gyro * gyro_tempcompfactor ) * gyro_scale) + gyro_bias )
    //
    // so to get gyro_raw do the following:
    // 1. set AHRSSettings.BiasCorrectedRaw to FALSE before measuring! (already done right now)
    // 2. set AHRSCalibration.gyro_tempcompfactor to 0 before measuring!
    // 3. gyro_raw = ( ( gyro * Pi / 180 ) - gyro_bias ) / gyro_scale
    //
    // a nice trick is to set gyro_bias to 0 and gyro_scale to (Pi / 180) in which case gyro = raw_gyro
    // note that Pi/180 is very close to the "real" scale of the AHRS gyros anyway (though with switched signs)

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
//    UAVObjectField *field = obj->getField(QString("measure_var"));

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

FORCE_ALIGN_FUNC
void ConfigAHRSWidget::attitudeRawUpdated(UAVObject * obj)
{
    QMutexLocker lock(&attitudeRawUpdateLock);

    UAVObjectField *accel_field = obj->getField(QString("accels"));
    UAVObjectField *gyro_field = obj->getField(QString("gyros"));
    UAVObjectField *mag_field = obj->getField(QString("magnetometers"));

    Q_ASSERT(gyro_field != 0 && accel_field != 0 && mag_field != 0);

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

    if(accel_accum_x.size() >= 8 && collectingData == true) {
        collectingData = false;
        disconnect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(attitudeRawUpdated(UAVObject*)));
        m_ahrs->sixPointsSave->setEnabled(true);

        accel_data[position] << listMean(accel_accum_x),
			listMean(accel_accum_y),
			listMean(accel_accum_z);

        mag_data[position] << listMean(mag_accum_x),
			listMean(mag_accum_y),
			listMean(mag_accum_z);

        gyro_data[position] << listMean(gyro_accum_x),
			listMean(gyro_accum_y),
			listMean(gyro_accum_z);


        std::cout << "observed accel: " << accel_data[position].transpose()
			<< "\nobserved mag: " << mag_data[position].transpose()
			<< "\nobserved gyro: " << gyro_data[position].transpose()
			<< std::endl;

        struct {
        	const char* instructions;
        	const char* display;
        } instructions[] = {
        		{ "Pitch up 45 deg and click save position...", "plane-horizontal" },
        		{ "Pitch down 45 deg and click save position...", "plane-horizontal" },
        		{ "Roll left 45 deg and click save position...", "plane-left" },
        		{ "Roll right 45 deg and click save position...", "plane-left" },

        		{ "Turn left 90 deg to 09:00 position and click save position...", "plane-horizontal" },
        		{ "Pitch up 45 deg and click save position...", "plane-horizontal" },
        		{ "Pitch down 45 deg and click save position...", "plane-horizontal" },
        		{ "Roll left 45 deg and click save position...", "plane-left" },
        		{ "Roll right 45 deg and click save position...", "plane-left" },

        		{ "Turn left 90 deg to 06:00 position and click save position...", "plane-horizontal" },
        		{ "Pitch up 45 deg and click save position...", "plane-horizontal" },
        		{ "Pitch down 45 deg and click save position...", "plane-horizontal" },
        		{ "Roll left 45 deg and click save position...", "plane-left" },
        		{ "Roll right 45 deg and click save position...", "plane-left" },

        		{ "Turn left 90 deg to 03:00 position and click save position...", "plane-horizontal" },
        		{ "Pitch up 45 deg and click save position...", "plane-horizontal" },
        		{ "Pitch down 45 deg and click save position...", "plane-horizontal" },
        		{ "Roll left 45 deg and click save position...", "plane-left" },
        		{ "Roll right 45 deg and click save position...", "plane-left" },

        		{ "Place with nose vertically up and click save position...", "plane-up" },
        		{ "Place with nose straight down and click save position...", "plane-down" },
        		{ "Place upside down and click save position...", "plane-flip" },
        };

        n_positions = sizeof(instructions) / sizeof(instructions[0]);
        position = (position + 1) % n_positions;

        if (position != 0 && position < n_positions) {

        	m_ahrs->sixPointCalibInstructions->append(instructions[position-1].instructions);
        	displayPlane(instructions[position-1].display);
        }
        else if(position == 0) {
        	position = n_positions;
            computeScaleBias();
            m_ahrs->sixPointsStart->setEnabled(true);
            m_ahrs->sixPointsSave->setEnabled(false);
            saveAHRSCalibration(); // Saves the result to SD.

            /* Cleanup original settings */
            getObjectManager()->getObject(QString("AttitudeRaw"))->setMetadata(initialMdata);
        }
    }
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
    gyro_accum_x.clear();
    gyro_accum_y.clear();
    gyro_accum_z.clear();

    collectingData = true;
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AttitudeRaw")));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(attitudeRawUpdated(UAVObject*)));

    m_ahrs->sixPointCalibInstructions->append("Hold...");
}

//*****************************************************************
namespace {

/*
 * Calibrated scale factors should be real values with scale factor less than 10% from nominal
 */
bool checkScaleFactors(const Vector3f& scalars)
{
	return isReal(scalars) &&
		scalars.cwise().abs().maxCoeff() < 1.10f;
}

/*
 * Calibrated offsets should be real values.  TODO: Add range checks
 */
bool checkOffsets(const Vector3f& offsets)
{
	return isReal(offsets);
}

/**
 * Given a UAVObjectField that is a 3-tuple, produce an Eigen::Vector3f
 * from it.
 */
Vector3f
tupleToVector(UAVObjectField *tuple)
{
	return (Vector3f() << tuple->getDouble(0),
			tuple->getDouble(1),
			tuple->getDouble(2)).finished();
}

/**
 * Convert a 3-vector to a 3-tuple
 * @param v A 3-vector
 * @param tuple[in] Assign the elements of this three-tuple to the elements of v
 */
void
vectorToTuple(UAVObjectField *tuple, const Vector3f& v)
{
	for (int i = 0; i < 3; ++i) {
		tuple->setDouble(v(i), i);
	}
}

/**
 * Updates the offsets for a calibrated gyro field.
 * @param scale[in] Non-null pointer to a 3-element scale factor field.
 * @param bias[out] Non-null pointer to a 3-element bias field.
 * @param updateBias the source bias matrix.
 */
void
updateBias(UAVObjectField *scale,
		UAVObjectField *bias ,
		const Vector3f& updateBias)
{
	Vector3f scale_factor = (Vector3f() << scale->getDouble(0),
		scale->getDouble(1),
		scale->getDouble(2)).finished();
	Vector3f old_bias = (Vector3f() << bias->getDouble(0),
		bias->getDouble(1),
		bias->getDouble(2)).finished();

	// Convert to radians/second
	Vector3f final_bias = -(M_PI)/180.0f * updateBias + old_bias;

    bias->setDouble(final_bias(0), 0);
    bias->setDouble(final_bias(1), 1);
    bias->setDouble(final_bias(2), 2);
}

void
updateRotation(UAVObjectField *rotation, const Vector3f& updateRotation)
{
	for (int i = 0; i < 3; ++i) {
		rotation->setDouble(updateRotation[i], i);
	}
}

} // !namespace (anon)


/**
 * Updates the scale factors and offsets for a calibrated vector field.
 * @param scale[out] Non-null pointer to a 3-element scale factor field.
 * @param bias[out] Non-null pointer to a 3-element bias field.
 * @param ortho[out] Optional pointer to a 3-element orthogonal correction field
 * @param updateScale the source scale factor matrix.
 * @param updateBias the source bias matrix.
 * @param oldScale The original sensor scale factor
 * @param oldBias The original bias value
 * @param oldOrtho Optional.  The original orthogonality scale factor value.
 * @return true if successful, false otherwise.
 */
bool
ConfigAHRSWidget::updateScaleFactors(UAVObjectField *scale,
		UAVObjectField *bias ,
		UAVObjectField *ortho,
		const Matrix3f& updateScale,
		const Vector3f& updateBias,
		const Vector3f& oldScale,
		const Vector3f& oldBias,
		const Vector3f& oldOrtho)
{
	// Compose a 4x4 affine transformation matrix composed of the scale factor,
	// orthogonality correction, and bias.
	Matrix4f calibration;
	calibration << tupleToVector(scale).asDiagonal(),
		tupleToVector(bias),
		Vector4f::UnitW().transpose();

	if (ortho) {
		Vector3f orthof = tupleToVector(ortho);
		calibration(1, 0) = calibration(0, 1) = orthof(0);
		calibration(2, 0) = calibration(0, 2) = orthof(1);
		calibration(1, 2) = calibration(2, 1) = orthof(2);
	}

	std::cout << "old calibration matrix: \n" << calibration << "\n";

	Matrix4f update;
	update << updateScale, updateBias, Vector4f::UnitW().transpose();
	std::cout << "new calibration matrix update: \n" << update << "\n";

	calibration = update * calibration;

	if (checkOffsets(updateBias) && checkScaleFactors(updateScale.diagonal())) {
		// Apply the new calibration
		vectorToTuple(scale, calibration.diagonal().start<3>());
		vectorToTuple(bias, calibration.col(3).start<3>());
	
		if (ortho) {
			ortho->setDouble(calibration(0, 1), 0);
			ortho->setDouble(calibration(0, 2), 1);
			ortho->setDouble(calibration(1, 2), 2);
		}
		return true;
    }
    else {
    	// Give the user the calibration data and restore their settings.
		std::ostringstream msg;
		msg << "Scale factors and/or offsets are out of range.\n";
		msg << "Please see the troubleshooting section of the manual and retry.\n\n"
			"The following values were computed:\n";
		msg << qPrintable(scale->getName()) << ": "
			<< calibration.diagonal().start<3>().transpose() << "\n";
		vectorToTuple(scale, oldScale);

		if (ortho) {
			msg << qPrintable(ortho->getName()) << ": "
				<< calibration(0,1) << ", " << calibration(0,2) << ", " << calibration(1,2) << "\n";
			vectorToTuple(ortho, oldOrtho);
		}

		msg << qPrintable(bias->getName()) << ": "
			<< calibration.col(3).start<3>().transpose() << "\n";
		vectorToTuple(bias, oldBias);

		m_ahrs->sixPointCalibInstructions->append(msg.str().c_str());
		return false;
    }
}

FORCE_ALIGN_FUNC
void ConfigAHRSWidget::computeScaleBias()
{
    // Extract the local magnetic and gravitational field vectors from HomeLocation.
    UAVObject *home = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("HomeLocation")));
    Vector3f localMagField;
    localMagField << home->getField("Be")->getValue(0).toDouble(),
		home->getField("Be")->getValue(1).toDouble(),
		home->getField("Be")->getValue(2).toDouble();

    float localGravity = home->getField("g_e")->getDouble();

    Vector3f referenceField = Vector3f::UnitZ()*localGravity;
    double noise = 0.04;
    Vector3f accelBias;
    Matrix3f accelScale;
    std::cout << "number of samples: " << n_positions << "\n";
    twostep_bias_scale(accelBias, accelScale, accel_data, n_positions, referenceField, noise*noise);
    // Twostep computes an offset from the identity scalar, and a negative bias offset
    accelScale += Matrix3f::Identity();
    accelBias = -accelBias;
    std::cout << "computed accel bias: " << accelBias.transpose()
		<< "\ncomputed accel scale:\n" << accelScale<< std::endl;

    // Apply the computed scale factor and bias to each sample
    for (int i = 0; i < n_positions; ++i) {
    	accel_data[i] = accelScale * accel_data[i] + accelBias;
    }

    // Magnetometer has excellent orthogonality, so only calibrate the scale factors.
    Vector3f magBias;
    Vector3f magScale;
    noise = 4.0;
    twostep_bias_scale(magBias, magScale, mag_data, n_positions, localMagField, noise*noise);
    magScale += Vector3f::Ones();
    magBias = -magBias;
    std::cout << "computed mag bias: " << magBias.transpose()
		<< "\ncomputed mag scale:\n" << magScale << std::endl;

    // Apply the computed scale factor and bias to each sample
    for (int i = 0; i < n_positions; ++i) {
    	mag_data[i] = magScale.asDiagonal() * mag_data[i] + magBias;
    }

    // Calibrate gyro bias and acceleration sensitivity
    Matrix3f accelSensitivity;
    Vector3f gyroBias;
    gyroscope_calibration(gyroBias, accelSensitivity, gyro_data, accel_data, n_positions);
    std::cout << "gyro bias: " << gyroBias.transpose()
		<< "\ngyro's acceleration sensitivity:\n" << accelSensitivity << std::endl;

    // Calibrate alignment between the accelerometer and magnetometer, taking the mag as the
    // reference.
    Vector3f accelRotation;
    calibration_misalignment(accelRotation, accel_data, -Vector3f::UnitZ()*localGravity,
			mag_data, localMagField, n_positions);
    std::cout << "magnetometer rotation vector: " << accelRotation.transpose() << std::endl;

    // Update the calibration scalars with a clear message box
    m_ahrs->sixPointCalibInstructions->clear();
	UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSCalibration")));


    bool success = updateScaleFactors(obj->getField(QString("accel_scale")),
			obj->getField(QString("accel_bias")),
			obj->getField(QString("accel_ortho")),
			accelScale,
			accelBias,
			saved_accel_scale,
			saved_accel_bias,
			saved_accel_ortho);

    success &= updateScaleFactors(obj->getField(QString("mag_scale")),
    		obj->getField(QString("mag_bias")),
    		NULL,
    		magScale.asDiagonal(),
    		magBias,
    		saved_mag_scale,
    		saved_mag_bias);

	updateBias(obj->getField(QString("gyro_scale")),
		obj->getField(QString("gyro_bias")),
		gyroBias);

#if 0
	// TODO: Enable after v1.0 feature freeze is lifted.
	updateRotation(obj->getField(QString("accel_rotation")), accelRotation);
#endif

    obj->updated();

    position = -1; //set to run again
	if (success)
		m_ahrs->sixPointCalibInstructions->append("Computed new accel and mag scale and bias.");

}

/**
  Multi-point calibration mode
  */
FORCE_ALIGN_FUNC
void ConfigAHRSWidget::multiPointCalibrationMode()
{
	cacheCurrentCalibration();
	resetCalibrationDefaults();

	UAVObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSSettings")));
    UAVObjectField *field = obj->getField(QString("BiasCorrectedRaw"));
    field->setValue("FALSE");
    obj->updated();


    Thread::usleep(100000);

    gyro_accum_x.clear();
    gyro_accum_y.clear();
    gyro_accum_z.clear();

    /* Need to get as many AttitudeRaw updates as possible */
    obj = getObjectManager()->getObject(QString("AttitudeRaw"));
    initialMdata = obj->getMetadata();
    UAVObject::Metadata mdata = initialMdata;
    mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    mdata.flightTelemetryUpdatePeriod = 100;
    obj->setMetadata(mdata);

    /* Show instructions and enable controls */
    m_ahrs->sixPointCalibInstructions->clear();
    m_ahrs->sixPointCalibInstructions->append("Stand facing Earth's magnetic N or S.  Place the vehicle horizontally facing forward and click save position...");
    displayPlane("plane-horizontal");
    m_ahrs->sixPointsStart->setEnabled(false);
    m_ahrs->sixPointsSave->setEnabled(true);
    position = 0;

}

/**
 * Read the current calibration scalars and offsets from the target board, and
 * save them for later use. In the event of a calibration failure, if the
 * calibration method began by resetting calibration values, they may be
 * restored later with this information.
 */
void ConfigAHRSWidget::cacheCurrentCalibration()
{
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSCalibration")));
    struct field_t {
    	const char* field_name;
    	Vector3f& cache;
    } fields[] = {
    		{ "accel_scale", this->saved_accel_scale },
    		{ "accel_bias", this->saved_accel_bias },
    		{ "accel_ortho", this->saved_accel_ortho },
// TODO: Enable after V1.0 feature freeze is lifted.
//    		{ "accel_rotation", this->saved_accel_rotation },
    		{ "gyro_bias", this->saved_gyro_bias },
    		{ "mag_scale", this->saved_mag_scale },
    		{ "mag_bias", this->saved_mag_bias },
    		{ NULL, this->saved_mag_bias }, // sentinnel
    };
    for (field_t* i = fields; i->field_name != NULL; ++i) {
    	UAVObjectField* field = obj->getField(QString(i->field_name));
    	if (field) {
    		i->cache = tupleToVector(field);
    	}
    	else {
    		qDebug() << "WARNING: AHRSCalibration field not found: " << i->field_name << "\n";
    	}
    }
}

/**
 * Reset all calibration scalars to their default values.
 */
void ConfigAHRSWidget::resetCalibrationDefaults()
{
    UAVObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AHRSCalibration")));

    // set accels to unity gain
    UAVObjectField *field = obj->getField(QString("accel_scale"));
    // TODO: Figure out how to load these directly from the saved metadata
    // about default values
    field->setDouble(0.0359, 0);
    field->setDouble(0.0359, 1);
    field->setDouble(0.0359, 2);

    field = obj->getField(QString("accel_bias"));
    field->setDouble(-73.5, 0);
    field->setDouble(-73.5, 1);
    field->setDouble(73.5, 2);

    field = obj->getField(QString("accel_ortho"));
    for (int i = 0; i < 3; ++i) {
    	field->setDouble(0, i);
    }

    field = obj->getField(QString("gyro_bias"));
    UAVObjectField *field2 = obj->getField(QString("gyro_scale"));
    field->setDouble(28/-0.017*field2->getDouble(0),0);
    field->setDouble(-28/0.017*field2->getDouble(1),1);
    field->setDouble(28/-0.017*field2->getDouble(2),2);

    field = obj->getField(QString("mag_scale"));
    for (int i = 0; i < 3; ++i) {
    	field->setDouble(-1, i);
    }

    field = obj->getField(QString("mag_bias"));
    for (int i = 0; i < 3; ++i) {
    	field->setDouble(0, i);
    }

#if 0
	// TODO: Enable after v1.0 feature freeze is lifted.
    field = obj->getField(QString("accel_rotation"));
    for (int i = 0; i < 3; ++i) {
    	field->setDouble(0, i);
    }
#endif
    obj->updated();
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

    // Scale by 1e-3 because mag vars are much higher.
    // TODO: Really?  This is the scale factor from mG to T
    field = obj->getField(QString("mag_var"));
    float mag_x_var = -1/steps*(1+steps+log10(1e-3*field->getValue(0).toFloat()));
    mag_x->setTransform(QTransform::fromScale(1,mag_x_var),false);
    float mag_y_var = -1/steps*(1+steps+log10(1e-3*field->getValue(1).toFloat()));
    mag_y->setTransform(QTransform::fromScale(1,mag_y_var),false);
    float mag_z_var = -1/steps*(1+steps+log10(1e-3*field->getValue(2).toFloat()));
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
    m_ahrs->accelBiasStart->setEnabled(true);
    m_ahrs->startDriftCalib->setEnabled(true);

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
        m_ahrs->sixPointsStart->setEnabled(obj->getField("Set")->getValue().toBool());
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
