/**
 ******************************************************************************
 *
 * @file       configairframewidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Airframe configuration panel
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
#include "configairframewidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>



ConfigAirframeWidget::ConfigAirframeWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_aircraft = new Ui_AircraftWidget();
    m_aircraft->setupUi(this);

    mixerTypes << "Mixer0Type" << "Mixer1Type" << "Mixer2Type" << "Mixer3Type"
            << "Mixer4Type" << "Mixer5Type" << "Mixer6Type" << "Mixer7Type";
    mixerVectors << "Mixer0Vector" << "Mixer1Vector" << "Mixer2Vector" << "Mixer3Vector"
            << "Mixer4Vector" << "Mixer5Vector" << "Mixer6Vector" << "Mixer7Vector";

    QStringList airframeTypes;
    airframeTypes << "Fixed Wing" << "Multirotor" << "Helicopter" << "Custom";
    m_aircraft->aircraftType->addItems(airframeTypes);
    m_aircraft->aircraftType->setCurrentIndex(1);

    QStringList fixedWingTypes;
    fixedWingTypes << "Elevator aileron rudder" << "Elevon" << "Vtail";
    m_aircraft->fixedWingType->addItems(fixedWingTypes);

    QStringList multiRotorTypes;
    multiRotorTypes << "Quad +" << "Quad X" << "Hexacopter" << "Octocopter";
    m_aircraft->multirotorFrameType->addItems(multiRotorTypes);



    QStringList channels;
    channels << "None" << "Channel0" << "Channel1" << "Channel2" <<
            "Channel3" << "Channel4" << "Channel5" << "Channel6" << "Channel7";
    // Now load all the channel assignements for fixed wing
    m_aircraft->fwElevator1Channel->addItems(channels);
    m_aircraft->fwElevator2Channel->addItems(channels);
    m_aircraft->fwEngineChannel->addItems(channels);
    m_aircraft->fwRudderChannel->addItems(channels);
    m_aircraft->fwAileron1Channel->addItems(channels);
    m_aircraft->fwAileron2Channel->addItems(channels);
    m_aircraft->multiMotor1->addItems(channels);
    m_aircraft->multiMotor2->addItems(channels);
    m_aircraft->multiMotor3->addItems(channels);
    m_aircraft->multiMotor4->addItems(channels);
    m_aircraft->multiMotor5->addItems(channels);
    m_aircraft->multiMotor6->addItems(channels);
    m_aircraft->multiMotor7->addItems(channels);
    m_aircraft->multiMotor8->addItems(channels);

// Setup the Multirotor picture in the Quad settings interface
    m_aircraft->quadShape->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_aircraft->quadShape->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/configgadget/images/quad-shapes.svg"));
    quad = new QGraphicsSvgItem();
    quad->setSharedRenderer(renderer);
    quad->setElementId("quad-plus");
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addItem(quad);
    scene->setSceneRect(quad->boundingRect());
    m_aircraft->quadShape->setScene(scene);
    

    connect(m_aircraft->saveAircraftToSD, SIGNAL(clicked()), this, SLOT(saveAircraftUpdate()));
    connect(m_aircraft->saveAircraftToRAM, SIGNAL(clicked()), this, SLOT(sendAircraftUpdate()));
    connect(m_aircraft->getAircraftCurrent, SIGNAL(clicked()), this, SLOT(requestAircraftUpdate()));
    connect(m_aircraft->fixedWingType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupAirframeUI(QString)));
    connect(m_aircraft->multirotorFrameType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupAirframeUI(QString)));
    connect(m_aircraft->aircraftType, SIGNAL(currentIndexChanged(int)), this, SLOT(switchAirframeType(int)));
    requestAircraftUpdate();

    connect(m_aircraft->fwThrottleReset, SIGNAL(clicked()), this, SLOT(resetFwMixer()));
    connect(m_aircraft->mrThrottleCurveReset, SIGNAL(clicked()), this, SLOT(resetMrMixer()));
    connect(m_aircraft->fixedWingThrottle, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateFwThrottleCurveValue(QList<double>,double)));
    connect(m_aircraft->multiThrottleCurve, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateMrThrottleCurveValue(QList<double>,double)));

//    connect(m_aircraft->fwAileron1Channel, SIGNAL(currentIndexChanged(int)), this, SLOT(toggleAileron2(int)));
//    connect(m_aircraft->fwElevator1Channel, SIGNAL(currentIndexChanged(int)), this, SLOT(toggleElevator2(int)));

    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(requestAircraftUpdate()));

}

ConfigAirframeWidget::~ConfigAirframeWidget()
{
   // Do nothing
}

/**
  Slot for switching the airframe type. We do it explicitely
  rather than a signal in the UI, because we want to force a fitInView of the quad shapes.
  This is because this method (fitinview) only works when the widget is shown.
  */
