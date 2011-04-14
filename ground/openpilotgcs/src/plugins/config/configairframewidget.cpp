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
#include <QTimer>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <math.h>

/**
  Helper delegate for the custom mixer editor table.
  Taken straight from Qt examples, thanks!
  */
SpinBoxDelegate::SpinBoxDelegate(QObject *parent)
     : QItemDelegate(parent)
 {
 }

QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    QSpinBox *editor = new QSpinBox(parent);
    editor->setMinimum(-127);
    editor->setMaximum(127);

    return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

/**********************************************************************************/



ConfigAirframeWidget::ConfigAirframeWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_aircraft = new Ui_AircraftWidget();
    m_aircraft->setupUi(this);

    ffTuningInProgress = false;
    ffTuningPhase = false;

    mixerTypes << "Mixer1Type" << "Mixer2Type" << "Mixer3Type"
            << "Mixer4Type" << "Mixer5Type" << "Mixer6Type" << "Mixer7Type" << "Mixer8Type";
    mixerVectors << "Mixer1Vector" << "Mixer2Vector" << "Mixer3Vector"
            << "Mixer4Vector" << "Mixer5Vector" << "Mixer6Vector" << "Mixer7Vector" << "Mixer8Vector";

    QStringList airframeTypes;
    airframeTypes << "Fixed Wing" << "Multirotor" << "Helicopter" << "Custom";
    m_aircraft->aircraftType->addItems(airframeTypes);
    m_aircraft->aircraftType->setCurrentIndex(1);

    QStringList fixedWingTypes;
    fixedWingTypes << "Elevator aileron rudder" << "Elevon" << "Vtail";
    m_aircraft->fixedWingType->addItems(fixedWingTypes);

    QStringList multiRotorTypes;
    multiRotorTypes << "Quad +" << "Quad X" << "Hexacopter" << "Octocopter" << "Hexacopter X" << "Octocopter V" << "Octo Coax +"
            << "Octo Coax X" << "Hexacopter Y6" << "Tricopter Y";
    m_aircraft->multirotorFrameType->addItems(multiRotorTypes);



    QStringList channels;
    channels << "None" << "Channel1" << "Channel2" << "Channel3" <<
            "Channel4" << "Channel5" << "Channel6" << "Channel7" << "Channel8";
    // Now load all the channel assignements for fixed wing
    m_aircraft->fwElevator1Channel->addItems(channels);
    m_aircraft->fwElevator2Channel->addItems(channels);
    m_aircraft->fwEngineChannel->addItems(channels);
    m_aircraft->fwRudder1Channel->addItems(channels);
    m_aircraft->fwRudder2Channel->addItems(channels);
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
    m_aircraft->triYawChannel->addItems(channels);

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

    // Put combo boxes in line one of the custom mixer table:
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("Mixer1Type"));
    QStringList list = field->getOptions();
    for (int i=0;i<8;i++) {
        QComboBox* qb = new QComboBox(m_aircraft->customMixerTable);
        qb->addItems(list);
        m_aircraft->customMixerTable->setCellWidget(0,i,qb);
    }

    SpinBoxDelegate *sbd = new SpinBoxDelegate();
    for (int i=1;i<8; i++) {
        m_aircraft->customMixerTable->setItemDelegateForRow(i, sbd);
    }

    connect(m_aircraft->saveAircraftToSD, SIGNAL(clicked()), this, SLOT(saveAircraftUpdate()));
    connect(m_aircraft->saveAircraftToRAM, SIGNAL(clicked()), this, SLOT(sendAircraftUpdate()));
    connect(m_aircraft->getAircraftCurrent, SIGNAL(clicked()), this, SLOT(requestAircraftUpdate()));
    connect(m_aircraft->fixedWingType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupAirframeUI(QString)));
    connect(m_aircraft->multirotorFrameType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupAirframeUI(QString)));
    connect(m_aircraft->aircraftType, SIGNAL(currentIndexChanged(int)), this, SLOT(switchAirframeType(int)));
    requestAircraftUpdate();

    connect(m_aircraft->fwThrottleReset, SIGNAL(clicked()), this, SLOT(resetFwMixer()));
    connect(m_aircraft->mrThrottleCurveReset, SIGNAL(clicked()), this, SLOT(resetMrMixer()));
    connect(m_aircraft->customReset1, SIGNAL(clicked()), this, SLOT(resetCt1Mixer()));
    connect(m_aircraft->customReset2, SIGNAL(clicked()), this, SLOT(resetCt2Mixer()));
    connect(m_aircraft->fixedWingThrottle, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateFwThrottleCurveValue(QList<double>,double)));
    connect(m_aircraft->multiThrottleCurve, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateMrThrottleCurveValue(QList<double>,double)));
    connect(m_aircraft->customThrottle1Curve, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateCustomThrottle1CurveValue(QList<double>,double)));
    connect(m_aircraft->customThrottle2Curve, SIGNAL(curveUpdated(QList<double>,double)), this, SLOT(updateCustomThrottle2CurveValue(QList<double>,double)));

//    connect(m_aircraft->fwAileron1Channel, SIGNAL(currentIndexChanged(int)), this, SLOT(toggleAileron2(int)));
//    connect(m_aircraft->fwElevator1Channel, SIGNAL(currentIndexChanged(int)), this, SLOT(toggleElevator2(int)));

    // Now connect the three feed forward test checkboxes
    connect(m_aircraft->ffTestBox1, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));
    connect(m_aircraft->ffTestBox2, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));
    connect(m_aircraft->ffTestBox3, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));

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
    if (m_aircraft->aircraftType->findText("Custom")) {
        m_aircraft->customMixerTable->resizeColumnsToContents();
        for (int i=0;i<8;i++) {
            m_aircraft->customMixerTable->setColumnWidth(i,(m_aircraft->customMixerTable->width()-
                                                            m_aircraft->customMixerTable->verticalHeader()->width())/8);
        }
    }
}

void ConfigAirframeWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
    m_aircraft->customMixerTable->resizeColumnsToContents();
    for (int i=0;i<8;i++) {
        m_aircraft->customMixerTable->setColumnWidth(i,(m_aircraft->customMixerTable->width()-
                                                        m_aircraft->customMixerTable->verticalHeader()->width())/8);
    }
}

void ConfigAirframeWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
    // Make the custom table columns autostretch:
    m_aircraft->customMixerTable->resizeColumnsToContents();
    for (int i=0;i<8;i++) {
        m_aircraft->customMixerTable->setColumnWidth(i,(m_aircraft->customMixerTable->width()-
                                                        m_aircraft->customMixerTable->verticalHeader()->width())/8);
    }

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

void ConfigAirframeWidget::toggleRudder2(int index)
{
    if (index) {
        m_aircraft->fwRudder2Channel->setEnabled(true);
        m_aircraft->fwRudder2Label->setEnabled(true);
    } else {
        m_aircraft->fwRudder2Channel->setEnabled(false);
        m_aircraft->fwRudder2Label->setEnabled(false);
    }
}

/////////////////////////////////////////////////////////
/// Feed Forward Testing
/////////////////////////////////////////////////////////

/**
  Enables and runs feed forward testing
  */
