/**
 ******************************************************************************
 *
 * @file       configgroundvehiclemwidget.cpp
 * @author     K. Sebesta & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief ccpm configuration panel
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
#include "configgroundvehiclewidget.h"
#include "mixersettings.h"
#include "systemsettings.h"
#include "actuatorsettings.h"
#include "actuatorcommand.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QBrush>
#include <QMessageBox>

#include <math.h>

QStringList ConfigGroundVehicleWidget::getChannelDescriptions()
{
    // init a channel_numelem list of channel desc defaults
    QStringList channelDesc;
    for (int i = 0; i < (int) ConfigGroundVehicleWidget::CHANNEL_NUMELEM; i++) {
        channelDesc.append(QString("-"));
    }

    // get the gui config data
    GUIConfigDataUnion configData = getConfigData();

    if (configData.ground.GroundVehicleSteering1 > 0) {
        channelDesc[configData.ground.GroundVehicleSteering1 - 1] = QString("GroundSteering1");
    }
    if (configData.ground.GroundVehicleSteering2 > 0) {
        channelDesc[configData.ground.GroundVehicleSteering2 - 1] = QString("GroundSteering2");
    }
    if (configData.ground.GroundVehicleThrottle1 > 0) {
        channelDesc[configData.ground.GroundVehicleThrottle1 - 1] = QString("GroundThrottle1");
    }
    if (configData.ground.GroundVehicleThrottle2 > 0) {
        channelDesc[configData.ground.GroundVehicleThrottle2 - 1] = QString("GroundThrottle2");
    }
    return channelDesc;
}

ConfigGroundVehicleWidget::ConfigGroundVehicleWidget(QWidget *parent) :
        VehicleConfig(parent), m_aircraft(new Ui_GroundConfigWidget())
{
    m_aircraft->setupUi(this);

    populateChannelComboBoxes();

    QStringList groundVehicleTypes;
    groundVehicleTypes << "Turnable (car)" << "Differential (tank)" << "Motorcycle";
    m_aircraft->groundVehicleType->addItems(groundVehicleTypes);

    // Set default model to "Turnable (car)"
    connect(m_aircraft->groundVehicleType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupUI(QString)));
    m_aircraft->groundVehicleType->setCurrentIndex(m_aircraft->groundVehicleType->findText("Turnable (car)"));
    setupUI(m_aircraft->groundVehicleType->currentText());
}

ConfigGroundVehicleWidget::~ConfigGroundVehicleWidget()
{
    delete m_aircraft;
}

/**
 Virtual function to setup the UI
 */
void ConfigGroundVehicleWidget::setupUI(QString frameType)
{
    // Setup the UI

    m_aircraft->gvEngineChannelBox->setEnabled(false);
    m_aircraft->gvAileron1ChannelBox->setEnabled(false);
    m_aircraft->gvAileron2ChannelBox->setEnabled(false);

    m_aircraft->differentialSteeringSlider1->setEnabled(false);
    m_aircraft->differentialSteeringSlider2->setEnabled(false);

    if (frameType == "GroundVehicleDifferential" || frameType == "Differential (tank)") {
        // Tank
        setComboCurrentIndex(m_aircraft->groundVehicleType,
                m_aircraft->groundVehicleType->findText("Differential (tank)"));
        m_aircraft->gvMotor1ChannelBox->setEnabled(true);
        m_aircraft->gvMotor2ChannelBox->setEnabled(true);

        m_aircraft->gvMotor1Label->setText("Left motor");
        m_aircraft->gvMotor2Label->setText("Right motor");

        m_aircraft->gvSteering1ChannelBox->setEnabled(false);
        m_aircraft->gvSteering2ChannelBox->setEnabled(false);

        m_aircraft->gvSteering2Label->setText("Rear steering");

        m_aircraft->differentialSteeringSlider1->setEnabled(true);
        m_aircraft->differentialSteeringSlider2->setEnabled(true);

        m_aircraft->gvThrottleCurve1GroupBox->setTitle("Left throttle curve");
        m_aircraft->gvThrottleCurve2GroupBox->setTitle("Right throttle curve");

    } else if (frameType == "GroundVehicleMotorcycle" || frameType == "Motorcycle") {
        // Motorcycle
        setComboCurrentIndex(m_aircraft->groundVehicleType, m_aircraft->groundVehicleType->findText("Motorcycle"));
        m_aircraft->gvMotor1ChannelBox->setEnabled(false);
        m_aircraft->gvMotor2ChannelBox->setEnabled(true);

        m_aircraft->gvMotor1Label->setText("Front motor");
        m_aircraft->gvMotor2Label->setText("Rear motor");

        m_aircraft->gvSteering1ChannelBox->setEnabled(true);
        m_aircraft->gvSteering2ChannelBox->setEnabled(true);

        m_aircraft->gvSteering2Label->setText("Balancing");

        m_aircraft->gvThrottleCurve1GroupBox->setTitle("Front throttle curve");
        m_aircraft->gvThrottleCurve2GroupBox->setTitle("Rear throttle curve");
    } else {
        // Car
        setComboCurrentIndex(m_aircraft->groundVehicleType, m_aircraft->groundVehicleType->findText("Turnable (car)"));

        m_aircraft->gvMotor1ChannelBox->setEnabled(true);
        m_aircraft->gvMotor2ChannelBox->setEnabled(true);

        m_aircraft->gvMotor1Label->setText("Front motor");
        m_aircraft->gvMotor2Label->setText("Rear motor");

        m_aircraft->gvSteering1ChannelBox->setEnabled(true);
        m_aircraft->gvSteering2ChannelBox->setEnabled(true);

        m_aircraft->gvThrottleCurve1GroupBox->setTitle("Front throttle curve");
        m_aircraft->gvThrottleCurve2GroupBox->setTitle("Rear throttle curve");
    }
}