void ConfigAirframeWidget::switchAirframeType(int index){
    m_aircraft->airframesWidget->setCurrentIndex(index);
    m_aircraft->quadShape->setSceneRect(quad->boundingRect());
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);


}

void ConfigAirframeWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
}

void ConfigAirframeWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);

}


void ConfigAirframeWidget::toggleAileron2(int index)
{
    if (index) {
        m_aircraft->fwAileron2Channel->setEnabled(true);
        m_aircraft->fwAileron2Label->setEnabled(true);
    } else {
        m_aircraft->fwAileron2Channel->setEnabled(false);
        m_aircraft->fwAileron2Label->setEnabled(false);
    }
}

void ConfigAirframeWidget::toggleElevator2(int index)
{
    if (index) {
        m_aircraft->fwElevator2Channel->setEnabled(true);
        m_aircraft->fwElevator2Label->setEnabled(true);
    } else {
        m_aircraft->fwElevator2Channel->setEnabled(false);
        m_aircraft->fwElevator2Label->setEnabled(false);
    }
}

/**
  Resets Fixed wing throttle mixer
  */
void ConfigAirframeWidget::resetFwMixer()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve1"));
    resetMixer(m_aircraft->fixedWingThrottle, field->getNumElements());
}

/**
  Resets Multirotor throttle mixer
  */
void ConfigAirframeWidget::resetMrMixer()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve1"));
    resetMixer(m_aircraft->multiThrottleCurve, field->getNumElements());
}

/**
  Resets a mixer curve
  */
void ConfigAirframeWidget::resetMixer(MixerCurveWidget *mixer, int numElements)
{
    QList<double> curveValues;
    for (double i=0; i<numElements; i++) {
        curveValues.append(i/(numElements-1));
    }
    // Setup all Throttle1 curves for all types of airframes
    mixer->initCurve(curveValues);
}

/**
  Updates the currently moved throttle curve item value
  */
void ConfigAirframeWidget::updateFwThrottleCurveValue(QList<double> list, double value)
{
    Q_UNUSED(list);
    m_aircraft->fwThrottleCurveItemValue->setText(QString().sprintf("Val: %.2f",value));
}

/**
  Updates the currently moved throttle curve item value
  */
void ConfigAirframeWidget::updateMrThrottleCurveValue(QList<double> list, double value)
{
    Q_UNUSED(list);
    m_aircraft->mrThrottleCurveItemValue->setText(QString().sprintf("Val: %.2f",value));
}


/**************************
  * Aircraft settings
  **************************/
/**
  Request the current value of the SystemSettings which holds the aircraft type
  */