void ConfigAirframeWidget::enableFFTest()
{
    // Role:
    // - Check if all three checkboxes are checked
    // - Every other timer event: toggle engine from 45% to 55%
    // - Every other time event: send FF settings to flight FW
    if (m_aircraft->ffTestBox1->isChecked() &&
        m_aircraft->ffTestBox2->isChecked() &&
        m_aircraft->ffTestBox3->isChecked()) {
        if (!ffTuningInProgress)
        {
            // Initiate tuning:
            UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ManualControlCommand")));
            UAVObject::Metadata mdata = obj->getMetadata();
            accInitialData = mdata;
            mdata.flightAccess = UAVObject::ACCESS_READONLY;
            obj->setMetadata(mdata);
        }
        // Depending on phase, either move actuator or send FF settings:
        if (ffTuningPhase) {
            // Send FF settings to the board
            UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            UAVObjectField* field = obj->getField(QString("FeedForward"));
            field->setDouble((double)m_aircraft->feedForwardSlider->value()/100);
            field = obj->getField(QString("AccelTime"));
            field->setDouble(m_aircraft->accelTime->value());
            field = obj->getField(QString("DecelTime"));
            field->setDouble(m_aircraft->decelTime->value());
            field = obj->getField(QString("MaxAccel"));
            field->setDouble(m_aircraft->maxAccelSlider->value());
            obj->updated();
        } else  {
            // Toggle motor state
            UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ManualControlCommand")));
            double value = obj->getField("Throttle")->getDouble();
            double target = (value < 0.5) ? 0.55 : 0.45;
            obj->getField("Throttle")->setValue(target);
            obj->updated();
        }
        ffTuningPhase = !ffTuningPhase;
        ffTuningInProgress = true;
        QTimer::singleShot(1000, this, SLOT(enableFFTest()));
    } else {
        // - If no: disarm timer, restore actuatorcommand metadata
        // Disarm!
        if (ffTuningInProgress) {
            ffTuningInProgress = false;
            UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ManualControlCommand")));
            UAVObject::Metadata mdata = obj->getMetadata();
            mdata = accInitialData; // Restore metadata
            obj->setMetadata(mdata);
        }
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
  Resets Custom throttle 1 mixer
  */
void ConfigAirframeWidget::resetCt1Mixer()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve1"));
    resetMixer(m_aircraft->customThrottle1Curve, field->getNumElements());
}

/**
  Resets Custom throttle 2 mixer
  */
void ConfigAirframeWidget::resetCt2Mixer()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve2"));
    resetMixer(m_aircraft->customThrottle2Curve, field->getNumElements());
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

/**
  Updates the currently moved throttle curve item value (Custom throttle 1)
  */
void ConfigAirframeWidget::updateCustomThrottle1CurveValue(QList<double> list, double value)
{
    Q_UNUSED(list);
    m_aircraft->customThrottleCurve1Value->setText(QString().sprintf("Val: %.2f",value));
}

/**
  Updates the currently moved throttle curve item value (Custom throttle 2)
  */
void ConfigAirframeWidget::updateCustomThrottle2CurveValue(QList<double> list, double value)
{
    Q_UNUSED(list);
    m_aircraft->customThrottleCurve2Value->setText(QString().sprintf("Val: %.2f",value));
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
        field = obj->getField(QString("FixedWingYaw1"));
        Q_ASSERT(field);
        m_aircraft->fwRudder1Channel->setCurrentIndex(m_aircraft->fwRudder1Channel->findText(field->getValue().toString()));
        field = obj->getField(QString("FixedWingYaw2"));
        Q_ASSERT(field);
        m_aircraft->fwRudder2Channel->setCurrentIndex(m_aircraft->fwRudder2Channel->findText(field->getValue().toString()));

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
               frameType == "Hexa" || frameType == "Octo" ||
               frameType == "HexaCoax" || frameType == "OctoV" ||
               frameType == "HexaX" || frameType == "OctoCoaxP" ||
               frameType == "OctoCoaxX" || frameType == "Tri") {
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
            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
            // This assumes that all vectors are identical - if not, the user should use the
            // "custom" setting.
            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            Q_ASSERT(obj);
            int eng= m_aircraft->multiMotor1->currentIndex()-1;
            // eng will be -1 if value is set to "None"
            if (eng > -1) {
                field = obj->getField(mixerVectors.at(eng));
                int i = field->getElementNames().indexOf("Pitch");
                double val = field->getDouble(i)/1.27;
                m_aircraft->mrPitchMixLevel->setValue(val);
                i = field->getElementNames().indexOf("Yaw");
                val = (1-field->getDouble(i)/1.27);
                m_aircraft->mrYawMixLevel->setValue(val);
                eng = m_aircraft->multiMotor2->currentIndex()-1;
                field = obj->getField(mixerVectors.at(eng));
                i = field->getElementNames().indexOf("Roll");
                val = -field->getDouble(i)/1.27;
                m_aircraft->mrRollMixLevel->setValue(val);
            }
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
            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
            // This assumes that all vectors are identical - if not, the user should use the
            // "custom" setting.
            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            Q_ASSERT(obj);
            int eng= m_aircraft->multiMotor1->currentIndex()-1;
            // eng will be -1 if value is set to "None"
            if (eng > -1) {
                field = obj->getField(mixerVectors.at(eng));
                int i = field->getElementNames().indexOf("Pitch");
                double val = field->getDouble(i)/1.27;
                m_aircraft->mrPitchMixLevel->setValue(val);
                i = field->getElementNames().indexOf("Yaw");
                val = 1-field->getDouble(i)/1.27;
                m_aircraft->mrYawMixLevel->setValue(val);
                i = field->getElementNames().indexOf("Roll");
                val = field->getDouble(i)/1.27;
                m_aircraft->mrRollMixLevel->setValue(val);
            }
        } else if (frameType == "Hexa") {
            // Motors 1/2/3 4/5/6 are: N / NE / SE / S / SW / NW
            field = obj->getField(QString("VTOLMotorN"));
            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorNE"));
            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorSE"));
            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorS"));
            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorSW"));
            m_aircraft->multiMotor5->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorNW"));
            m_aircraft->multiMotor6->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
            // This assumes that all vectors are identical - if not, the user should use the
            // "custom" setting.
            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            int eng= m_aircraft->multiMotor1->currentIndex()-1;
            // eng will be -1 if value is set to "None"
            if (eng > -1) {
                field = obj->getField(mixerVectors.at(eng));
                int i = field->getElementNames().indexOf("Pitch");
                double val = floor(field->getDouble(i)/1.27);
                m_aircraft->mrPitchMixLevel->setValue(val);
                i = field->getElementNames().indexOf("Yaw");
                val = floor(-field->getDouble(i)/1.27);
                m_aircraft->mrYawMixLevel->setValue(val);
                eng = m_aircraft->multiMotor2->currentIndex()-1;
                field = obj->getField(mixerVectors.at(eng));
                i = field->getElementNames().indexOf("Roll");
                val = floor(1-field->getDouble(i)/1.27);
                m_aircraft->mrRollMixLevel->setValue(val);
            }
        } else if (frameType == "HexaX") {
            // Motors 1/2/3 4/5/6 are: NE / E / SE / SW / W / NW
            field = obj->getField(QString("VTOLMotorNE"));
            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorE"));
            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorSE"));
            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorSW"));
            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorW"));
            m_aircraft->multiMotor5->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorNW"));
            m_aircraft->multiMotor6->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
            // This assumes that all vectors are identical - if not, the user should use the
            // "custom" setting.
            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            int eng= m_aircraft->multiMotor1->currentIndex()-1;
            // eng will be -1 if value is set to "None"
            if (eng > -1) {
                field = obj->getField(mixerVectors.at(eng));
                int i = field->getElementNames().indexOf("Pitch");
                double val = floor(field->getDouble(i)/1.27);
                m_aircraft->mrPitchMixLevel->setValue(val);
                i = field->getElementNames().indexOf("Yaw");
                val = floor(-field->getDouble(i)/1.27);
                m_aircraft->mrYawMixLevel->setValue(val);
                eng = m_aircraft->multiMotor2->currentIndex()-1;
                field = obj->getField(mixerVectors.at(eng));
                i = field->getElementNames().indexOf("Roll");
                val = floor(1-field->getDouble(i)/1.27);
                m_aircraft->mrRollMixLevel->setValue(val);
            }
        } else if (frameType == "HexaCoax") {
            // Motors 1/2/3 4/5/6 are: NW/W NE/E S/SE
            field = obj->getField(QString("VTOLMotorNW"));
            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorW"));
            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorNE"));
            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorE"));
            m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorS"));
            m_aircraft->multiMotor5->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorSE"));
            m_aircraft->multiMotor6->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
            // This assumes that all vectors are identical - if not, the user should use the
            // "custom" setting.
            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            int eng= m_aircraft->multiMotor1->currentIndex()-1;
            // eng will be -1 if value is set to "None"
            if (eng > -1) {
                field = obj->getField(mixerVectors.at(eng));
                int i = field->getElementNames().indexOf("Pitch");
                double val = floor(2*field->getDouble(i)/1.27);
                m_aircraft->mrPitchMixLevel->setValue(val);
                i = field->getElementNames().indexOf("Yaw");
                val = floor(-field->getDouble(i)/1.27);
                m_aircraft->mrYawMixLevel->setValue(val);
                i = field->getElementNames().indexOf("Roll");
                val = floor(field->getDouble(i)/1.27);
                m_aircraft->mrRollMixLevel->setValue(val);
            }
        }  else if (frameType == "Octo" || frameType == "OctoV" ||
                    frameType == "OctoCoaxP") {
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
            // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
            // This assumes that all vectors are identical - if not, the user should use the
            // "custom" setting.
            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            int eng= m_aircraft->multiMotor1->currentIndex()-1;
            // eng will be -1 if value is set to "None"
            if (eng > -1) {
                if (frameType == "Octo") {
                    field = obj->getField(mixerVectors.at(eng));
                    int i = field->getElementNames().indexOf("Pitch");
                    double val = floor(field->getDouble(i)/1.27);
                    m_aircraft->mrPitchMixLevel->setValue(val);
                    i = field->getElementNames().indexOf("Yaw");
                    val = floor(-field->getDouble(i)/1.27);
                    m_aircraft->mrYawMixLevel->setValue(val);
                    eng = m_aircraft->multiMotor2->currentIndex()-1;
                    field = obj->getField(mixerVectors.at(eng));
                    i = field->getElementNames().indexOf("Roll");
                    val = floor(-field->getDouble(i)/1.27);
                    m_aircraft->mrRollMixLevel->setValue(val);
                } else if (frameType == "OctoV") {
                    field = obj->getField(mixerVectors.at(eng));
                    int i = field->getElementNames().indexOf("Yaw");
                    double val = floor(-field->getDouble(i)/1.27);
                    m_aircraft->mrYawMixLevel->setValue(val);
                    i = field->getElementNames().indexOf("Roll");
                    val = floor(-field->getDouble(i)/1.27);
                    m_aircraft->mrRollMixLevel->setValue(val);
                    eng = m_aircraft->multiMotor2->currentIndex()-1;
                    field = obj->getField(mixerVectors.at(eng));
                    i = field->getElementNames().indexOf("Pitch");
                    val = floor(field->getDouble(i)/1.27);
                    m_aircraft->mrPitchMixLevel->setValue(val);

                } else if (frameType == "OctoCoaxP") {
                    field = obj->getField(mixerVectors.at(eng));
                    int i = field->getElementNames().indexOf("Pitch");
                    double val = floor(field->getDouble(i)/1.27);
                    m_aircraft->mrPitchMixLevel->setValue(val);
                    i = field->getElementNames().indexOf("Yaw");
                    val = floor(-field->getDouble(i)/1.27);
                    m_aircraft->mrYawMixLevel->setValue(val);
                    eng = m_aircraft->multiMotor3->currentIndex()-1;
                    field = obj->getField(mixerVectors.at(eng));
                    i = field->getElementNames().indexOf("Roll");
                    val = floor(-field->getDouble(i)/1.27);
                    m_aircraft->mrRollMixLevel->setValue(val);

                }
            }
        } else if (frameType == "OctoCoaxX") {
                 // Motors 1 to 8 are N / NE / E / etc
                 field = obj->getField(QString("VTOLMotorNW"));
                 Q_ASSERT(field);
                 m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
                 field = obj->getField(QString("VTOLMotorN"));
                 Q_ASSERT(field);
                 m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
                 field = obj->getField(QString("VTOLMotorNE"));
                 Q_ASSERT(field);
                 m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
                 field = obj->getField(QString("VTOLMotorE"));
                 Q_ASSERT(field);
                 m_aircraft->multiMotor4->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
                 field = obj->getField(QString("VTOLMotorSE"));
                 Q_ASSERT(field);
                 m_aircraft->multiMotor5->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
                 field = obj->getField(QString("VTOLMotorS"));
                 Q_ASSERT(field);
                 m_aircraft->multiMotor6->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
                 field = obj->getField(QString("VTOLMotorSW"));
                 Q_ASSERT(field);
                 m_aircraft->multiMotor7->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
                 field = obj->getField(QString("VTOLMotorW"));
                 Q_ASSERT(field);
                 m_aircraft->multiMotor8->setCurrentIndex(m_aircraft->multiMotor4->findText(field->getValue().toString()));
                 // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
                 // This assumes that all vectors are identical - if not, the user should use the
                 // "custom" setting.
                 obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
                 int eng= m_aircraft->multiMotor1->currentIndex()-1;
                 // eng will be -1 if value is set to "None"
                 if (eng > -1) {
                     field = obj->getField(mixerVectors.at(eng));
                     int i = field->getElementNames().indexOf("Pitch");
                     double val = floor(field->getDouble(i)/1.27);
                     m_aircraft->mrPitchMixLevel->setValue(val);
                     i = field->getElementNames().indexOf("Yaw");
                     val = floor(-field->getDouble(i)/1.27);
                     m_aircraft->mrYawMixLevel->setValue(val);
                     i = field->getElementNames().indexOf("Roll");
                     val = floor(field->getDouble(i)/1.27);
                     m_aircraft->mrRollMixLevel->setValue(val);
                 }
        } else if (frameType == "Tri") {
            // Motors 1 to 8 are N / NE / E / etc
            field = obj->getField(QString("VTOLMotorNW"));
            Q_ASSERT(field);
            m_aircraft->multiMotor1->setCurrentIndex(m_aircraft->multiMotor1->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorNE"));
            Q_ASSERT(field);
            m_aircraft->multiMotor2->setCurrentIndex(m_aircraft->multiMotor2->findText(field->getValue().toString()));
            field = obj->getField(QString("VTOLMotorS"));
            Q_ASSERT(field);
            m_aircraft->multiMotor3->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));
            field = obj->getField(QString("FixedWingYaw1"));
            Q_ASSERT(field);
            m_aircraft->triYawChannel->setCurrentIndex(m_aircraft->multiMotor3->findText(field->getValue().toString()));

            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            int eng= m_aircraft->multiMotor1->currentIndex()-1;
            // eng will be -1 if value is set to "None"
            if (eng > -1) {
                field = obj->getField(mixerVectors.at(eng));
                int i = field->getElementNames().indexOf("Pitch");
                double val = floor(2*field->getDouble(i)/1.27);
                m_aircraft->mrPitchMixLevel->setValue(val);
                i = field->getElementNames().indexOf("Roll");
                val = floor(field->getDouble(i)/1.27);
                m_aircraft->mrRollMixLevel->setValue(val);
            }

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

    } else if (frameType == "HeliCP") {
        m_aircraft->widget_3->requestccpmUpdate();
         m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Helicopter"));//"Helicopter"
     } else if (frameType == "Custom") {
         m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Custom"));
     }

    updateCustomAirframeUI();
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
        m_aircraft->fwRudder1Channel->setEnabled(true);
        m_aircraft->fwRudder1Label->setEnabled(true);
        m_aircraft->fwRudder2Channel->setEnabled(true);
        m_aircraft->fwRudder2Label->setEnabled(true);
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
        m_aircraft->fwRudder1Channel->setEnabled(true);
        m_aircraft->fwRudder1Label->setEnabled(true);
        m_aircraft->fwRudder2Channel->setEnabled(true);
        m_aircraft->fwRudder2Label->setEnabled(true);
        m_aircraft->fwElevator1Label->setText("Elevator 1");
        m_aircraft->fwElevator2Label->setText("Elevator 2");
        m_aircraft->elevonMixBox->setHidden(false);
        m_aircraft->elevonLabel1->setText("Roll");
        m_aircraft->elevonLabel2->setText("Pitch");

     } else if (frameType == "FixedWingVtail" || frameType == "Vtail") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Fixed Wing"));
        m_aircraft->fixedWingType->setCurrentIndex(m_aircraft->fixedWingType->findText("Vtail"));
        m_aircraft->fwRudder1Channel->setEnabled(false);
        m_aircraft->fwRudder1Label->setEnabled(false);
        m_aircraft->fwRudder2Channel->setEnabled(false);
        m_aircraft->fwRudder2Label->setEnabled(false);
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
        m_aircraft->multiMotor4->setEnabled(true);
        m_aircraft->multiMotor5->setEnabled(false);
        m_aircraft->multiMotor6->setEnabled(false);
        m_aircraft->multiMotor7->setEnabled(false);
        m_aircraft->multiMotor8->setEnabled(false);
        m_aircraft->triYawChannel->setEnabled(false);
        m_aircraft->mrRollMixLevel->setValue(50);
        m_aircraft->mrPitchMixLevel->setValue(50);
        m_aircraft->mrYawMixLevel->setValue(50);
    } else if (frameType == "QuadP" || frameType == "Quad +") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Quad +"));
        quad->setElementId("quad-plus");
        m_aircraft->multiMotor4->setEnabled(true);
        m_aircraft->multiMotor5->setEnabled(false);
        m_aircraft->multiMotor6->setEnabled(false);
        m_aircraft->multiMotor7->setEnabled(false);
        m_aircraft->multiMotor8->setEnabled(false);
        m_aircraft->triYawChannel->setEnabled(false);
                m_aircraft->mrRollMixLevel->setValue(100);
        m_aircraft->mrPitchMixLevel->setValue(100);
        m_aircraft->mrYawMixLevel->setValue(50);
    } else if (frameType == "Hexa" || frameType == "Hexacopter") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Hexacopter"));
        quad->setElementId("quad-hexa");
        m_aircraft->multiMotor4->setEnabled(true);
        m_aircraft->multiMotor5->setEnabled(true);
        m_aircraft->multiMotor6->setEnabled(true);
        m_aircraft->multiMotor7->setEnabled(false);
        m_aircraft->multiMotor8->setEnabled(false);
        m_aircraft->triYawChannel->setEnabled(false);
        m_aircraft->mrRollMixLevel->setValue(50);
        m_aircraft->mrPitchMixLevel->setValue(33);
        m_aircraft->mrYawMixLevel->setValue(33);
    } else if (frameType == "Octo" || frameType == "Octocopter") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Octocopter"));
        quad->setElementId("quad-octo");
        m_aircraft->multiMotor4->setEnabled(true);
        m_aircraft->multiMotor5->setEnabled(true);
        m_aircraft->multiMotor6->setEnabled(true);
        m_aircraft->multiMotor7->setEnabled(true);
        m_aircraft->multiMotor8->setEnabled(true);
        m_aircraft->triYawChannel->setEnabled(false);
        m_aircraft->mrRollMixLevel->setValue(33);
        m_aircraft->mrPitchMixLevel->setValue(33);
        m_aircraft->mrYawMixLevel->setValue(25);
    } else if (frameType == "HexaX" || frameType == "Hexacopter X" ) {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Hexacopter X"));
        quad->setElementId("quad-hexa-X");
        m_aircraft->multiMotor4->setEnabled(true);
        m_aircraft->multiMotor5->setEnabled(true);
        m_aircraft->multiMotor6->setEnabled(true);
        m_aircraft->multiMotor7->setEnabled(false);
        m_aircraft->multiMotor8->setEnabled(false);
        m_aircraft->triYawChannel->setEnabled(false);
        m_aircraft->mrRollMixLevel->setValue(33);
        m_aircraft->mrPitchMixLevel->setValue(50);
        m_aircraft->mrYawMixLevel->setValue(33);

    } else if (frameType == "OctoV" || frameType == "Octocopter V") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Octocopter V"));
        quad->setElementId("quad-octo-v");
        m_aircraft->multiMotor4->setEnabled(true);
        m_aircraft->multiMotor5->setEnabled(true);
        m_aircraft->multiMotor6->setEnabled(true);
        m_aircraft->multiMotor7->setEnabled(true);
        m_aircraft->multiMotor8->setEnabled(true);
        m_aircraft->triYawChannel->setEnabled(false);
        m_aircraft->mrRollMixLevel->setValue(25);
        m_aircraft->mrPitchMixLevel->setValue(25);
        m_aircraft->mrYawMixLevel->setValue(25);

    } else if (frameType == "OctoCoaxP" || frameType == "Octo Coax +") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Octo Coax +"));
        quad->setElementId("octo-coax-P");
        m_aircraft->multiMotor4->setEnabled(true);
        m_aircraft->multiMotor5->setEnabled(true);
        m_aircraft->multiMotor6->setEnabled(true);
        m_aircraft->multiMotor7->setEnabled(true);
        m_aircraft->multiMotor8->setEnabled(true);
        m_aircraft->triYawChannel->setEnabled(false);
        m_aircraft->mrRollMixLevel->setValue(100);
        m_aircraft->mrPitchMixLevel->setValue(100);
        m_aircraft->mrYawMixLevel->setValue(50);

    } else if (frameType == "OctoCoaxX" || frameType == "Octo Coax X") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Octo Coax X"));
        quad->setElementId("octo-coax-X");
        m_aircraft->multiMotor4->setEnabled(true);
        m_aircraft->multiMotor5->setEnabled(true);
        m_aircraft->multiMotor6->setEnabled(true);
        m_aircraft->multiMotor7->setEnabled(true);
        m_aircraft->multiMotor8->setEnabled(true);
        m_aircraft->triYawChannel->setEnabled(false);
        m_aircraft->mrRollMixLevel->setValue(50);
        m_aircraft->mrPitchMixLevel->setValue(50);
        m_aircraft->mrYawMixLevel->setValue(50);

    } else if (frameType == "HexaCoax" || frameType == "Hexacopter Y6") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Hexacopter Y6"));
        quad->setElementId("hexa-coax");
        m_aircraft->multiMotor4->setEnabled(true);
        m_aircraft->multiMotor5->setEnabled(true);
        m_aircraft->multiMotor6->setEnabled(true);
        m_aircraft->multiMotor7->setEnabled(false);
        m_aircraft->multiMotor8->setEnabled(false);
        m_aircraft->triYawChannel->setEnabled(false);
        m_aircraft->mrRollMixLevel->setValue(100);
        m_aircraft->mrPitchMixLevel->setValue(50);
        m_aircraft->mrYawMixLevel->setValue(66);

    } else if (frameType == "Tri" || frameType == "Tricopter Y") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Multirotor"));
        m_aircraft->multirotorFrameType->setCurrentIndex(m_aircraft->multirotorFrameType->findText("Tricopter Y"));
        quad->setElementId("tri");
        m_aircraft->multiMotor4->setEnabled(false);
        m_aircraft->multiMotor5->setEnabled(false);
        m_aircraft->multiMotor6->setEnabled(false);
        m_aircraft->multiMotor7->setEnabled(false);
        m_aircraft->multiMotor8->setEnabled(false);
        m_aircraft->triYawChannel->setEnabled(true);

    }
    m_aircraft->quadShape->setSceneRect(quad->boundingRect());
    m_aircraft->quadShape->fitInView(quad, Qt::KeepAspectRatio);
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
        (m_aircraft->fwRudder1Channel->currentText() == "None"))) {
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
    field = obj->getField("FixedWingYaw1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwRudder1Channel->currentText());
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
    field->setValue(127, ti);

    // Rudder
    eng = m_aircraft->fwRudder1Channel->currentIndex()-1;
    // eng will be -1 if rudder is set to "None"
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Yaw");
        field->setValue(127, ti);
    } // Else: we have no rudder, only ailerons, we're fine with it.

    // Ailerons
    eng = m_aircraft->fwAileron1Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Roll");
        field->setValue(127, ti);
        // Only set Aileron 2 if Aileron 1 is defined
        eng = m_aircraft->fwAileron2Channel->currentIndex()-1;
        if (eng > -1) {
            field = obj->getField(mixerTypes.at(eng));
            field->setValue("Servo");
            field = obj->getField(mixerVectors.at(eng));
            resetField(field);
            ti = field->getElementNames().indexOf("Roll");
            field->setValue(127, ti);
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
    field->setValue(127, ti);
    // Only set Elevator 2 if it is defined
    eng = m_aircraft->fwElevator2Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Pitch");
        field->setValue(127, ti);
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
    // Rudder 1 (can be None)
    field = obj->getField("FixedWingYaw1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwRudder1Channel->currentText());
    // Rudder 2 (can be None)
    field = obj->getField("FixedWingYaw2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwRudder2Channel->currentText());
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
    field->setValue(127, ti);

    // Rudder 1
    eng = m_aircraft->fwRudder1Channel->currentIndex()-1;
    // eng will be -1 if rudder 1 is set to "None"
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Yaw");
        field->setValue(127, ti);
    } // Else: we have no rudder, only elevons, we're fine with it.

    // Rudder 2
    eng = m_aircraft->fwRudder2Channel->currentIndex()-1;
    // eng will be -1 if rudder 2 is set to "None"
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Yaw");
        field->setValue(-127, ti);
    } // Else: we have no rudder, only elevons, we're fine with it.

    eng = m_aircraft->fwAileron1Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Pitch");
        field->setValue((double)m_aircraft->elevonSlider2->value()*1.27, ti);
        ti = field->getElementNames().indexOf("Roll");
        field->setValue((double)m_aircraft->elevonSlider1->value()*1.27,ti);
    }

    eng = m_aircraft->fwAileron2Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Pitch");
        field->setValue((double)m_aircraft->elevonSlider2->value()*1.27, ti);
        ti = field->getElementNames().indexOf("Roll");
        field->setValue(-(double)m_aircraft->elevonSlider1->value()*1.27,ti);
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
    field->setValue(127, ti);

    eng = m_aircraft->fwAileron1Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Roll");
        field->setValue(127,ti);
    }

    eng = m_aircraft->fwAileron2Channel->currentIndex()-1;
    if (eng > -1) {
        field = obj->getField(mixerTypes.at(eng));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(eng));
        resetField(field);
        ti = field->getElementNames().indexOf("Roll");
        field->setValue(-127,ti);
    }

    // Now compute the VTail
    eng = m_aircraft->fwElevator1Channel->currentIndex()-1;
    field = obj->getField(mixerTypes.at(eng));
    field->setValue("Servo");
    field = obj->getField(mixerVectors.at(eng));
    resetField(field);
    ti = field->getElementNames().indexOf("Pitch");
    field->setValue((double)m_aircraft->elevonSlider2->value()*1.27, ti);
    ti = field->getElementNames().indexOf("Yaw");
    field->setValue((double)m_aircraft->elevonSlider1->value()*1.27,ti);

    eng = m_aircraft->fwElevator2Channel->currentIndex()-1;
    field = obj->getField(mixerTypes.at(eng));
    field->setValue("Servo");
    field = obj->getField(mixerVectors.at(eng));
    resetField(field);
    ti = field->getElementNames().indexOf("Pitch");
    field->setValue((double)m_aircraft->elevonSlider2->value()*1.27, ti);
    ti = field->getElementNames().indexOf("Yaw");
    field->setValue(-(double)m_aircraft->elevonSlider1->value()*1.27,ti);

    obj->updated();
    m_aircraft->fwStatusLabel->setText("Mixer generated");
    return true;
}

