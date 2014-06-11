/**
 ******************************************************************************
 *
 * @file       configfixedwidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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
#include "configfixedwingwidget.h"
#include "mixersettings.h"
#include "systemsettings.h"
#include "actuatorsettings.h"
#include "actuatorcommand.h"

#include <QDebug>
#include <QStringList>
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QBrush>
#include <math.h>
#include <QMessageBox>

const QString ConfigFixedWingWidget::CHANNELBOXNAME = QString("fixedWingChannelBox");

QStringList ConfigFixedWingWidget::getChannelDescriptions()
{
    // init a channel_numelem list of channel desc defaults
    QStringList channelDesc;

    for (int i = 0; i < (int)ConfigFixedWingWidget::CHANNEL_NUMELEM; i++) {
        channelDesc.append(QString("-"));
    }

    // get the gui config data
    GUIConfigDataUnion configData = getConfigData();
    fixedGUISettingsStruct fixed  = configData.fixedwing;

    if (fixed.FixedWingThrottle > 0 && fixed.FixedWingThrottle <= ConfigFixedWingWidget::CHANNEL_NUMELEM) {
        channelDesc[fixed.FixedWingThrottle - 1] = QString("WingThrottle");
    }
    if (fixed.FixedWingPitch1 > 0 && fixed.FixedWingPitch1 <= ConfigFixedWingWidget::CHANNEL_NUMELEM) {
        channelDesc[fixed.FixedWingPitch1 - 1] = QString("FixedWingPitch1");
    }
    if (fixed.FixedWingPitch2 > 0 && fixed.FixedWingPitch2 <= ConfigFixedWingWidget::CHANNEL_NUMELEM) {
        channelDesc[fixed.FixedWingPitch2 - 1] = QString("FixedWingPitch2");
    }
    if (fixed.FixedWingRoll1 > 0 && fixed.FixedWingRoll1 <= ConfigFixedWingWidget::CHANNEL_NUMELEM) {
        channelDesc[fixed.FixedWingRoll1 - 1] = QString("FixedWingRoll1");
    }
    if (fixed.FixedWingRoll2 > 0 && fixed.FixedWingRoll2 <= ConfigFixedWingWidget::CHANNEL_NUMELEM) {
        channelDesc[fixed.FixedWingRoll2 - 1] = QString("FixedWingRoll2");
    }
    if (fixed.FixedWingYaw1 > 0 && fixed.FixedWingYaw1 <= ConfigFixedWingWidget::CHANNEL_NUMELEM) {
        channelDesc[fixed.FixedWingYaw1 - 1] = QString("FixedWingYaw1");
    }
    if (fixed.FixedWingYaw2 > 0 && fixed.FixedWingYaw2 <= ConfigFixedWingWidget::CHANNEL_NUMELEM) {
        channelDesc[fixed.FixedWingYaw2 - 1] = QString("FixedWingYaw2");
    }
    return channelDesc;
}

ConfigFixedWingWidget::ConfigFixedWingWidget(QWidget *parent) :
    VehicleConfig(parent), m_aircraft(new Ui_FixedWingConfigWidget())
{
    m_aircraft->setupUi(this);

    populateChannelComboBoxes();

    QStringList fixedWingTypes;
    fixedWingTypes << "Elevator aileron rudder" << "Elevon";
    m_aircraft->fixedWingType->addItems(fixedWingTypes);

    // Set default model to "Elevator aileron rudder"
    m_aircraft->fixedWingType->setCurrentIndex(m_aircraft->fixedWingType->findText("Elevator aileron rudder"));

    //setupUI(m_aircraft->fixedWingType->currentText());    

    connect(m_aircraft->fixedWingType, SIGNAL(currentIndexChanged(QString)), this, SLOT(setupUI(QString)));
    updateEnableControls();
}

ConfigFixedWingWidget::~ConfigFixedWingWidget()
{
    delete m_aircraft;
}

void ConfigFixedWingWidget::setupUI(QString frameType)
{
    Q_ASSERT(m_aircraft);
    Q_ASSERT(plane);

    // This had to be moved from ConfigFixedWingWidget() here since m_aircraft->fixedWingType->currentText() 
    // did not seem to work properly to choose alternate .svg files. 
    m_aircraft->planeShape->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_aircraft->planeShape->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/configgadget/images/fixedwing-shapes.svg"));
    plane = new QGraphicsSvgItem();
    plane->setSharedRenderer(renderer);

    qDebug() << "Current Aircraft type: \n" << m_aircraft->fixedWingType->currentText();

    if (frameType == "FixedWing" || frameType == "Elevator aileron rudder") {
        plane->setElementId("aileron");
        setComboCurrentIndex(m_aircraft->fixedWingType, m_aircraft->fixedWingType->findText("Elevator aileron rudder"));
        m_aircraft->fwRudder1ChannelBox->setEnabled(true);
        m_aircraft->fwRudder2ChannelBox->setEnabled(true);
        m_aircraft->fwElevator1ChannelBox->setEnabled(true);
        m_aircraft->fwElevator2ChannelBox->setEnabled(true);
        m_aircraft->fwAileron1ChannelBox->setEnabled(true);
        m_aircraft->fwAileron2ChannelBox->setEnabled(true);

        m_aircraft->fwAileron1Label->setText("Aileron 1");
        m_aircraft->fwAileron2Label->setText("Aileron 2");
        m_aircraft->fwElevator1Label->setText("Elevator 1");
        m_aircraft->fwElevator2Label->setText("Elevator 2");

        m_aircraft->elevonSlider1->setEnabled(false);
        m_aircraft->elevonSlider2->setEnabled(false);
    } else if (frameType == "FixedWingVtail" || frameType == "Vtail") {
	// do nothing for now
    } else if (frameType == "FixedWingElevon" || frameType == "Elevon") {
        plane->setElementId("elevon");
        setComboCurrentIndex(m_aircraft->fixedWingType, m_aircraft->fixedWingType->findText("Elevon"));
        m_aircraft->fwRudder1ChannelBox->setEnabled(false);
        m_aircraft->fwRudder2ChannelBox->setEnabled(false);

        m_aircraft->fwElevator1Label->setText("Elevon 1");
        m_aircraft->fwElevator1ChannelBox->setEnabled(true);

        m_aircraft->fwElevator2Label->setText("Elevon 2");
        m_aircraft->fwElevator2ChannelBox->setEnabled(true);

        m_aircraft->fwAileron1Label->setText("Aileron 1");
        m_aircraft->fwAileron2Label->setText("Aileron 2");
        m_aircraft->elevonLabel1->setText("Roll");
        m_aircraft->elevonLabel2->setText("Pitch");

        m_aircraft->elevonSlider1->setEnabled(true);
        m_aircraft->elevonSlider2->setEnabled(true);
    }

    QGraphicsScene *scene = new QGraphicsScene();
    scene->addItem(plane);
    scene->setSceneRect(plane->boundingRect());
    m_aircraft->planeShape->setScene(scene);

    setupEnabledControls(frameType);
    // Draw the appropriate airframe
    updateAirframe(frameType);
}

void ConfigFixedWingWidget::setupEnabledControls(QString frameType)
{

    // disable all motor channel boxes
    for (int i = 1; i <= 8; i++) {
        // do it manually so we can turn off any error decorations
        QComboBox *combobox = this->findChild<QComboBox *>("fixedWingChannelBox" + QString::number(i));
        if (combobox) {
            combobox->setEnabled(false);
            combobox->setItemData(0, 0, Qt::DecorationRole);
        }
    }

    if (frameType == "Vtail" || frameType == "vtail") {
        // enableComboBoxes(this, CHANNELBOXNAME, 3, true);
    } else if (frameType == "Elevon" || frameType == "Elevon") {
        enableComboBoxes(this, CHANNELBOXNAME, 3, true);
    } else if (frameType == "aileron" || frameType == "Elevator aileron rudder") {
        enableComboBoxes(this, CHANNELBOXNAME, 4, true);
    } 
}

void ConfigFixedWingWidget::registerWidgets(ConfigTaskWidget &parent)
{
    parent.addWidget(m_aircraft->fixedWingThrottle->getCurveWidget());
    parent.addWidget(m_aircraft->fixedWingThrottle);
    parent.addWidget(m_aircraft->fixedWingType);

    parent.addWidget(m_aircraft->fwEngineChannelBox);
    parent.addWidget(m_aircraft->fwAileron1ChannelBox);
    parent.addWidget(m_aircraft->fwAileron2ChannelBox);
    parent.addWidget(m_aircraft->fwElevator1ChannelBox);
    parent.addWidget(m_aircraft->fwElevator2ChannelBox);
    parent.addWidget(m_aircraft->fwRudder1ChannelBox);
    parent.addWidget(m_aircraft->fwRudder2ChannelBox);
    parent.addWidget(m_aircraft->elevonSlider1);
    parent.addWidget(m_aircraft->elevonSlider2);
}

void ConfigFixedWingWidget::resetActuators(GUIConfigDataUnion *configData)
{
    configData->fixedwing.FixedWingPitch1   = 0;
    configData->fixedwing.FixedWingPitch2   = 0;
    configData->fixedwing.FixedWingRoll1    = 0;
    configData->fixedwing.FixedWingRoll2    = 0;
    configData->fixedwing.FixedWingYaw1     = 0;
    configData->fixedwing.FixedWingYaw2     = 0;
    configData->fixedwing.FixedWingThrottle = 0;
}

/**
   Virtual function to refresh the UI widget values
 */
