/**
 ******************************************************************************
 *
 * @file       configccpmwidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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
//#include "configfixedwingwidget.h"
#include "configvehicletypewidget.h"
#include "mixersettings.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QBrush>
#include <math.h>
#include <QMessageBox>

#include "mixersettings.h"
#include "systemsettings.h"
#include "actuatorcommand.h"


/**
 Helper function to setup the UI
 */
void ConfigVehicleTypeWidget::setupFixedWingUI(QString frameType)
{
	if (frameType == "FixedWing" || frameType == "Elevator aileron rudder") {
        // Setup the UI
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Fixed Wing"));
        m_aircraft->fixedWingType->setCurrentIndex(m_aircraft->fixedWingType->findText("Elevator aileron rudder"));
        m_aircraft->fwRudder1ChannelBox->setEnabled(true);
        m_aircraft->fwRudder1Label->setEnabled(true);
        m_aircraft->fwRudder2ChannelBox->setEnabled(true);
        m_aircraft->fwRudder2Label->setEnabled(true);
        m_aircraft->fwElevator1ChannelBox->setEnabled(true);
        m_aircraft->fwElevator1Label->setEnabled(true);
        m_aircraft->fwElevator2ChannelBox->setEnabled(true);
        m_aircraft->fwElevator2Label->setEnabled(true);
        m_aircraft->fwAileron1ChannelBox->setEnabled(true);
        m_aircraft->fwAileron1Label->setEnabled(true);
        m_aircraft->fwAileron2ChannelBox->setEnabled(true);
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
        m_aircraft->fwElevator1ChannelBox->setEnabled(false);
        m_aircraft->fwElevator1Label->setEnabled(false);
        m_aircraft->fwElevator2ChannelBox->setEnabled(false);
        m_aircraft->fwElevator2Label->setEnabled(false);
        m_aircraft->fwRudder1ChannelBox->setEnabled(true);
        m_aircraft->fwRudder1Label->setEnabled(true);
        m_aircraft->fwRudder2ChannelBox->setEnabled(true);
        m_aircraft->fwRudder2Label->setEnabled(true);
        m_aircraft->fwElevator1Label->setText("Elevator 1");
        m_aircraft->fwElevator2Label->setText("Elevator 2");
        m_aircraft->elevonMixBox->setHidden(false);
        m_aircraft->elevonLabel1->setText("Roll");
        m_aircraft->elevonLabel2->setText("Pitch");
		
	} else if (frameType == "FixedWingVtail" || frameType == "Vtail") {
        m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Fixed Wing"));
        m_aircraft->fixedWingType->setCurrentIndex(m_aircraft->fixedWingType->findText("Vtail"));
        m_aircraft->fwRudder1ChannelBox->setEnabled(false);
        m_aircraft->fwRudder1Label->setEnabled(false);
        m_aircraft->fwRudder2ChannelBox->setEnabled(false);
        m_aircraft->fwRudder2Label->setEnabled(false);
        m_aircraft->fwElevator1ChannelBox->setEnabled(true);
        m_aircraft->fwElevator1Label->setEnabled(true);
        m_aircraft->fwElevator1Label->setText("Vtail 1");
        m_aircraft->fwElevator2Label->setText("Vtail 2");
        m_aircraft->elevonMixBox->setHidden(false);
        m_aircraft->fwElevator2ChannelBox->setEnabled(true);
        m_aircraft->fwElevator2Label->setEnabled(true);
        m_aircraft->fwAileron1Label->setText("Aileron 1");
        m_aircraft->fwAileron2Label->setText("Aileron 2");
        m_aircraft->elevonLabel1->setText("Rudder");
        m_aircraft->elevonLabel2->setText("Pitch");
	}
}



/**
 Helper function to update the UI widget objects
 */