void ConfigAirframeWidget::requestAircraftUpdate()
{
    // Get the Airframe type from the system settings:
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("SystemSettings")));
    Q_ASSERT(obj);
    obj->requestUpdate();
    UAVObjectField *field = obj->getField(QString("AirframeType"));
    Q_ASSERT(field);
    // At this stage, we will need to have some hardcoded settings in this code, this
    // is not ideal, but here you go.
    QString frameType = field->getValue().toString();
    setupAirframeUI(frameType);

    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    obj->requestUpdate();
    field = obj->getField(QString("ThrottleCurve1"));
    Q_ASSERT(field);
    QList<double> curveValues;
    // If the 1st element of the curve is <= -10, then the curve
    // is a straight line (that's how the mixer works on the mainboard):
    if (field->getValue(0).toInt() <= -10) {
        for (double i=0; i<field->getNumElements(); i++) {
            curveValues.append(i/(field->getNumElements()-1));
        }
    } else {
        for (unsigned int i=0; i < field->getNumElements(); i++) {
            curveValues.append(field->getValue(i).toDouble());
        }
    }
    // Setup all Throttle1 curves for all types of airframes
    m_aircraft->fixedWingThrottle->initCurve(curveValues);
    m_aircraft->multiThrottleCurve->initCurve(curveValues);

    // Load the Settings for fixed wing frames:
    if (frameType.startsWith("FixedWing")) {
         // Then retrieve how channels are setup
        obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
        Q_ASSERT(obj);
        field = obj->getField(QString("FixedWingThrottle"));
        Q_ASSERT(field);
        m_aircraft->fwEngineChannel->setCurrentIndex(m_aircraft->fwEngineChannel->findText(field->getValue().toString()));
        field = obj->getField(QString("FixedWingRoll1"));
        Q_ASSERT(field);
        m_aircraft->fwAileron1Channel->setCurrentIndex(m_aircraft->fwAileron1Channel->findText(field->getValue().toString()));
        field = obj->getField(QString("FixedWingRoll2"));
        Q_ASSERT(field);
        m_aircraft->fwAileron2Channel->setCurrentIndex(m_aircraft->fwAileron2Channel->findText(field->getValue().toString()));
        field = obj->getField(QString("FixedWingPitch1"));
        Q_ASSERT(field);
        m_aircraft->fwElevator1Channel->setCurrentIndex(m_aircraft->fwElevator1Channel->findText(field->getValue().toString()));
        field = obj->getField(QString("FixedWingPitch2"));
        Q_ASSERT(field);
        m_aircraft->fwElevator2Channel->setCurrentIndex(m_aircraft->fwElevator2Channel->findText(field->getValue().toString()));
        field = obj->getField(QString("FixedWingYaw"));
        Q_ASSERT(field);
        m_aircraft->fwRudderChannel->setCurrentIndex(m_aircraft->fwRudderChannel->findText(field->getValue().toString()));

        if (frameType == "FixedWingElevon") {
        // If the airframe is elevon, restore the slider setting
            // Find the channel number for Elevon1 (FixedWingRoll1)
            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            Q_ASSERT(obj);
            int chMixerNumber = m_aircraft->fwAileron1Channel->currentIndex()-1;
            if (chMixerNumber >= 0) { // If for some reason the actuators were incoherent, we might fail here, hence the check.
                field = obj->getField(mixerVectors.at(chMixerNumber));
                int ti = field->getElementNames().indexOf("Roll");
                m_aircraft->elevonSlider1->setValue(field->getDouble(ti)*100);
                ti = field->getElementNames().indexOf("Pitch");
                m_aircraft->elevonSlider2->setValue(field->getDouble(ti)*100);
            }
        }
        if (frameType == "FixedWingVtail") {
            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            Q_ASSERT(obj);
            int chMixerNumber = m_aircraft->fwElevator1Channel->currentIndex()-1;
            if (chMixerNumber >=0) {
                field = obj->getField(mixerVectors.at(chMixerNumber));
                int ti = field->getElementNames().indexOf("Yaw");
                m_aircraft->elevonSlider1->setValue(field->getDouble(ti)*100);
                ti = field->getElementNames().indexOf("Pitch");
                m_aircraft->elevonSlider2->setValue(field->getDouble(ti)*100);
            }
        }

    } else if (frameType == "QuadX" || frameType == "QuadP" ||
               frameType == "Hexa" || frameType == "Octo" ) {
        //////////////////////////////////////////////////////////////////
        // Retrieve Multirotor settings
        //////////////////////////////////////////////////////////////////

        obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
        Q_ASSERT(obj);
        if (frameType == "QuadP") {
            // Motors 1/2/3/4 are: N / E / S / W
            field = obj->getField(QString("VTOLMotorN"));
            Q_ASSERT(field);
            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorE"));
            Q_ASSERT(field);
            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorS"));
            Q_ASSERT(field);
            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorW"));
            Q_ASSERT(field);
            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
        } else if (frameType == "QuadX") {
            // Motors 1/2/3/4 are: NW / NE / SE / SW
            field = obj->getField(QString("VTOLMotorNW"));
            Q_ASSERT(field);
            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorNE"));
            Q_ASSERT(field);
            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorSE"));
            Q_ASSERT(field);
            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorSW"));
            Q_ASSERT(field);
            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));

        } else if (frameType == "Hexa") {
            // Motors 1/2/3 4/5/6 are: NW / N / NE and SE / S / SW
            field = obj->getField(QString("VTOLMotorNW"));
            Q_ASSERT(field);
            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorN"));
            Q_ASSERT(field);
            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorNE"));
            Q_ASSERT(field);
            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorSE"));
            Q_ASSERT(field);
            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorS"));
            Q_ASSERT(field);
            m_aircraft->multiMotor5->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorSW"));
            Q_ASSERT(field);
            m_aircraft->multiMotor6->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
        } else if (frameType == "Octo") {
            // Motors 1 to 8 are N / NE / E / etc
            field = obj->getField(QString("VTOLMotorN"));
            Q_ASSERT(field);
            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorNE"));
            Q_ASSERT(field);
            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorE"));
            Q_ASSERT(field);
            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorSE"));
            Q_ASSERT(field);
            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorS"));
            Q_ASSERT(field);
            m_aircraft->multiMotor5->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorSW"));
            Q_ASSERT(field);
            m_aircraft->multiMotor6->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorW"));
            Q_ASSERT(field);
            m_aircraft->multiMotor7->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorNW"));
            Q_ASSERT(field);
            m_aircraft->multiMotor8->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
        }

        obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
        Q_ASSERT(obj);
        // Now, retrieve the Feedforward values:
        field = obj->getField(QString("FeedForward"));
        Q_ASSERT(field);
        m_aircraft->feedForwardSlider->setValue(field->getDouble()*100);
        field = obj->getField(QString("AccelTime"));
        Q_ASSERT(field);
        m_aircraft->accelTime->setValue(field->getDouble());
        field = obj->getField(QString("DecelTime"));
        Q_ASSERT(field);
        m_aircraft->decelTime->setValue(field->getDouble());
        field = obj->getField(QString("MaxAccel"));
        Q_ASSERT(field);
        m_aircraft->maxAccelSlider->setValue(field->getDouble());

    }

}