/**
  Set up a complete mixer, taking a table of factors. The factors
  shoudl mainly be +/- 1 factors, since they will be weighted by the
  value of the Pitch/Roll/Yaw sliders.

  Example:
    double xMixer [8][3] =  {
           P   R   Y
         { 1,  1, -1},  Motor 1
         { 1, -1,  1},  Motor 2
         {-1, -1, -1},  Motor 3
         {-1,  1,  1},  ...
         { 0,  0,  0},
         { 0,  0,  0},
         { 0,  0,  0},
         { 0,  0,  0}
     };
  */
bool ConfigAirframeWidget::setupMixer(double mixerFactors[8][3])
{
    UAVObjectField *field;
    QList<QComboBox*> mmList;
    mmList << m_aircraft->multiMotor1 << m_aircraft->multiMotor2 << m_aircraft->multiMotor3
            << m_aircraft->multiMotor4 << m_aircraft->multiMotor5 << m_aircraft->multiMotor6
            << m_aircraft->multiMotor7 << m_aircraft->multiMotor8;
    UAVDataObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    // 1. Assign the servo/motor/none for each channel
    // Disable all
    foreach(QString mixer, mixerTypes) {
        field = obj->getField(mixer);
        Q_ASSERT(field);
        field->setValue("Disabled");
    }
    // and enable only the relevant channels:
    double pFactor = (double)m_aircraft->mrPitchMixLevel->value()/100;
    double rFactor = (double)m_aircraft->mrRollMixLevel->value()/100;
    double yFactor = (double)m_aircraft->mrYawMixLevel->value()/100;
    for (int i=0 ; i<8; i++) {
        int channel = mmList.at(i)->currentIndex()-1;
        if (channel > -1)
            setupQuadMotor(channel, mixerFactors[i][0]*pFactor,
                       rFactor*mixerFactors[i][1], yFactor*mixerFactors[i][2]);
    }
    obj->updated();
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
    field->setValue(127, ti);
    ti = field->getElementNames().indexOf("Roll");
    field->setValue(roll*127,ti);
    ti = field->getElementNames().indexOf("Pitch");
    field->setValue(pitch*127,ti);
    ti = field->getElementNames().indexOf("Yaw");
    field->setValue(yaw*127,ti);
}