void ConfigGroundVehicleWidget::enableControls(bool enable)
{
    ConfigTaskWidget::enableControls(enable);
    if(enable) {
        setupUI(m_aircraft->groundVehicleType->currentText());
    }
}

void ConfigGroundVehicleWidget::registerWidgets(ConfigTaskWidget &parent) {
    parent.addWidget(m_aircraft->groundVehicleThrottle1->getCurveWidget());
    parent.addWidget(m_aircraft->groundVehicleThrottle1);
    parent.addWidget(m_aircraft->groundVehicleThrottle2->getCurveWidget());
    parent.addWidget(m_aircraft->groundVehicleThrottle2);
    parent.addWidget(m_aircraft->groundVehicleType);
    parent.addWidget(m_aircraft->gvEngineChannelBox);
    parent.addWidget(m_aircraft->gvAileron1ChannelBox);
    parent.addWidget(m_aircraft->gvAileron2ChannelBox);
    parent.addWidget(m_aircraft->gvMotor1ChannelBox);
    parent.addWidget(m_aircraft->gvMotor2ChannelBox);
    parent.addWidget(m_aircraft->gvSteering1ChannelBox);
    parent.addWidget(m_aircraft->gvSteering2ChannelBox);
}

void ConfigGroundVehicleWidget::resetActuators(GUIConfigDataUnion *configData)
{
    configData->ground.GroundVehicleSteering1 = 0;
    configData->ground.GroundVehicleSteering2 = 0;
    configData->ground.GroundVehicleThrottle1 = 0;
    configData->ground.GroundVehicleThrottle2 = 0;
}

/**
 Virtual function to refresh the UI widget values
 */