/**
  \brief Sets up the mixer depending on Airframe type. Accepts either system settings or
  combo box entry from airframe type, as those do not overlap.
  */
void ConfigAirframeWidget::setupAirframeUI(QString frameType)
{
    if (frameType == "FixedWing" || frameType == "Elevator aileron rudder") {
        // Setup the UI
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Fixed Wing"));
        m_aircraft->fixedWingType->setCurrentIndex(m_aircraft->fixedWingType->findText("Elevator aileron rudder"));
        m_aircraft->fwRudderChannel->setEnabled(true);
        m_aircraft->fwRudderLabel->setEnabled(true);
        m_aircraft->fwElevator1Channel->setEnabled(true);
        m_aircraft->fwElevator1Label->setEnabled(true);
        m_aircraft->fwElevator2Channel->setEnabled(true);
        m_aircraft->fwElevator2Label->setEnabled(true);
        m_aircraft->fwAileron1Channel->setEnabled(true);
        m_aircraft->fwAileron1Label->setEnabled(true);
        m_aircraft->fwAileron2Channel->setEnabled(true);
        m_aircraft->fwAileron2Label->setEnabled(true);

        m_aircraft->fwAileron1Label->setText("Aileron 1");
        m_aircraft->fwAileron2Label->setText("Aileron 2");
        m_aircraft->fwElevator1Label->setText("Elevator 1");
        m_aircraft->fwElevator2Label->setText("Elevator 2");
        m_aircraft->elevonMixBox->setHidden(true);

    } else if (frameType == "FixedWingElevon" || frameType == "Elevon") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Fixed Wing"));
        m_aircraft->fixedWingType->setCurrentIndex(m_aircraft->fixedWingType->findText("Elevon"));
        m_aircraft->fwAileron1Label->setText("Elevon 1");
        m_aircraft->fwAileron2Label->setText("Elevon 2");
        m_aircraft->fwElevator1Channel->setEnabled(false);
        m_aircraft->fwElevator1Label->setEnabled(false);
        m_aircraft->fwElevator2Channel->setEnabled(false);
        m_aircraft->fwElevator2Label->setEnabled(false);
        m_aircraft->fwRudderChannel->setEnabled(true);
        m_aircraft->fwRudderLabel->setEnabled(true);
        m_aircraft->fwElevator1Label->setText("Elevator 1");
        m_aircraft->fwElevator2Label->setText("Elevator 2");
        m_aircraft->elevonMixBox->setHidden(false);
        m_aircraft->elevonLabel1->setText("Roll");
        m_aircraft->elevonLabel2->setText("Pitch");

     } else if (frameType == "FixedWingVtail" || frameType == "Vtail") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Fixed Wing"));
        m_aircraft->fixedWingType->setCurrentIndex(m_aircraft->fixedWingType->findText("Vtail"));
        m_aircraft->fwRudderChannel->setEnabled(false);
        m_aircraft->fwRudderLabel->setEnabled(false);
        m_aircraft->fwElevator1Channel->setEnabled(true);
        m_aircraft->fwElevator1Label->setEnabled(true);
        m_aircraft->fwElevator1Label->setText("Vtail 1");
        m_aircraft->fwElevator2Label->setText("Vtail 2");
        m_aircraft->elevonMixBox->setHidden(false);
        m_aircraft->fwElevator2Channel->setEnabled(true);
        m_aircraft->fwElevator2Label->setEnabled(true);
        m_aircraft->fwAileron1Label->setText("Aileron 1");
        m_aircraft->fwAileron2Label->setText("Aileron 2");
        m_aircraft->elevonLabel1->setText("Rudder");
        m_aircraft->elevonLabel2->setText("Pitch");
    } else if (frameType == "QuadX" || frameType == "Quad X") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Quad X"));
        quad->setElementId("quad-X");
        m_aircraft->quadShape->setSceneRect(quad->boundingRect());
        m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
        m_aircraft->multiMotor5->setEnabled(false);
        m_aircraft->multiMotor6->setEnabled(false);
        m_aircraft->multiMotor7->setEnabled(false);
        m_aircraft->multiMotor8->setEnabled(false);
    } else if (frameType == "QuadP" || frameType == "Quad +") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Quad +"));
        quad->setElementId("quad-plus");
        m_aircraft->quadShape->setSceneRect(quad->boundingRect());
        m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
        m_aircraft->multiMotor5->setEnabled(false);
        m_aircraft->multiMotor6->setEnabled(false);
        m_aircraft->multiMotor7->setEnabled(false);
        m_aircraft->multiMotor8->setEnabled(false);
    } else if (frameType == "Hexa" || frameType == "Hexacopter") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Hexacopter"));
        quad->setElementId("quad-hexa");
        m_aircraft->quadShape->setSceneRect(quad->boundingRect());
        m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
        m_aircraft->multiMotor5->setEnabled(true);
        m_aircraft->multiMotor6->setEnabled(true);
        m_aircraft->multiMotor7->setEnabled(false);
        m_aircraft->multiMotor8->setEnabled(false);
    } else if (frameType == "Octo" || frameType == "Octocopter") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Octocopter"));
        quad->setElementId("quad-octo");
        m_aircraft->quadShape->setSceneRect(quad->boundingRect());
        m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
        m_aircraft->multiMotor5->setEnabled(true);
        m_aircraft->multiMotor6->setEnabled(true);
        m_aircraft->multiMotor7->setEnabled(true);
        m_aircraft->multiMotor8->setEnabled(true);
    }
}