QString ConfigVehicleTypeWidget::updateFixedWingObjectsFromWidgets()
{
	QString airframeType = "FixedWing";
	
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
	
	//All airframe types must start with "FixedWing"
	if (m_aircraft->fixedWingType->currentText() == "Elevator aileron rudder" ) {
		airframeType = "FixedWing";
		setupFrameFixedWing( airframeType );
	} else if (m_aircraft->fixedWingType->currentText() == "Elevon") {
		airframeType = "FixedWingElevon";
		setupFrameElevon( airframeType );
	} else { // "Vtail"
		airframeType = "FixedWingVtail";
		setupFrameVtail( airframeType );
	}
	
	// Now reflect those settings in the "Custom" panel as well
	updateCustomAirframeUI();
	
	return airframeType;
}


/**
 Helper function to refresh the UI widget values
 */
void ConfigVehicleTypeWidget::refreshFixedWingWidgetsValues(QString frameType)
{
	
	UAVDataObject* obj;
	UAVObjectField *field;
	
	// Then retrieve how channels are setup
	obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
	Q_ASSERT(obj);
	field = obj->getField(QString("FixedWingThrottle"));
	Q_ASSERT(field);
	m_aircraft->fwEngineChannelBox->setCurrentIndex(m_aircraft->fwEngineChannelBox->findText(field->getValue().toString()));

	field = obj->getField(QString("FixedWingRoll1"));
	Q_ASSERT(field);
	m_aircraft->fwAileron1ChannelBox->setCurrentIndex(m_aircraft->fwAileron1ChannelBox->findText(field->getValue().toString()));
	
	field = obj->getField(QString("FixedWingRoll2"));
	Q_ASSERT(field);
	m_aircraft->fwAileron2ChannelBox->setCurrentIndex(m_aircraft->fwAileron2ChannelBox->findText(field->getValue().toString()));
	
	field = obj->getField(QString("FixedWingPitch1"));
	Q_ASSERT(field);
	m_aircraft->fwElevator1ChannelBox->setCurrentIndex(m_aircraft->fwElevator1ChannelBox->findText(field->getValue().toString()));
	
	field = obj->getField(QString("FixedWingPitch2"));
	Q_ASSERT(field);
	m_aircraft->fwElevator2ChannelBox->setCurrentIndex(m_aircraft->fwElevator2ChannelBox->findText(field->getValue().toString()));
	
	field = obj->getField(QString("FixedWingYaw1"));
	Q_ASSERT(field);
	m_aircraft->fwRudder1ChannelBox->setCurrentIndex(m_aircraft->fwRudder1ChannelBox->findText(field->getValue().toString()));
	
	field = obj->getField(QString("FixedWingYaw2"));
	Q_ASSERT(field);
	m_aircraft->fwRudder2ChannelBox->setCurrentIndex(m_aircraft->fwRudder2ChannelBox->findText(field->getValue().toString()));
	
	if (frameType == "FixedWingElevon") {
        // If the airframe is elevon, restore the slider setting
		// Find the channel number for Elevon1 (FixedWingRoll1)
		obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
		Q_ASSERT(obj);
		int chMixerNumber = m_aircraft->fwAileron1ChannelBox->currentIndex()-1;
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
		int chMixerNumber = m_aircraft->fwElevator1ChannelBox->currentIndex()-1;
		if (chMixerNumber >=0) {
			field = obj->getField(mixerVectors.at(chMixerNumber));
			int ti = field->getElementNames().indexOf("Yaw");
			m_aircraft->elevonSlider1->setValue(field->getDouble(ti)*100);
			ti = field->getElementNames().indexOf("Pitch");
			m_aircraft->elevonSlider2->setValue(field->getDouble(ti)*100);
		}
	}
	
}



/**
 Setup Elevator/Aileron/Rudder airframe.
 
 If both Aileron channels are set to 'None' (EasyStar), do Pitch/Rudder mixing
 
 Returns False if impossible to create the mixer.
 */