void ConfigFixedWingWidget::refreshWidgetsValues(QString frameType)
{
    Q_ASSERT(m_aircraft);

    setupUI(frameType);

    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    QList<double> curveValues;
    getThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, &curveValues);

    // is at least one of the curve values != 0?
    if (isValidThrottleCurve(&curveValues)) {
        // yes, use the curve we just read from mixersettings
        m_aircraft->fixedWingThrottle->initCurve(&curveValues);
    } else {
        // no, init a straight curve
        m_aircraft->fixedWingThrottle->initLinearCurve(curveValues.count(), 1.0);
    }

    GUIConfigDataUnion config    = getConfigData();
    fixedGUISettingsStruct fixed = config.fixedwing;

    // Then retrieve how channels are setup
    setComboCurrentIndex(m_aircraft->fwEngineChannelBox, fixed.FixedWingThrottle);
    setComboCurrentIndex(m_aircraft->fwAileron1ChannelBox, fixed.FixedWingRoll1);
    setComboCurrentIndex(m_aircraft->fwAileron2ChannelBox, fixed.FixedWingRoll2);
    setComboCurrentIndex(m_aircraft->fwElevator1ChannelBox, fixed.FixedWingPitch1);
    setComboCurrentIndex(m_aircraft->fwElevator2ChannelBox, fixed.FixedWingPitch2);

    if (frameType == "FixedWingElevon") {
        // If the airframe is elevon, restore the slider setting
        // Find the channel number for Elevon1 (FixedWingRoll1)
        int channel = m_aircraft->fwElevator1ChannelBox->currentIndex() - 1;
        if (channel > -1) {
            // If for some reason the actuators were incoherent, we might fail here, hence the check.
            m_aircraft->elevonSlider1->setValue(
                getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL) * 100);
            m_aircraft->elevonSlider2->setValue(
                getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH) * 100);
        }
    }

    updateAirframe(frameType);
}