void ConfigGroundVehicleWidget::refreshWidgetsValues(QString frameType)
{
    setupUI(frameType);

    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    QList<double> curveValues;
    getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, &curveValues);

    // is at least one of the curve values != 0?
    if (isValidThrottleCurve(&curveValues)) {
        // yes, use the curve we just read from mixersettings
        m_aircraft->groundVehicleThrottle1->initCurve(&curveValues);
    } else {
        // no, init a straight curve
        m_aircraft->groundVehicleThrottle1->initLinearCurve(curveValues.count(), 1.0);
    }

    // Setup all Throttle2 curves for all types of airframes
    getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE2, &curveValues);

    if (isValidThrottleCurve(&curveValues)) {
        m_aircraft->groundVehicleThrottle2->initCurve(&curveValues);
    } else {
        m_aircraft->groundVehicleThrottle2->initLinearCurve(curveValues.count(), 1.0);
    }
    GUIConfigDataUnion config = getConfigData();

    // THIS SECTION STILL NEEDS WORK. FOR THE MOMENT, USE THE FIXED-WING ONBOARD SETTING IN ORDER TO MINIMIZE CHANCES OF BOLLOXING REAL CODE
    // Retrieve channel setup values
    setComboCurrentIndex(m_aircraft->gvMotor1ChannelBox, config.ground.GroundVehicleThrottle1);
    setComboCurrentIndex(m_aircraft->gvMotor2ChannelBox, config.ground.GroundVehicleThrottle2);
    setComboCurrentIndex(m_aircraft->gvSteering1ChannelBox, config.ground.GroundVehicleSteering1);
    setComboCurrentIndex(m_aircraft->gvSteering2ChannelBox, config.ground.GroundVehicleSteering2);

    if (frameType == "GroundVehicleDifferential") {
        //CURRENTLY BROKEN UNTIL WE DECIDE HOW DIFFERENTIAL SHOULD BEHAVE
        // If the vehicle type is "differential", restore the slider setting

        // Find the channel number for Motor1
        //obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
        //Q_ASSERT(obj);
        int channel = m_aircraft->gvMotor1ChannelBox->currentIndex() - 1;
        if (channel > -1) {
            // If for some reason the actuators were incoherent, we might fail here, hence the check.
            m_aircraft->differentialSteeringSlider1->setValue(
                    getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL) * 100);
            m_aircraft->differentialSteeringSlider2->setValue(
                    getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH) * 100);
        }
    } else if (frameType == "GroundVehicleMotorcycle") {
        // CURRENTLY BROKEN UNTIL WE DECIDE HOW MOTORCYCLE SHOULD BEHAVE
//      obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//      Q_ASSERT(obj);
//      int chMixerNumber = m_aircraft->gvMotor1ChannelBox->currentIndex()-1;
//      if (chMixerNumber >=0) {
//          field = obj->getField(mixerVectors.at(chMixerNumber));
//          int ti = field->getElementNames().indexOf("Yaw");
//          m_aircraft->differentialSteeringSlider1->setValue(field->getDouble(ti)*100);
//
//          ti = field->getElementNames().indexOf("Pitch");
//          m_aircraft->differentialSteeringSlider2->setValue(field->getDouble(ti)*100);
//      }
    }
}

/**
 Virtual function to update the UI widget objects
 */
QString ConfigGroundVehicleWidget::updateConfigObjectsFromWidgets()
{
    QString airframeType = "GroundVehicleCar";

    // Save the curve (common to all ground vehicle frames)
    UAVDataObject *mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));

    // Remove Feed Forward, it is pointless on a ground vehicle:
    setMixerValue(mixer, "FeedForward", 0.0);

    // set the throttle curves
    setThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, m_aircraft->groundVehicleThrottle1->getCurve());
    setThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE2, m_aircraft->groundVehicleThrottle2->getCurve());

    // All airframe types must start with "GroundVehicle"
    if (m_aircraft->groundVehicleType->currentText() == "Turnable (car)") {
        airframeType = "GroundVehicleCar";
        setupGroundVehicleCar(airframeType);
    } else if (m_aircraft->groundVehicleType->currentText() == "Differential (tank)") {
        airframeType = "GroundVehicleDifferential";
        setupGroundVehicleDifferential(airframeType);
    } else { // "Motorcycle"
        airframeType = "GroundVehicleMotorcycle";
        setupGroundVehicleMotorcycle(airframeType);
    }

    return airframeType;
}

/**
 Setup balancing ground vehicle.
 
 Returns False if impossible to create the mixer.
 */
bool ConfigGroundVehicleWidget::setupGroundVehicleMotorcycle(QString airframeType)
{
    // Check coherence:
    // Show any config errors in GUI
    if (throwConfigError(airframeType)) {
        return false;
    }

    // Now setup the channels:
    GUIConfigDataUnion config = getConfigData();
    resetActuators(&config);

    config.ground.GroundVehicleThrottle1 = m_aircraft->gvMotor1ChannelBox->currentIndex();
    config.ground.GroundVehicleThrottle2 = m_aircraft->gvMotor2ChannelBox->currentIndex();

    setConfigData(config);

    UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);
    resetMotorAndServoMixers(mixer);

    // motor
    int channel = m_aircraft->gvMotor2ChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE1, 127);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW, 127);

    // steering
    channel = m_aircraft->gvSteering1ChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW, -127);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL, -127);

    // balance
    channel = m_aircraft->gvSteering2ChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW, 127);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL, 127);

    m_aircraft->gvStatusLabel->setText("Mixer generated");

    return true;
}