/**
  Reset the contents of a field
  */
void ConfigAirframeWidget::resetField(UAVObjectField * field)
{
    for (unsigned int i=0;i<field->getNumElements();i++) {
        field->setValue(0,i);
    }
}

/**
  Reset actuator values
  */
void ConfigAirframeWidget::resetActuators()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
    QList<UAVObjectField*> fieldList = obj->getFields();
    // Reset all assignements first:
    foreach (UAVObjectField* field, fieldList) {
        // NOTE: we assume that all options in ActuatorSettings are a channel assignement
        // except for the options called "ChannelXXX"
        if (field->getUnits().contains("channel")) {
            field->setValue(field->getOptions().last());
        }
    }
}

/**
  Setup Elevator/Aileron/Rudder airframe.

   If both Aileron channels are set to 'None' (EasyStar), do Pitch/Rudder mixing

   Returns False if impossible to create the mixer.
  */
bool ConfigAirframeWidget::setupFrameFixedWing()
{
    // Check coherence:
    // - At least Pitch and either Roll or Yaw
    if (m_aircraft->fwEngineChannel->currentText() == "None" ||
        m_aircraft->fwElevator1Channel->currentText() == "None" ||
        ((m_aircraft->fwAileron1Channel->currentText() == "None") &&
        (m_aircraft->fwRudderChannel->currentText() == "None"))) {
        // TODO: explain the problem in the UI
        m_aircraft->fwStatusLabel->setText("ERROR: check channel assignment");
        return false;
    }
    // Now setup the channels:
    resetActuators();

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);

    // Elevator
    UAVObjectField *field = obj->getField("FixedWingPitch1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwElevator1Channel->currentText());
    field = obj->getField("FixedWingPitch2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwElevator2Channel->currentText());
    // Aileron
    field = obj->getField("FixedWingRoll1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron1Channel->currentText());
    field = obj->getField("FixedWingRoll2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron2Channel->currentText());
    // Rudder
    field = obj->getField("FixedWingYaw");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwRudderChannel->currentText());
    // Throttle
    field = obj->getField("FixedWingThrottle");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwEngineChannel->currentText());

    obj->updated();

    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    // ... and compute the matrix:
    // In order to make code a bit nicer, we assume:
    // - Channel dropdowns start with 'None', then 0 to 7

    // 1. Assign the servo/motor/none for each channel
    // Disable all
    foreach(QString mixer, mixerTypes) {
        field = obj->getField(mixer);
        Q_ASSERT(field);
        field->setValue("Disabled");
    }
    // and set only the relevant channels:
    // Engine
    int eng = m_aircraft->fwEngineChannel->currentIndex()-1;
    field = obj->getField(mixerTypes.at(eng));
    field->setValue("Motor");
    field = obj->getField(mixerVectors.at(eng));
    // First of all reset the vector
    resetField(field);
    int ti = field->getElementNames().indexOf("ThrottleCurve1");
    field->setValue(1, ti);

    // Rudder
    eng = m_aircraft->fwRudderChannel->currentIndex()-1;
    // eng will be -1 if rudder is set to "None"
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Yaw");
        field->setValue(1, ti);
    } // Else: we have no rudder, only ailerons, we're fine with it.

    // Ailerons
    eng = m_aircraft->fwAileron1Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Roll");
        field->setValue(1, ti);
        // Only set Aileron 2 if Aileron 1 is defined
        eng = m_aircraft->fwAileron2Channel->currentIndex()-1;
        if (eng > -1) {
            field = obj->getField(mixerTypes.at(eng));
            field->setValue("Servo");
            field = obj->getField(mixerVectors.at(eng));
            resetField(field);
            ti = field->getElementNames().indexOf("Roll");
            field->setValue(1, ti);
        }
    } // Else we have no ailerons. Our consistency check guarantees we have
      // rudder in this case, so we're fine with it too.

    // Elevator
    eng = m_aircraft->fwElevator1Channel->currentIndex()-1;
    field = obj->getField(mixerTypes.at(eng));
    field->setValue("Servo");
    field = obj->getField(mixerVectors.at(eng));
    resetField(field);
    ti = field->getElementNames().indexOf("Pitch");
    field->setValue(1, ti);
    // Only set Elevator 2 if it is defined
    eng = m_aircraft->fwElevator2Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Pitch");
        field->setValue(1, ti);
    }

    obj->updated();
    m_aircraft->fwStatusLabel->setText("Mixer generated");

    return true;
}