/**
   Helper function to update the UI widget objects
 */
QString ConfigFixedWingWidget::updateConfigObjectsFromWidgets()
{
    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));

    Q_ASSERT(mixer);

    // Curve is also common to all quads:
    setThrottleCurve(mixer, VehicleConfig::MIXER_THROTTLECURVE1, m_aircraft->fixedWingThrottle->getCurve());

    QString airframeType;
    QList<QString> motor_servo_List;

    if (m_aircraft->fixedWingType->currentText() == "Elevator aileron rudder") {
        airframeType = "FixedWing";
        setupFrameFixedWing(airframeType);

        motor_servo_List << "FixedWingThrottle" << "FixedWingPitch1" << "FixedWingPitch2" << "FixedWingRoll1" << "FixedWingRoll2" << "FixedWingYaw1" << "FixedWingYaw2";
        setupMotors(motor_servo_List);
	
	GUIConfigDataUnion config = getConfigData();
	setConfigData(config);

	m_aircraft->fwStatusLabel->setText(tr("Configuration OK"));

    } 
    else if (m_aircraft->fixedWingType->currentText() == "elevon") {
        airframeType = "FixedWingElevon";
        setupFrameElevon(airframeType);

        motor_servo_List << "FixedWingThrottle" << "FixedWingRoll1" << "FixedWingRoll2";
        setupMotors(motor_servo_List);
	
	GUIConfigDataUnion config = getConfigData();
	setConfigData(config);

        // Elevon Layout:
        // pitch   roll    yaw
        double mixerMatrix[8][3] = {
            { 0,   0,  0 },
            { 0,   0,  0 },
            { 0,   0,  0 },
            { 0,   0,  0 },
            { 0,   0,  0 },
            { 0,   0,  0 },
            { 0,   0,  0 },
            { 0,   0,  0 }
        };
	setupFixedWingElevonMixer(mixerMatrix);

      	m_aircraft->fwStatusLabel->setText(tr("Configuration OK"));

    }
 
    // Remove Feed Forward, it is pointless on a plane:
    setMixerValue(mixer, "FeedForward", 0.0);

    return airframeType;
}