/**
 Setup differentially steered ground vehicle.
 
 Returns False if impossible to create the mixer.
 */
bool ConfigGroundVehicleWidget::setupGroundVehicleDifferential(QString airframeType)
{
    // Check coherence:
    // Show any config errors in GUI

    if (throwConfigError(airframeType)) {
        return false;
    }

    // Now setup the channels:
    GUIConfigDataUnion config = getConfigData();
    resetActuators(&config);

    config.ground.GroundVehicleThrottle1 = m_aircraft->gvMotor1ChannelBox->currentIndex();
    config.ground.GroundVehicleThrottle2 = m_aircraft->gvMotor2ChannelBox->currentIndex();

    setConfigData(config);

    UAVDataObject *mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);
    resetMotorAndServoMixers(mixer);

    // left motor
    int channel = m_aircraft->gvMotor1ChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE1, 127);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW, 127);

    // right motor
    channel = m_aircraft->gvMotor2ChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE2, 127);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW, -127);

    // Output success message
    m_aircraft->gvStatusLabel->setText("Mixer generated");

    return true;
}

/**
 Setup steerable ground vehicle.
 
 Returns False if impossible to create the mixer.
 */
bool ConfigGroundVehicleWidget::setupGroundVehicleCar(QString airframeType)
{
    // Check coherence:
    // Show any config errors in GUI
    if (throwConfigError(airframeType)) {
        return false;
    }

    // Now setup the channels:
    GUIConfigDataUnion config = getConfigData();
    resetActuators(&config);

    config.ground.GroundVehicleThrottle1 = m_aircraft->gvMotor1ChannelBox->currentIndex();
    config.ground.GroundVehicleThrottle2 = m_aircraft->gvMotor2ChannelBox->currentIndex();
    config.ground.GroundVehicleSteering1 = m_aircraft->gvSteering1ChannelBox->currentIndex();
    config.ground.GroundVehicleSteering2 = m_aircraft->gvSteering2ChannelBox->currentIndex();

    setConfigData(config);

    UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);
    resetMotorAndServoMixers(mixer);

    int channel = m_aircraft->gvSteering1ChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW, 127);

    channel = m_aircraft->gvSteering2ChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW, -127);

    channel = m_aircraft->gvMotor1ChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE1, 127);

    channel = m_aircraft->gvMotor2ChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE2, 127);

    // Output success message
    m_aircraft->gvStatusLabel->setText("Mixer generated");

    return true;
}

/**
 This function displays text and color formatting in order to help the user understand what channels have not yet been configured.
 */