/**
  Helper function: setup motors. Takes a list of channel names in input.
  */
void ConfigAirframeWidget::setupMotors(QList<QString> motorList)
{
    resetActuators();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
    UAVObjectField *field;
    QList<QComboBox*> mmList;
    mmList << m_aircraft->multiMotor1 << m_aircraft->multiMotor2 << m_aircraft->multiMotor3
            << m_aircraft->multiMotor4 << m_aircraft->multiMotor5 << m_aircraft->multiMotor6
            << m_aircraft->multiMotor7 << m_aircraft->multiMotor8;
    foreach (QString motor, motorList) {
        field = obj->getField(motor);
        field->setValue(mmList.takeFirst()->currentText());
    }
    obj->updated(); // Save...
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


    QList<QString> motorList;
    if (pLayout) {
        motorList << "VTOLMotorN" << "VTOLMotorE" << "VTOLMotorS"
                << "VTOLMotorW";
    } else {
        motorList << "VTOLMotorNW" << "VTOLMotorNE" << "VTOLMotorSE"
                << "VTOLMotorSW";
    }
    setupMotors(motorList);

    // Now, setup the mixer:
    // Motor 1 to 4, X Layout:
    //     pitch   roll    yaw
    //    {0.5    ,0.5    ,-0.5     //Front left motor (CW)
    //    {0.5    ,-0.5   ,0.5   //Front right motor(CCW)
    //    {-0.5  ,-0.5    ,-0.5    //rear right motor (CW)
    //    {-0.5   ,0.5    ,0.5   //Rear left motor  (CCW)
    double xMixer [8][3] =  {
         { 1,  1, -1},
         { 1, -1,  1},
         {-1, -1, -1},
         {-1,  1,  1},
         { 0,  0,  0},
         { 0,  0,  0},
         { 0,  0,  0},
         { 0,  0,  0}
     };
    //
    // Motor 1 to 4, P Layout:
    // pitch   roll    yaw
    //  {1      ,0      ,-0.5    //Front motor (CW)
    //  {0      ,-1     ,0.5   //Right  motor(CCW)
    //  {-1     ,0      ,-0.5    //Rear motor  (CW)
    //  {0      ,1      ,0.5   //Left motor  (CCW)
    double pMixer [8][3] =  {
         { 1,  0, -1},
         { 0, -1,  1},
         {-1,  0, -1},
         { 0,  1,  1},
         { 0,  0,  0},
         { 0,  0,  0},
         { 0,  0,  0},
         { 0,  0,  0}
     };

    if (pLayout) {
        setupMixer(pMixer);
    } else {
        setupMixer(xMixer);
    }
    m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
    return true;
}