void ConfigFixedWingWidget::setupMotors(QList<QString> motorList)
{
    QList<QComboBox *> mmList;
    mmList << m_aircraft->fwEngineChannelBox << m_aircraft->fwAileron1ChannelBox
           << m_aircraft->fwAileron2ChannelBox << m_aircraft->fwElevator1ChannelBox
           << m_aircraft->fwElevator2ChannelBox << m_aircraft->fwRudder1ChannelBox
           << m_aircraft->fwRudder2ChannelBox;

    GUIConfigDataUnion configData = getConfigData();
    resetActuators(&configData);

    foreach(QString motor, motorList) {
        int index = mmList.takeFirst()->currentIndex();

        if (motor == QString("FixedWingThrottle")) {
            configData.fixedwing.FixedWingThrottle = index;
        } else if (motor == QString("FixedWingPitch1")) {
            configData.fixedwing.FixedWingPitch1 = index;
        } else if (motor == QString("FixedWingPitch2")) {
            configData.fixedwing.FixedWingPitch2 = index;
        } else if (motor == QString("FixedWingRoll1")) {
            configData.fixedwing.FixedWingRoll1 = index;
        } else if (motor == QString("FixedWingRoll2")) {
            configData.fixedwing.FixedWingRoll2 = index;
        } else if (motor == QString("FixedWingYaw1")) {
            configData.fixedwing.FixedWingYaw1 = index;
        } else if (motor == QString("FixedWingYaw2")) {
            configData.fixedwing.FixedWingYaw1 = index;
        }
    }
    setConfigData(configData);
}

void ConfigFixedWingWidget::updateAirframe(QString frameType)
{
    qDebug() << "ConfigFixedWingWidget::updateAirframe - frame type" << frameType;
    
    // this is not doing anything right now 

    m_aircraft->planeShape->setSceneRect(plane->boundingRect());
    m_aircraft->planeShape->fitInView(plane, Qt::KeepAspectRatio);
}

/**
   Setup Elevator/Aileron/Rudder airframe.

   If both Aileron channels are set to 'None' (EasyStar), do Pitch/Rudder mixing

   Returns False if impossible to create the mixer.
 */