/**
  Setup Elevon
  */
bool ConfigAirframeWidget::setupFrameElevon()
{
    // Check coherence:
    // - At least Aileron1 and Aileron 2, and engine
    if (m_aircraft->fwEngineChannel->currentText() == "None" ||
        m_aircraft->fwAileron1Channel->currentText() == "None" ||
        m_aircraft->fwAileron2Channel->currentText() == "None") {
        // TODO: explain the problem in the UI
        m_aircraft->fwStatusLabel->setText("ERROR: check channel assignment");
        return false;
    }

    resetActuators();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);

    // Elevons
    UAVObjectField *field = obj->getField("FixedWingRoll1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron1Channel->currentText());
    field = obj->getField("FixedWingRoll2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron2Channel->currentText());
    // Rudder (can be None)
    field = obj->getField("FixedWingYaw");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwRudderChannel->currentText());
    // Throttle
    field = obj->getField("FixedWingThrottle");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwEngineChannel->currentText());

    obj->updated();

    // Save the curve:
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    // ... and compute the matrix:
    // In order to make code a bit nicer, we assume:
    // - Channel dropdowns start with 'None', then 0 to 7

    // 1. Assign the servo/motor/none for each channel
    // Disable all
    foreach(QString mixer, mixerTypes) {
        field = obj->getField(mixer);
        Q_ASSERT(field);
        field->setValue("Disabled");
    }
    // and set only the relevant channels:
    // Engine
    int eng = m_aircraft->fwEngineChannel->currentIndex()-1;
    field = obj->getField(mixerTypes.at(eng));
    field->setValue("Motor");
    field = obj->getField(mixerVectors.at(eng));
    // First of all reset the vector
    resetField(field);
    int ti = field->getElementNames().indexOf("ThrottleCurve1");
    field->setValue(1, ti);

    // Rudder
    eng = m_aircraft->fwRudderChannel->currentIndex()-1;
    // eng will be -1 if rudder is set to "None"
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Yaw");
        field->setValue(1, ti);
    } // Else: we have no rudder, only elevons, we're fine with it.

    eng = m_aircraft->fwAileron1Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Pitch");
        field->setDouble((double)m_aircraft->elevonSlider2->value()/100, ti);
        ti = field->getElementNames().indexOf("Roll");
        field->setDouble((double)m_aircraft->elevonSlider1->value()/100,ti);
    }

    eng = m_aircraft->fwAileron2Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Pitch");
        field->setDouble((double)m_aircraft->elevonSlider2->value()/100, ti);
        ti = field->getElementNames().indexOf("Roll");
        field->setDouble(-(double)m_aircraft->elevonSlider1->value()/100,ti);
    }

    obj->updated();
    m_aircraft->fwStatusLabel->setText("Mixer generated");
    return true;
}

/**
  Setup VTail
  */