/**
  Set up a Hexa-X or Hexa-P
*/
bool ConfigAirframeWidget::setupHexa(bool pLayout)
{
    // Check coherence:
    // - Four engines have to be defined
    if (m_aircraft->multiMotor1->currentText() == "None" ||
        m_aircraft->multiMotor2->currentText() == "None" ||
        m_aircraft->multiMotor3->currentText() == "None" ||
        m_aircraft->multiMotor4->currentText() == "None" ||
        m_aircraft->multiMotor5->currentText() == "None" ||
        m_aircraft->multiMotor6->currentText() == "None") {
        m_aircraft->mrStatusLabel->setText("ERROR: Assign 6 motor channels");
        return false;
    }

    QList<QString> motorList;
    if (pLayout) {
        motorList << "VTOLMotorN" << "VTOLMotorNE" << "VTOLMotorSE"
                << "VTOLMotorS" << "VTOLMotorSW" << "VTOLMotorNW";
    } else {
        motorList << "VTOLMotorNE" << "VTOLMotorE" << "VTOLMotorSE"
                << "VTOLMotorSW" << "VTOLMotorW" << "VTOLMotorNW";
    }
    setupMotors(motorList);

    // and set only the relevant channels:

    // Motor 1 to 6, P Layout:
    //     pitch   roll    yaw
    //  1 { 0.3  , 0      ,-0.3 // N   CW
    //  2 { 0.3  ,-0.5    , 0.3 // NE CCW
    //  3 {-0.3  ,-0.5    ,-0.3 // SE  CW
    //  4 {-0.3  , 0      , 0.3 // S  CCW
    //  5 {-0.3  , 0.5    ,-0.3 // SW  CW
    //  6 { 0.3  , 0.5    , 0.3 // NW CCW

   double pMixer [8][3] =  {
        { 1,  0, -1},
        { 1, -1,  1},
        {-1, -1, -1},
        {-1,  0,  1},
        {-1,  1, -1},
        { 1,  1,  1},
        { 0,  0,  0},
        { 0,  0,  0}
    };

   //
    // Motor 1 to 6, X Layout:
    // 1 [  0.5, -0.3, -0.3 ] NE
    // 2 [  0  , -0.3,  0.3 ] E
    // 3 [ -0.5, -0.3, -0.3 ] SE
    // 4 [ -0.5,  0.3,  0.3 ] SW
    // 5 [  0  ,  0.3, -0.3 ] W
    // 6 [  0.5,  0.3,  0.3 ] NW
   double xMixer [8][3] = {
       {  1, -1, -1},
       {  0, -1,  1},
       { -1, -1, -1},
       { -1,  1,  1},
       {  0,  1, -1},
       {  1,  1,  1},
       {  0,  0,  0},
       {  0,  0,  0}
   };

   if (pLayout) {
       setupMixer(pMixer);
   } else {
       setupMixer(xMixer);
   }
   m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
   return true;
}