bool ConfigFixedWingWidget::setupFrameFixedWing(QString airframeType)
{
    // Check coherence:
    // Show any config errors in GUI
    if (throwConfigError(airframeType)) {
        return false;
    }

    // Now setup the channels:
    GUIConfigDataUnion config = getConfigData();
    resetActuators(&config);

    config.fixedwing.FixedWingPitch1   = m_aircraft->fwElevator1ChannelBox->currentIndex();
    config.fixedwing.FixedWingPitch2   = m_aircraft->fwElevator2ChannelBox->currentIndex();
    config.fixedwing.FixedWingRoll1    = m_aircraft->fwAileron1ChannelBox->currentIndex();
    config.fixedwing.FixedWingRoll2    = m_aircraft->fwAileron2ChannelBox->currentIndex();
    config.fixedwing.FixedWingYaw1     = m_aircraft->fwRudder1ChannelBox->currentIndex();
    config.fixedwing.FixedWingYaw2     = m_aircraft->fwRudder2ChannelBox->currentIndex();
    config.fixedwing.FixedWingThrottle = m_aircraft->fwEngineChannelBox->currentIndex();

    setConfigData(config);

    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);
    resetMotorAndServoMixers(mixer);

    // ... and compute the matrix:
    // In order to make code a bit nicer, we assume:
    // - Channel dropdowns start with 'None', then 0 to 7

    // 1. Assign the servo/motor/none for each channel

    // motor
    int channel = m_aircraft->fwEngineChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_MOTOR);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE1, 127);

    // rudder
    channel = m_aircraft->fwRudder1ChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW, 127);

    // ailerons
    channel = m_aircraft->fwAileron1ChannelBox->currentIndex() - 1;
    if (channel > -1) {
        setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
        setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL, 127);

        channel = m_aircraft->fwAileron2ChannelBox->currentIndex() - 1;
        setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
        setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL, 127);
    }

    // elevators
    channel = m_aircraft->fwElevator1ChannelBox->currentIndex() - 1;
    if (channel > -1) {
        setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
        setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH, 127);

        channel = m_aircraft->fwElevator2ChannelBox->currentIndex() - 1;
        setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
        setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH, 127);
    }

    m_aircraft->fwStatusLabel->setText("Mixer generated");

    return true;
}

/**
   Setup Elevon
 */
bool ConfigFixedWingWidget::setupFrameElevon(QString airframeType)
{
    // Check coherence:
    // Show any config errors in GUI
    if (throwConfigError(airframeType)) {
        return false;
    }

    GUIConfigDataUnion config = getConfigData();
    resetActuators(&config);

    config.fixedwing.FixedWingPitch1   = m_aircraft->fwElevator1ChannelBox->currentIndex();
    config.fixedwing.FixedWingPitch2   = m_aircraft->fwElevator2ChannelBox->currentIndex();    
    config.fixedwing.FixedWingRoll1    = m_aircraft->fwAileron1ChannelBox->currentIndex();
    config.fixedwing.FixedWingRoll2    = m_aircraft->fwAileron2ChannelBox->currentIndex();
    config.fixedwing.FixedWingThrottle = m_aircraft->fwEngineChannelBox->currentIndex();

    setConfigData(config);

    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);
    resetMotorAndServoMixers(mixer);

    // Save the curve:
    // ... and compute the matrix:
    // In order to make code a bit nicer, we assume:
    // - Channel dropdowns start with 'None', then 0 to 7

    // 1. Assign the servo/motor/none for each channel

    // motor
    int channel = m_aircraft->fwEngineChannelBox->currentIndex() - 1;
    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_MOTOR);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE1, 127);

    // ailerons
    channel = m_aircraft->fwAileron1ChannelBox->currentIndex() - 1;
    if (channel > -1) {
        setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
	setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL, 127);

        channel = m_aircraft->fwAileron2ChannelBox->currentIndex() - 1;
        setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
	setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL, -127);
    }

    // elevon
    channel = m_aircraft->fwElevator1ChannelBox->currentIndex() - 1;
    if (channel > -1) {
        setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
        setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH, 127);

        channel = m_aircraft->fwElevator2ChannelBox->currentIndex() - 1;
        setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);
        setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH, -127);
    }

    m_aircraft->fwStatusLabel->setText("Mixer generated");
    return true;


}

/**
   Helper function: setupElevonServo
 */
void ConfigFixedWingWidget::setupElevonServo(int channel, double pitch, double roll)
{
    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));

    Q_ASSERT(mixer);

    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_SERVO);

    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL, roll * 127);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH, pitch * 127);
}

/**
    This function sets up the elevon fixed wing mixer values.
  */
bool ConfigFixedWingWidget::setupFixedWingElevonMixer(double mixerFactors[8][3]){
    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));

    Q_ASSERT(mixer);
    resetMotorAndServoMixers(mixer);

    // and enable only the relevant channels:
    double pFactor = (double)m_aircraft->elevonSlider1->value() / 100.0;
    double rFactor = (double)m_aircraft->elevonSlider2->value() / 100.0;

    QList<QComboBox *> mmList;
    mmList << m_aircraft->fwEngineChannelBox << m_aircraft->fwAileron1ChannelBox
           << m_aircraft->fwAileron2ChannelBox << m_aircraft->fwElevator1ChannelBox
	   << m_aircraft->fwElevator2ChannelBox;

    for (int i = 0; i < 8; i++) {
        if (mmList.at(i)->isEnabled()) {
            int channel = mmList.at(i)->currentIndex() - 1;
            if (channel > -1) {
		setupElevonServo(channel, mixerFactors[i][0] * pFactor, rFactor * mixerFactors[i][1]);
            }
        }
    }
    return true;
}