bool ConfigAirframeWidget::setupFrameVtail()
{
    // Check coherence:
    // - At least Pitch1 and Pitch2, and engine
    if (m_aircraft->fwEngineChannel->currentText() == "None" ||
        m_aircraft->fwElevator1Channel->currentText() == "None" ||
        m_aircraft->fwElevator2Channel->currentText() == "None") {
        // TODO: explain the problem in the UI
        m_aircraft->fwStatusLabel->setText("WARNING: check channel assignment");
        return false;
    }

    resetActuators();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);

    // Elevons
    UAVObjectField *field = obj->getField("FixedWingPitch1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwElevator1Channel->currentText());
    field = obj->getField("FixedWingPitch2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwElevator2Channel->currentText());
    field = obj->getField("FixedWingRoll1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron1Channel->currentText());
    field = obj->getField("FixedWingRoll2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron2Channel->currentText());

    // Throttle
    field = obj->getField("FixedWingThrottle");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwEngineChannel->currentText());

    obj->updated();

    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    // ... and compute the matrix:
    // In order to make code a bit nicer, we assume:
    // - Channel dropdowns start with 'None', then 0 to 7

    // 1. Assign the servo/motor/none for each channel
    // Disable all
    foreach(QString mixer, mixerTypes) {
        field = obj->getField(mixer);
        Q_ASSERT(field);
        field->setValue("Disabled");
    }
    // and set only the relevant channels:
    // Engine
    int eng = m_aircraft->fwEngineChannel->currentIndex()-1;
    field = obj->getField(mixerTypes.at(eng));
    field->setValue("Motor");
    field = obj->getField(mixerVectors.at(eng));
    // First of all reset the vector
    resetField(field);
    int ti = field->getElementNames().indexOf("ThrottleCurve1");
    field->setValue(1, ti);

    eng = m_aircraft->fwAileron1Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Roll");
        field->setDouble(1,ti);
    }

    eng = m_aircraft->fwAileron2Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Roll");
        field->setDouble(-1,ti);
    }

    // Now compute the VTail
    eng = m_aircraft->fwElevator1Channel->currentIndex()-1;
    field = obj->getField(mixerTypes.at(eng));
    field->setValue("Servo");
    field = obj->getField(mixerVectors.at(eng));
    resetField(field);
    ti = field->getElementNames().indexOf("Pitch");
    field->setDouble((double)m_aircraft->elevonSlider2->value()/100, ti);
    ti = field->getElementNames().indexOf("Yaw");
    field->setDouble((double)m_aircraft->elevonSlider1->value()/100,ti);

    eng = m_aircraft->fwElevator2Channel->currentIndex()-1;
    field = obj->getField(mixerTypes.at(eng));
    field->setValue("Servo");
    field = obj->getField(mixerVectors.at(eng));
    resetField(field);
    ti = field->getElementNames().indexOf("Pitch");
    field->setDouble((double)m_aircraft->elevonSlider2->value()/100, ti);
    ti = field->getElementNames().indexOf("Yaw");
    field->setDouble(-(double)m_aircraft->elevonSlider1->value()/100,ti);

    obj->updated();
    m_aircraft->fwStatusLabel->setText("Mixer generated");
    return true;
}


/**
  Help function: setupQuadMotor
  */
void ConfigAirframeWidget::setupQuadMotor(int channel, double pitch, double roll, double yaw)
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    UAVObjectField *field = obj->getField(mixerTypes.at(channel));
    field->setValue("Motor");
    field = obj->getField(mixerVectors.at(channel));
    // First of all reset the vector
    resetField(field);
    int ti = field->getElementNames().indexOf("ThrottleCurve1");
    field->setValue(1, ti);
    ti = field->getElementNames().indexOf("Roll");
    field->setValue(roll,ti);
    ti = field->getElementNames().indexOf("Pitch");
    field->setValue(pitch,ti);
    ti = field->getElementNames().indexOf("Yaw");
    field->setValue(yaw,ti);
}

/**
  Set up a Quad-X or Quad-P
*/
bool ConfigAirframeWidget::setupQuad(bool pLayout)
{
    // Check coherence:
    // - Four engines have to be defined
    if (m_aircraft->multiMotor1->currentText() == "None" ||
        m_aircraft->multiMotor2->currentText() == "None" ||
        m_aircraft->multiMotor3->currentText() == "None" ||
        m_aircraft->multiMotor4->currentText() == "None") {
        m_aircraft->mrStatusLabel->setText("ERROR: Assign 4 motor channels");
        return false;
    }

    resetActuators();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
    UAVObjectField *field;
    // Save the actuator channels for motor 1 to 4 (N/E/S/W)
    field = pLayout ? obj->getField("VTOLMotorN") : obj->getField("VTOLMotorNW");
    Q_ASSERT(field);
    field->setValue(m_aircraft->multiMotor1->currentText());
    field = pLayout ? obj->getField("VTOLMotorE") : obj->getField("VTOLMotorNE");
    Q_ASSERT(field);
    field->setValue(m_aircraft->multiMotor2->currentText());
    field = pLayout ? obj->getField("VTOLMotorS") : obj->getField("VTOLMotorSE");
    Q_ASSERT(field);
    field->setValue(m_aircraft->multiMotor3->currentText());
    field = pLayout ? obj->getField("VTOLMotorW") : obj->getField("VTOLMotorSW");
    Q_ASSERT(field);
    field->setValue(m_aircraft->multiMotor4->currentText());

    obj->updated(); // Save...

    // Now, setup the mixer:
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    // 1. Assign the servo/motor/none for each channel
    // Disable all
    foreach(QString mixer, mixerTypes) {
        field = obj->getField(mixer);
        Q_ASSERT(field);
        field->setValue("Disabled");
    }
    // and set only the relevant channels:

    // Motor 1 to 4, X Layout:
    //     pitch   roll    yaw
    //    {0.5    ,0.5    ,-0.5     //Front left motor (CW)
    //    {0.5    ,-0.5   ,0.5   //Front right motor(CCW)
    //    {-0.5  ,-0.5    ,-0.5    //rear right motor (CW)
    //    {-0.5   ,0.5    ,0.5   //Rear left motor  (CCW)
    //
    // Motor 1 to 4, P Layout:
    // pitch   roll    yaw
    //  {1      ,0      ,-0.5    //Front motor (CW)
    //  {0      ,-1     ,0.5   //Right  motor(CCW)
    //  {-1     ,0      ,-0.5    //Rear motor  (CW)
    //  {0      ,1      ,0.5   //Left motor  (CCW)
    int channel = m_aircraft->multiMotor1->currentIndex()-1;
    pLayout ? setupQuadMotor(channel, 1, 0, -0.5)
            : setupQuadMotor(channel, 0.5, 0.5, -0.5);
    channel = m_aircraft->multiMotor2->currentIndex()-1;
    pLayout ? setupQuadMotor(channel, 0, -1, 0.5)
            : setupQuadMotor(channel, 0.5, -0.5, 0.5);
    channel = m_aircraft->multiMotor3->currentIndex()-1;
    pLayout ? setupQuadMotor(channel, -1, 0, -0.5)
            : setupQuadMotor(channel, -0.5, -0.5, -0.5);
    channel = m_aircraft->multiMotor4->currentIndex()-1;
    pLayout ? setupQuadMotor(channel, 0, 1, 0.5)
            : setupQuadMotor(channel, -0.5, 0.5, 0.5);

    obj->updated();
    m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
    return true;
}