/**
  Updates the custom airframe settings based on the current airframe.

  Note: does NOT ask for an object refresh itself!
  */
void ConfigAirframeWidget::updateCustomAirframeUI()
{
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    UAVObjectField* field = obj->getField(QString("ThrottleCurve1"));
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
    m_aircraft->customThrottle1Curve->initCurve(curveValues);

    field = obj->getField(QString("ThrottleCurve2"));
    curveValues.clear();;
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
    m_aircraft->customThrottle2Curve->initCurve(curveValues);

    // Retrieve Feed Forward:
    field = obj->getField(QString("FeedForward"));
    m_aircraft->customFFSlider->setValue(field->getDouble()*100);
    field = obj->getField(QString("AccelTime"));
    m_aircraft->customFFaccel->setValue(field->getDouble());
    field = obj->getField(QString("DecelTime"));
    m_aircraft->customFFdecel->setValue(field->getDouble());
    field = obj->getField(QString("MaxAccel"));
    m_aircraft->customFFMaxAccel->setValue(field->getDouble());

    // Update the table:
    for (int i=0; i<8; i++) {
        field = obj->getField(mixerTypes.at(i));
        QComboBox* q = (QComboBox*)m_aircraft->customMixerTable->cellWidget(0,i);
        QString s = field->getValue().toString();
        q->setCurrentIndex(q->findText(s));
        //bool en = (s != "Disabled");
        field = obj->getField(mixerVectors.at(i));
        int ti = field->getElementNames().indexOf("ThrottleCurve1");
        m_aircraft->customMixerTable->item(1,i)->setText(field->getValue(ti).toString());
        ti = field->getElementNames().indexOf("ThrottleCurve2");
        m_aircraft->customMixerTable->item(2,i)->setText(field->getValue(ti).toString());
        ti = field->getElementNames().indexOf("Roll");
        m_aircraft->customMixerTable->item(3,i)->setText(field->getValue(ti).toString());
        ti = field->getElementNames().indexOf("Pitch");
        m_aircraft->customMixerTable->item(4,i)->setText(field->getValue(ti).toString());
        ti = field->getElementNames().indexOf("Yaw");
        m_aircraft->customMixerTable->item(5,i)->setText(field->getValue(ti).toString());
    }
}