bool ConfigVehicleTypeWidget::setupFrameFixedWing(QString airframeType)
{
    // Check coherence:
	//Show any config errors in GUI
	throwFixedWingChannelConfigError(airframeType);
    
	// - At least Pitch and either Roll or Yaw
    if (m_aircraft->fwEngineChannelBox->currentText() == "None" ||
        m_aircraft->fwElevator1ChannelBox->currentText() == "None" ||
        ((m_aircraft->fwAileron1ChannelBox->currentText() == "None") &&
		 (m_aircraft->fwRudder1ChannelBox->currentText() == "None"))) {
			// TODO: explain the problem in the UI
//			m_aircraft->fwStatusLabel->setText("ERROR: check channel assignment");
			return false;
		}
    // Now setup the channels:
    resetActuators();
	
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
	
    // Elevator
    UAVObjectField *field = obj->getField("FixedWingPitch1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwElevator1ChannelBox->currentText());
    field = obj->getField("FixedWingPitch2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwElevator2ChannelBox->currentText());

    // Aileron
    field = obj->getField("FixedWingRoll1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron1ChannelBox->currentText());
    field = obj->getField("FixedWingRoll2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron2ChannelBox->currentText());
    
	// Rudder
    field = obj->getField("FixedWingYaw1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwRudder1ChannelBox->currentText());
    
	// Throttle
    field = obj->getField("FixedWingThrottle");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwEngineChannelBox->currentText());
	
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
    int tmpVal = m_aircraft->fwEngineChannelBox->currentIndex()-1;
    field = obj->getField(mixerTypes.at(tmpVal));
    field->setValue("Motor");
    field = obj->getField(mixerVectors.at(tmpVal));
    // First of all reset the vector
    resetField(field);
    int ti = field->getElementNames().indexOf("ThrottleCurve1");
    field->setValue(127, ti);
	
    // Rudder
    tmpVal = m_aircraft->fwRudder1ChannelBox->currentIndex()-1;
    // tmpVal will be -1 if rudder is set to "None"
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
        resetField(field);
        ti = field->getElementNames().indexOf("Yaw");
        field->setValue(127, ti);
    } // Else: we have no rudder, only ailerons, we're fine with it.
	
    // Ailerons
    tmpVal = m_aircraft->fwAileron1ChannelBox->currentIndex()-1;
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
        resetField(field);
        ti = field->getElementNames().indexOf("Roll");
        field->setValue(127, ti);
        // Only set Aileron 2 if Aileron 1 is defined
        tmpVal = m_aircraft->fwAileron2ChannelBox->currentIndex()-1;
        if (tmpVal > -1) {
            field = obj->getField(mixerTypes.at(tmpVal));
            field->setValue("Servo");
            field = obj->getField(mixerVectors.at(tmpVal));
            resetField(field);
            ti = field->getElementNames().indexOf("Roll");
            field->setValue(127, ti);
        }
    } // Else we have no ailerons. Our consistency check guarantees we have
	// rudder in this case, so we're fine with it too.
	
    // Elevator
    tmpVal = m_aircraft->fwElevator1ChannelBox->currentIndex()-1;
    field = obj->getField(mixerTypes.at(tmpVal));
    field->setValue("Servo");
    field = obj->getField(mixerVectors.at(tmpVal));
    resetField(field);
    ti = field->getElementNames().indexOf("Pitch");
    field->setValue(127, ti);
    // Only set Elevator 2 if it is defined
    tmpVal = m_aircraft->fwElevator2ChannelBox->currentIndex()-1;
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
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
bool ConfigVehicleTypeWidget::setupFrameElevon(QString airframeType)
{
    // Check coherence:
	//Show any config errors in GUI
	throwFixedWingChannelConfigError(airframeType);

    // - At least Aileron1 and Aileron 2, and engine
    if (m_aircraft->fwEngineChannelBox->currentText() == "None" ||
        m_aircraft->fwAileron1ChannelBox->currentText() == "None" ||
        m_aircraft->fwAileron2ChannelBox->currentText() == "None") {
        // TODO: explain the problem in the UI
//        m_aircraft->fwStatusLabel->setText("ERROR: check channel assignment");
        return false;
    }
	
    resetActuators();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
	
    // Elevons
    UAVObjectField *field = obj->getField("FixedWingRoll1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron1ChannelBox->currentText());
    field = obj->getField("FixedWingRoll2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron2ChannelBox->currentText());
    // Rudder 1 (can be None)
    field = obj->getField("FixedWingYaw1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwRudder1ChannelBox->currentText());
    // Rudder 2 (can be None)
    field = obj->getField("FixedWingYaw2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwRudder2ChannelBox->currentText());
    // Throttle
    field = obj->getField("FixedWingThrottle");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwEngineChannelBox->currentText());
	
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
    int tmpVal = m_aircraft->fwEngineChannelBox->currentIndex()-1;
    field = obj->getField(mixerTypes.at(tmpVal));
    field->setValue("Motor");
    field = obj->getField(mixerVectors.at(tmpVal));
    // First of all reset the vector
    resetField(field);
    int ti = field->getElementNames().indexOf("ThrottleCurve1");
    field->setValue(127, ti);
	
    // Rudder 1
    tmpVal = m_aircraft->fwRudder1ChannelBox->currentIndex()-1;
    // tmpVal will be -1 if rudder 1 is set to "None"
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
        resetField(field);
        ti = field->getElementNames().indexOf("Yaw");
        field->setValue(127, ti);
    } // Else: we have no rudder, only elevons, we're fine with it.
	
    // Rudder 2
    tmpVal = m_aircraft->fwRudder2ChannelBox->currentIndex()-1;
    // tmpVal will be -1 if rudder 2 is set to "None"
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
        resetField(field);
        ti = field->getElementNames().indexOf("Yaw");
        field->setValue(-127, ti);
    } // Else: we have no rudder, only elevons, we're fine with it.
	
    tmpVal = m_aircraft->fwAileron1ChannelBox->currentIndex()-1;
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
        resetField(field);
        ti = field->getElementNames().indexOf("Pitch");
        field->setValue((double)m_aircraft->elevonSlider2->value()*1.27, ti);
        ti = field->getElementNames().indexOf("Roll");
        field->setValue((double)m_aircraft->elevonSlider1->value()*1.27,ti);
    }
	
    tmpVal = m_aircraft->fwAileron2ChannelBox->currentIndex()-1;
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
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
bool ConfigVehicleTypeWidget::setupFrameVtail(QString airframeType)
{
    // Check coherence:
	//Show any config errors in GUI
	throwFixedWingChannelConfigError(airframeType);
    
	// - At least Pitch1 and Pitch2, and engine
    if (m_aircraft->fwEngineChannelBox->currentText() == "None" ||
        m_aircraft->fwElevator1ChannelBox->currentText() == "None" ||
        m_aircraft->fwElevator2ChannelBox->currentText() == "None") {
        // TODO: explain the problem in the UI
//        m_aircraft->fwStatusLabel->setText("WARNING: check channel assignment");
        return false;
    }
	
    resetActuators();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
	
    // Elevons
    UAVObjectField *field = obj->getField("FixedWingPitch1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwElevator1ChannelBox->currentText());
    field = obj->getField("FixedWingPitch2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwElevator2ChannelBox->currentText());
    field = obj->getField("FixedWingRoll1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron1ChannelBox->currentText());
    field = obj->getField("FixedWingRoll2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwAileron2ChannelBox->currentText());
	
    // Throttle
    field = obj->getField("FixedWingThrottle");
    Q_ASSERT(field);
    field->setValue(m_aircraft->fwEngineChannelBox->currentText());
	
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
    int tmpVal = m_aircraft->fwEngineChannelBox->currentIndex()-1;
    field = obj->getField(mixerTypes.at(tmpVal));
    field->setValue("Motor");
    field = obj->getField(mixerVectors.at(tmpVal));
    // First of all reset the vector
    resetField(field);
    int ti = field->getElementNames().indexOf("ThrottleCurve1");
    field->setValue(127, ti);
	
    tmpVal = m_aircraft->fwAileron1ChannelBox->currentIndex()-1;
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
        resetField(field);
        ti = field->getElementNames().indexOf("Roll");
        field->setValue(127,ti);
    }
	
    tmpVal = m_aircraft->fwAileron2ChannelBox->currentIndex()-1;
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
        resetField(field);
        ti = field->getElementNames().indexOf("Roll");
        field->setValue(-127,ti);
    }
	
    // Now compute the VTail
    tmpVal = m_aircraft->fwElevator1ChannelBox->currentIndex()-1;
    field = obj->getField(mixerTypes.at(tmpVal));
    field->setValue("Servo");
    field = obj->getField(mixerVectors.at(tmpVal));
    resetField(field);
    ti = field->getElementNames().indexOf("Pitch");
    field->setValue((double)m_aircraft->elevonSlider2->value()*1.27, ti);
    ti = field->getElementNames().indexOf("Yaw");
    field->setValue((double)m_aircraft->elevonSlider1->value()*1.27,ti);
	
    tmpVal = m_aircraft->fwElevator2ChannelBox->currentIndex()-1;
    field = obj->getField(mixerTypes.at(tmpVal));
    field->setValue("Servo");
    field = obj->getField(mixerVectors.at(tmpVal));
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
 This function displays text and color formatting in order to help the user understand what channels have not yet been configured.
 */
void ConfigVehicleTypeWidget::throwFixedWingChannelConfigError(QString airframeType)
{
	//Initialize configuration error flag
	bool error=false;
	
	//Create a red block. All combo boxes are the same size, so any one should do as a model
	int size = m_aircraft->fwEngineChannelBox->style()->pixelMetric(QStyle::PM_SmallIconSize);
	QPixmap pixmap(size,size);
	pixmap.fill(QColor("red"));
	
	if (airframeType == "FixedWing" ) {
		if (m_aircraft->fwEngineChannelBox->currentText() == "None"){
			m_aircraft->fwEngineChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			error=true;
		}
		else{
			m_aircraft->fwEngineChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}
		
		if (m_aircraft->fwElevator1ChannelBox->currentText() == "None"){
			m_aircraft->fwElevator1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			error=true;
		}
		else{
			m_aircraft->fwElevator1ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}	
	
		if	((m_aircraft->fwAileron1ChannelBox->currentText() == "None") &&	 (m_aircraft->fwRudder1ChannelBox->currentText() == "None")) {
			pixmap.fill(QColor("green"));
			m_aircraft->fwAileron1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			m_aircraft->fwRudder1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			error=true;
		}
		else{
			m_aircraft->fwAileron1ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
			m_aircraft->fwRudder1ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}	
	} else if (airframeType == "FixedWingElevon"){
		if (m_aircraft->fwEngineChannelBox->currentText() == "None"){
			m_aircraft->fwEngineChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			error=true;
		}
		else{
			m_aircraft->fwEngineChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}	
		
		if(m_aircraft->fwAileron1ChannelBox->currentText() == "None"){
			m_aircraft->fwAileron1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			error=true;
		}
		else{
			m_aircraft->fwAileron1ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}	
		
		if (m_aircraft->fwAileron2ChannelBox->currentText() == "None"){
			m_aircraft->fwAileron2ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			error=true;
		}
		else{
			m_aircraft->fwAileron2ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}	
			
	} else if ( airframeType == "FixedWingVtail"){
		if (m_aircraft->fwEngineChannelBox->currentText() == "None"){
			m_aircraft->fwEngineChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			error=true;
		}
		else{
			m_aircraft->fwEngineChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}	
		
		if(m_aircraft->fwElevator1ChannelBox->currentText() == "None"){
			m_aircraft->fwElevator1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			error=true;
		}
		else{
			m_aircraft->fwElevator1ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}	
		
		if(m_aircraft->fwElevator2ChannelBox->currentText() == "None"){
			m_aircraft->fwElevator2ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			error=true;
		}		
		else{
			m_aircraft->fwElevator2ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}	
		
	}
	
	if (error){
		m_aircraft->fwStatusLabel->setText(QString("<font color='red'>ERROR: Assign all necessary channels</font>"));
	}
}