/**
  Sends the config to the board (airframe type)

  We do all the tasks commong to all airframes, or family of airframes, and
  we call additional methods for specific frames, so that we do not have a code
  that is too heavy.
  */
void ConfigAirframeWidget::sendAircraftUpdate()
{
    QString airframeType;
    if (m_aircraft->aircraftType->currentText() == "Fixed Wing") {
        // Save the curve (common to all Fixed wing frames)
        UAVDataObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
        Q_ASSERT(obj);
        UAVObjectField* field = obj->getField("ThrottleCurve1");
        QList<double> curve = m_aircraft->fixedWingThrottle->getCurve();
        for (int i=0;i<curve.length();i++) {
            field->setValue(curve.at(i),i);
        }

        if (m_aircraft->fixedWingType->currentText() == "Elevator aileron rudder" ) {
            airframeType = "FixedWing";
            setupFrameFixedWing();
        } else if (m_aircraft->fixedWingType->currentText() == "Elevon") {
            airframeType = "FixedWingElevon";
            setupFrameElevon();
        } else { // Vtail
            airframeType = "FixedWingVtail";
            setupFrameVtail();
        }
    } else if (m_aircraft->aircraftType->currentText() == "Multirotor") {
        // We can already setup the feedforward here, as it is common to all platforms
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
        Q_ASSERT(obj);
        UAVObjectField* field = obj->getField(QString("FeedForward"));
        Q_ASSERT(field);
        field->setDouble((double)m_aircraft->feedForwardSlider->value()/100);
        field = obj->getField(QString("AccelTime"));
        Q_ASSERT(field);
        field->setDouble(m_aircraft->accelTime->value());
        field = obj->getField(QString("DecelTime"));
        Q_ASSERT(field);
        field->setDouble(m_aircraft->decelTime->value());
        field = obj->getField(QString("MaxAccel"));
        Q_ASSERT(field);
        field->setDouble(m_aircraft->maxAccelSlider->value());

        // Curve is also common to all quads:
        field = obj->getField("ThrottleCurve1");
        QList<double> curve = m_aircraft->multiThrottleCurve->getCurve();
        for (int i=0;i<curve.length();i++) {
            field->setValue(curve.at(i),i);
        }

        if (m_aircraft->multirotorFrameType->currentText() == "Quad +") {
            airframeType = "QuadP";
            setupQuad(true);
        } else if (m_aircraft->multirotorFrameType->currentText() == "Quad X") {
            airframeType = "QuadX";
            setupQuad(false);
        } else if (m_aircraft->multirotorFrameType->currentText() == "Hexacopter") {
            airframeType = "Hexa";
        } else if (m_aircraft->multirotorFrameType->currentText() == "Octocopter") {
            airframeType = "Octo";
        }

    } else {
        airframeType = "FixedWing";
    }

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("SystemSettings")));
    Q_ASSERT(obj);
    UAVObjectField* field = obj->getField(QString("AirframeType"));
    field->setValue(airframeType);
    obj->updated();
}

/**
  Send airframe type to the board and request saving to SD card
  */
void ConfigAirframeWidget::saveAircraftUpdate()
{
    // Send update so that the latest value is saved
    sendAircraftUpdate();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("SystemSettings")));
    Q_ASSERT(obj);
    saveObjectToSD(obj);
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    saveObjectToSD(obj);
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    saveObjectToSD(obj);

}