bool ConfigGroundVehicleWidget::throwConfigError(QString airframeType)
{
    // Initialize configuration error flag
    bool error = false;

    // Create a red block. All combo boxes are the same size, so any one should do as a model
    int size = m_aircraft->gvEngineChannelBox->style()->pixelMetric(QStyle::PM_SmallIconSize);
    QPixmap pixmap(size, size);
    pixmap.fill(QColor("red"));

    if (airframeType == "GroundVehicleCar") { //Car
        if (m_aircraft->gvMotor1ChannelBox->currentText() == "None"
                && m_aircraft->gvMotor2ChannelBox->currentText() == "None") {
            pixmap.fill(QColor("green"));
            m_aircraft->gvMotor1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole); //Set color palettes
            m_aircraft->gvMotor2ChannelBox->setItemData(0, pixmap, Qt::DecorationRole); //Set color palettes
            //m_aircraft->gvMotor1Label->setText("<font color='red'>" + m_aircraft->gvMotor1Label->text() + "</font>");
            //m_aircraft->gvMotor2Label->setText("<font color='red'>" + m_aircraft->gvMotor2Label->text() + "</font>");
            error = true;

        } else {
            m_aircraft->gvMotor1ChannelBox->setItemData(0, 0, Qt::DecorationRole);			//Reset color palettes
            m_aircraft->gvMotor2ChannelBox->setItemData(0, 0, Qt::DecorationRole);			//Reset color palettes
            //QTextEdit* htmlText=new QTextEdit(m_aircraft->gvMotor1Label->text());  // HtmlText is any QString with html tags.
            //m_aircraft->gvMotor1Label->setText(htmlText->toPlainText());
            //delete htmlText;

            //htmlText=new QTextEdit(m_aircraft->gvMotor2Label->text());  // HtmlText is any QString with html tags.
            //m_aircraft->gvMotor2Label->setText(htmlText->toPlainText());
        }

        if (m_aircraft->gvSteering1ChannelBox->currentText() == "None"
                && m_aircraft->gvSteering2ChannelBox->currentText() == "None") {
            m_aircraft->gvSteering1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);		//Set color palettes
            m_aircraft->gvSteering2ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);		//Set color palettes
            //m_aircraft->gvStatusLabel->setText("<font color='red'>ERROR: check steering channel assignment</font>");
            //m_aircraft->gvSteering1Label->setText("<font color='red'>" + m_aircraft->gvSteering1Label->text() + "</font>");
            //m_aircraft->gvSteering2Label->setText("<font color='red'>" + m_aircraft->gvSteering2Label->text() + "</font>");
            error = true;
        } else {
            m_aircraft->gvSteering1ChannelBox->setItemData(0, 0, Qt::DecorationRole);			//Reset color palettes
            m_aircraft->gvSteering2ChannelBox->setItemData(0, 0, Qt::DecorationRole);			//Reset color palettes
            //QTextEdit* htmlText=new QTextEdit(m_aircraft->gvSteering1Label->text());  // HtmlText is any QString with html tags.
            //m_aircraft->gvSteering1Label->setText(htmlText->toPlainText());
            //delete htmlText;

            //htmlText=new QTextEdit(m_aircraft->gvSteering2Label->text());  // HtmlText is any QString with html tags.
            //m_aircraft->gvSteering2Label->setText(htmlText->toPlainText());
        }
    } else if (airframeType == "GroundVehicleDifferential") { //Tank
        if (m_aircraft->gvMotor1ChannelBox->currentText() == "None"
                || m_aircraft->gvMotor2ChannelBox->currentText() == "None") {
            m_aircraft->gvMotor1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole); //Set color palettes
            m_aircraft->gvMotor2ChannelBox->setItemData(0, pixmap, Qt::DecorationRole); //Set color palettes
            error = true;
        } else {
            m_aircraft->gvMotor1ChannelBox->setItemData(0, 0, Qt::DecorationRole); //Reset color palettes
            m_aircraft->gvMotor2ChannelBox->setItemData(0, 0, Qt::DecorationRole); //Reset color palettes
        }

        // Always reset
        m_aircraft->gvSteering1ChannelBox->setItemData(0, 0, Qt::DecorationRole); //Reset color palettes
        m_aircraft->gvSteering2ChannelBox->setItemData(0, 0, Qt::DecorationRole); //Reset color palettes
    } else if (airframeType == "GroundVehicleMotorcycle") { //Motorcycle
        if (m_aircraft->gvMotor1ChannelBox->currentText() == "None"
                && m_aircraft->gvMotor2ChannelBox->currentText() == "None") {
            m_aircraft->gvMotor2ChannelBox->setItemData(0, pixmap, Qt::DecorationRole); //Set color palettes
            error = true;
        } else {
            m_aircraft->gvMotor2ChannelBox->setItemData(0, 0, Qt::DecorationRole); //Reset color palettes
        }

        if (m_aircraft->gvSteering1ChannelBox->currentText() == "None"
                && m_aircraft->gvSteering2ChannelBox->currentText() == "None") {
            m_aircraft->gvSteering1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole); //Set color palettes
            error = true;
        } else {
            m_aircraft->gvSteering1ChannelBox->setItemData(0, 0, Qt::DecorationRole); //Reset color palettes
        }

        // Always reset
        m_aircraft->gvMotor1ChannelBox->setItemData(0, 0, Qt::DecorationRole); //Reset color palettes
        m_aircraft->gvSteering2ChannelBox->setItemData(0, 0, Qt::DecorationRole); //Reset color palettes
    }

    if (error) {
        m_aircraft->gvStatusLabel->setText(QString("<font color='red'>ERROR: Assign all necessary channels</font>"));
    }
    return error;
}