/**
  Sends the config to the board (airframe type)

  We do all the tasks common to all airframes, or family of airframes, and
  we call additional methods for specific frames, so that we do not have a code
  that is too heavy.
*/
void ConfigAirframeWidget::sendAircraftUpdate()
{
    QString airframeType = "Custom";
    if (m_aircraft->aircraftType->currentText() == "Fixed Wing") {
        // Save the curve (common to all Fixed wing frames)
        UAVDataObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
        // Remove Feed Forward, it is pointless on a plane:
        UAVObjectField* field = obj->getField(QString("FeedForward"));
        field->setDouble(0);
        field = obj->getField("ThrottleCurve1");
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
        } else { // "Vtail"
            airframeType = "FixedWingVtail";
            setupFrameVtail();
        }
        // Now reflect those settings in the "Custom" panel as well
        updateCustomAirframeUI();
     } else if (m_aircraft->aircraftType->currentText() == "Multirotor") {

         QList<QString> motorList;

        // We can already setup the feedforward here, as it is common to all platforms
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
        UAVObjectField* field = obj->getField(QString("FeedForward"));
        field->setDouble((double)m_aircraft->feedForwardSlider->value()/100);
        field = obj->getField(QString("AccelTime"));
        field->setDouble(m_aircraft->accelTime->value());
        field = obj->getField(QString("DecelTime"));
        field->setDouble(m_aircraft->decelTime->value());
        field = obj->getField(QString("MaxAccel"));
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
            setupHexa(true);
        } else if (m_aircraft->multirotorFrameType->currentText() == "Octocopter") {
            airframeType = "Octo";
            if (m_aircraft->multiMotor1->currentText() == "None" ||
                m_aircraft->multiMotor2->currentText() == "None" ||
                m_aircraft->multiMotor3->currentText() == "None" ||
                m_aircraft->multiMotor4->currentText() == "None" ||
                m_aircraft->multiMotor5->currentText() == "None" ||
                m_aircraft->multiMotor6->currentText() == "None" ||
                m_aircraft->multiMotor7->currentText() == "None" ||
                m_aircraft->multiMotor8->currentText() == "None") {
                m_aircraft->mrStatusLabel->setText("ERROR: Assign 8 motor channels");
                return;
            }
            motorList << "VTOLMotorN" << "VTOLMotorNE" << "VTOLMotorE" << "VTOLMotorSE"
                    << "VTOLMotorS" << "VTOLMotorSW" << "VTOLMotorW" << "VTOLMotorNW";
            setupMotors(motorList);
            // Motor 1 to 8:
            //     pitch   roll    yaw
           double mixer [8][3] = {
               {  1,  0, -1},
               {  1, -1,  1},
               {  0, -1, -1},
               { -1, -1,  1},
               { -1,  0, -1},
               { -1,  1,  1},
               {  0,  1, -1},
               {  1,  1,  1}
           };
           setupMixer(mixer);
           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");

        } else if (m_aircraft->multirotorFrameType->currentText() == "Hexacopter X") {
            airframeType = "HexaX";
            setupHexa(false);
        } else if (m_aircraft->multirotorFrameType->currentText() == "Octocopter V") {
            airframeType = "OctoV";
            if (m_aircraft->multiMotor1->currentText() == "None" ||
                m_aircraft->multiMotor2->currentText() == "None" ||
                m_aircraft->multiMotor3->currentText() == "None" ||
                m_aircraft->multiMotor4->currentText() == "None" ||
                m_aircraft->multiMotor5->currentText() == "None" ||
                m_aircraft->multiMotor6->currentText() == "None" ||
                m_aircraft->multiMotor7->currentText() == "None" ||
                m_aircraft->multiMotor8->currentText() == "None") {
                m_aircraft->mrStatusLabel->setText("ERROR: Assign 8 motor channels");
                return;
            }
            motorList << "VTOLMotorN" << "VTOLMotorNE" << "VTOLMotorE" << "VTOLMotorSE"
                    << "VTOLMotorS" << "VTOLMotorSW" << "VTOLMotorW" << "VTOLMotorNW";
            setupMotors(motorList);
            // Motor 1 to 8:
            // IMPORTANT: Assumes evenly spaced engines
            //     pitch   roll    yaw
           double mixer [8][3] = {
               {  0.33, -1, -1},
               {  1   , -1,  1},
               { -1   , -1, -1},
               { -0.33, -1,  1},
               { -0.33,  1, -1},
               { -1   ,  1,  1},
               {  1   ,  1, -1},
               {  0.33,  1,  1}
           };
           setupMixer(mixer);
           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");

        } else if (m_aircraft->multirotorFrameType->currentText() == "Octo Coax +") {
            airframeType = "OctoCoaxP";
            if (m_aircraft->multiMotor1->currentText() == "None" ||
                m_aircraft->multiMotor2->currentText() == "None" ||
                m_aircraft->multiMotor3->currentText() == "None" ||
                m_aircraft->multiMotor4->currentText() == "None" ||
                m_aircraft->multiMotor5->currentText() == "None" ||
                m_aircraft->multiMotor6->currentText() == "None" ||
                m_aircraft->multiMotor7->currentText() == "None" ||
                m_aircraft->multiMotor8->currentText() == "None") {
                m_aircraft->mrStatusLabel->setText("ERROR: Assign 8 motor channels");
                return;
            }
            motorList << "VTOLMotorN" << "VTOLMotorNE" << "VTOLMotorE" << "VTOLMotorSE"
                    << "VTOLMotorS" << "VTOLMotorSW" << "VTOLMotorW" << "VTOLMotorNW";
            setupMotors(motorList);
            // Motor 1 to 8:
            //     pitch   roll    yaw
           double mixer [8][3] = {
               {  1,  0, -1},
               {  1,  0,  1},
               {  0, -1, -1},
               {  0, -1,  1},
               { -1,  0, -1},
               { -1,  0,  1},
               {  0,  1, -1},
               {  0,  1,  1}
           };
           setupMixer(mixer);
           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");

        } else if (m_aircraft->multirotorFrameType->currentText() == "Octo Coax X") {
            airframeType = "OctoCoaxX";
            if (m_aircraft->multiMotor1->currentText() == "None" ||
                m_aircraft->multiMotor2->currentText() == "None" ||
                m_aircraft->multiMotor3->currentText() == "None" ||
                m_aircraft->multiMotor4->currentText() == "None" ||
                m_aircraft->multiMotor5->currentText() == "None" ||
                m_aircraft->multiMotor6->currentText() == "None" ||
                m_aircraft->multiMotor7->currentText() == "None" ||
                m_aircraft->multiMotor8->currentText() == "None") {
                m_aircraft->mrStatusLabel->setText("ERROR: Assign 8 motor channels");
                return;
            }
            motorList << "VTOLMotorNW" << "VTOLMotorN" << "VTOLMotorNE" << "VTOLMotorE"
                    << "VTOLMotorSE" << "VTOLMotorS" << "VTOLMotorSW" << "VTOLMotorW";
            setupMotors(motorList);
            // Motor 1 to 8:
            //     pitch   roll    yaw
           double mixer [8][3] = {
               {  1,  1, -1},
               {  1,  1,  1},
               {  1, -1, -1},
               {  1, -1,  1},
               { -1, -1, -1},
               { -1, -1,  1},
               { -1,  1, -1},
               { -1,  1,  1}
           };
           setupMixer(mixer);
           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");

        } else if (m_aircraft->multirotorFrameType->currentText() == "Hexacopter Y6") {
            airframeType = "HexaCoax";
            if (m_aircraft->multiMotor1->currentText() == "None" ||
                m_aircraft->multiMotor2->currentText() == "None" ||
                m_aircraft->multiMotor3->currentText() == "None" ||
                m_aircraft->multiMotor4->currentText() == "None" ||
                m_aircraft->multiMotor5->currentText() == "None" ||
                m_aircraft->multiMotor6->currentText() == "None" ) {
                m_aircraft->mrStatusLabel->setText("ERROR: Assign 6 motor channels");
                return;
            }
            motorList << "VTOLMotorNW" << "VTOLMotorW" << "VTOLMotorNE" << "VTOLMotorE"
                    << "VTOLMotorS" << "VTOLMotorSE";
            setupMotors(motorList);

            // Motor 1 to 6, Y6 Layout:
            //     pitch   roll    yaw
           double mixer [8][3] = {
               {  0.5,  1, -1},
               {  0.5,  1,  1},
               {  0.5, -1, -1},
               {  0.5, -1,  1},
               { -1,    0, -1},
               { -1,    0,  1},
               {  0,    0,  0},
               {  0,    0,  0}
           };
           setupMixer(mixer);
           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");

        } else if (m_aircraft->multirotorFrameType->currentText() == "Tricopter Y") {
            airframeType = "Tri";
            if (m_aircraft->multiMotor1->currentText() == "None" ||
                m_aircraft->multiMotor2->currentText() == "None" ||
                m_aircraft->multiMotor3->currentText() == "None" ) {
                m_aircraft->mrStatusLabel->setText("ERROR: Assign 3 motor channels");
                return;
            }
            if (m_aircraft->triYawChannel->currentText() == "None") {
                m_aircraft->mrStatusLabel->setText("Error: Assign a Yaw channel");
                return;
            }
            obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
            Q_ASSERT(obj);
            field = obj->getField("FixedWingYaw1");
            field->setValue(m_aircraft->triYawChannel->currentText());
            // No need to send a obj->updated() here because setupMotors
            // will do it.
            motorList << "VTOLMotorNW" << "VTOLMotorNE" << "VTOLMotorS";
            setupMotors(motorList);

            // Motor 1 to 6, Y6 Layout:
            //     pitch   roll    yaw
           double mixer [8][3] = {
               {  0.5,  1,  0},
               {  0.5, -1,  0},
               { -1,  0,  0},
               {  0,  0,  0},
               {  0,  0,  0},
               {  0,  0,  0},
               {  0,  0,  0},
               {  0,  0,  0}
           };
           setupMixer(mixer);

           int eng = m_aircraft->triYawChannel->currentIndex()-1;
           obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
           field = obj->getField(mixerTypes.at(eng));
           field->setValue("Servo");
           field = obj->getField(mixerVectors.at(eng));
           resetField(field);
           int ti = field->getElementNames().indexOf("Yaw");
           field->setValue(127,ti);

           m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");

        }
        // Now reflect those settings in the "Custom" panel as well
        updateCustomAirframeUI();

    } else  if (m_aircraft->aircraftType->currentText() == "Helicopter") {
        airframeType = "HeliCP";
        m_aircraft->widget_3->sendccpmUpdate();
    } else {
        airframeType = "Custom";

        UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
        UAVObjectField* field = obj->getField(QString("FeedForward"));
        field->setDouble((double)m_aircraft->customFFSlider->value()/100);
        field = obj->getField(QString("AccelTime"));
        field->setDouble(m_aircraft->customFFaccel->value());
        field = obj->getField(QString("DecelTime"));
        field->setDouble(m_aircraft->customFFdecel->value());
        field = obj->getField(QString("MaxAccel"));
        field->setDouble(m_aircraft->customFFMaxAccel->value());

        // Curve is also common to all quads:
        field = obj->getField("ThrottleCurve1");
        QList<double> curve = m_aircraft->customThrottle1Curve->getCurve();
        for (int i=0;i<curve.length();i++) {
            field->setValue(curve.at(i),i);
        }

        field = obj->getField("ThrottleCurve2");
        curve.clear();
        curve = m_aircraft->customThrottle2Curve->getCurve();
        for (int i=0;i<curve.length();i++) {
            field->setValue(curve.at(i),i);
        }

        // Update the table:
        for (int i=0; i<8; i++) {
            field = obj->getField(mixerTypes.at(i));
            QComboBox* q = (QComboBox*)m_aircraft->customMixerTable->cellWidget(0,i);
            field->setValue(q->currentText());
            field = obj->getField(mixerVectors.at(i));
            int ti = field->getElementNames().indexOf("ThrottleCurve1");
            field->setValue(m_aircraft->customMixerTable->item(1,i)->text(),ti);
            ti = field->getElementNames().indexOf("ThrottleCurve2");
            field->setValue(m_aircraft->customMixerTable->item(2,i)->text(),ti);
            ti = field->getElementNames().indexOf("Roll");
            field->setValue(m_aircraft->customMixerTable->item(3,i)->text(),ti);
            ti = field->getElementNames().indexOf("Pitch");
            field->setValue(m_aircraft->customMixerTable->item(4,i)->text(),ti);
            ti = field->getElementNames().indexOf("Yaw");
            field->setValue(m_aircraft->customMixerTable->item(5,i)->text(),ti);
        }

        obj->updated();
    }

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("SystemSettings")));
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