/**
   This function displays text and color formatting in order to help the user understand what channels have not yet been configured.
 */
bool ConfigFixedWingWidget::throwConfigError(QString airframeType)
{
    // Initialize configuration error flag
    bool error = false;

    // Create a red block. All combo boxes are the same size, so any one should do as a model
    int size   = m_aircraft->fwEngineChannelBox->style()->pixelMetric(QStyle::PM_SmallIconSize);
    QPixmap pixmap(size, size);

    pixmap.fill(QColor("red"));

    if (airframeType == "FixedWing") {
        if (m_aircraft->fwEngineChannelBox->currentText() == "None") {
            m_aircraft->fwEngineChannelBox->setItemData(0, pixmap, Qt::DecorationRole); // Set color palettes
            error = true;
        } else {
            m_aircraft->fwEngineChannelBox->setItemData(0, 0, Qt::DecorationRole); // Reset color palettes
        }

        if (m_aircraft->fwElevator1ChannelBox->currentText() == "None") {
            m_aircraft->fwElevator1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole); // Set color palettes
            error = true;
        } else {
            m_aircraft->fwElevator1ChannelBox->setItemData(0, 0, Qt::DecorationRole); // Reset color palettes
        }

        if ((m_aircraft->fwAileron1ChannelBox->currentText() == "None")
            && (m_aircraft->fwRudder1ChannelBox->currentText() == "None")) {
            pixmap.fill(QColor("green"));
            m_aircraft->fwAileron1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole); // Set color palettes
            m_aircraft->fwRudder1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole); // Set color palettes
            error = true;
        } else {
            m_aircraft->fwAileron1ChannelBox->setItemData(0, 0, Qt::DecorationRole); // Reset color palettes
            m_aircraft->fwRudder1ChannelBox->setItemData(0, 0, Qt::DecorationRole); // Reset color palettes
        }
    } else if (airframeType == "FixedWingElevon") {
/*
        if (m_aircraft->fwEngineChannelBox->currentText() == "None") {
            m_aircraft->fwEngineChannelBox->setItemData(0, pixmap, Qt::DecorationRole); // Set color palettes
            error = true;
        } else {
            m_aircraft->fwEngineChannelBox->setItemData(0, 0, Qt::DecorationRole); // Reset color palettes
        }

        if (m_aircraft->fwElevator1ChannelBox->currentText() == "None") {
            m_aircraft->fwElevator1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole); // Set color palettes
            error = true;
        } else {
            m_aircraft->fwElevator1ChannelBox->setItemData(0, 0, Qt::DecorationRole); // Reset color palettes
        }

        if (m_aircraft->fwElevator2ChannelBox->currentText() == "None") {
            m_aircraft->fwElevator2ChannelBox->setItemData(0, pixmap, Qt::DecorationRole); // Set color palettes
            error = true;
        } else {
	   m_aircraft->fwElevator2ChannelBox->setItemData(0, 0, Qt::DecorationRole); // Reset color palettes
        }
*/
    }

    if (error) {
        m_aircraft->fwStatusLabel->setText(QString("<font color='red'>ERROR: Assign all necessary channels</font>"));
    }

    return error;
}

/**
   WHAT DOES THIS DO???
 */
void ConfigFixedWingWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    m_aircraft->planeShape->fitInView(plane, Qt::KeepAspectRatio);
}

/**
   Resize the GUI contents when the user changes the window size
 */
void ConfigFixedWingWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_aircraft->planeShape->fitInView(plane, Qt::KeepAspectRatio);
}

void ConfigFixedWingWidget::enableControls(bool enable)
{
    ConfigTaskWidget::enableControls(enable);

    if (enable) {
        setupEnabledControls(m_aircraft->fixedWingType->currentText());
    }
}

