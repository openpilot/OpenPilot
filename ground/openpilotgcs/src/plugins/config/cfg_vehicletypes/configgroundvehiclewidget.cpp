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
//#include "configgroundvehiclewidget.h"
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
void ConfigVehicleTypeWidget::setupGroundVehicleUI(QString frameType)
{
	m_aircraft->differentialSteeringMixBox->setHidden(true);			
	//STILL NEEDS WORK
	// Setup the UI
	m_aircraft->aircraftType->setCurrentIndex(m_aircraft->aircraftType->findText("Ground"));
	
	m_aircraft->gvEngineChannelBox->setEnabled(false);
	m_aircraft->gvEngineLabel->setEnabled(false);

	
	m_aircraft->gvAileron1ChannelBox->setEnabled(false);
	m_aircraft->gvAileron1Label->setEnabled(false);
	
	m_aircraft->gvAileron2ChannelBox->setEnabled(false);
	m_aircraft->gvAileron2Label->setEnabled(false);
	
	if (frameType == "GroundVehicleDifferential" || frameType == "Differential (tank)"){ //Tank
		m_aircraft->groundVehicleType->setCurrentIndex(m_aircraft->groundVehicleType->findText("Differential (tank)"));
		m_aircraft->gvMotor1ChannelBox->setEnabled(true);
		m_aircraft->gvMotor1Label->setEnabled(true);
		
		m_aircraft->gvMotor2ChannelBox->setEnabled(true);
		m_aircraft->gvMotor2Label->setEnabled(true);

		m_aircraft->gvMotor1Label->setText("Left motor");
		m_aircraft->gvMotor2Label->setText("Right motor");

		m_aircraft->gvSteering1ChannelBox->setEnabled(false);
		m_aircraft->gvSteering1Label->setEnabled(false);       
		
		m_aircraft->gvSteering2ChannelBox->setEnabled(false);
		m_aircraft->gvSteering2Label->setEnabled(false);

		m_aircraft->gvSteering2Label->setText("Rear steering");

		m_aircraft->differentialSteeringMixBox->setHidden(false);

		m_aircraft->gvThrottleCurve1GroupBox->setTitle("Left throttle curve");
		m_aircraft->gvThrottleCurve2GroupBox->setTitle("Right throttle curve");

	}
	else if (frameType == "GroundVehicleMotorcycle" || frameType == "Motorcycle"){ //Motorcycle
		m_aircraft->groundVehicleType->setCurrentIndex(m_aircraft->groundVehicleType->findText("Motorcycle"));
		m_aircraft->gvMotor1ChannelBox->setEnabled(false);
		m_aircraft->gvMotor1Label->setEnabled(false);

		m_aircraft->gvMotor2ChannelBox->setEnabled(true);
		m_aircraft->gvMotor2Label->setEnabled(true);

		m_aircraft->gvMotor1Label->setText("Front motor");
		m_aircraft->gvMotor2Label->setText("Rear motor");

		m_aircraft->gvSteering1ChannelBox->setEnabled(true);
		m_aircraft->gvSteering1Label->setEnabled(true);       
		
		m_aircraft->gvSteering2ChannelBox->setEnabled(true);
		m_aircraft->gvSteering2Label->setEnabled(true);

		m_aircraft->gvSteering2Label->setText("Balancing");

		m_aircraft->differentialSteeringMixBox->setHidden(true);
		
		m_aircraft->gvThrottleCurve1GroupBox->setTitle("Front throttle curve");
		m_aircraft->gvThrottleCurve2GroupBox->setTitle("Rear throttle curve");
	}
	else {//Car
		m_aircraft->groundVehicleType->setCurrentIndex(m_aircraft->groundVehicleType->findText("Turnable (car)"));
		
		m_aircraft->gvMotor1ChannelBox->setEnabled(true);
		m_aircraft->gvMotor1Label->setEnabled(true);

		m_aircraft->gvMotor2ChannelBox->setEnabled(true);
		m_aircraft->gvMotor2Label->setEnabled(true);
		
		m_aircraft->gvMotor1Label->setText("Front motor");
		m_aircraft->gvMotor2Label->setText("Rear motor");
		
		m_aircraft->gvSteering1ChannelBox->setEnabled(true);
		m_aircraft->gvSteering1Label->setEnabled(true);       
		
		m_aircraft->gvSteering2ChannelBox->setEnabled(true);
		m_aircraft->gvSteering2Label->setEnabled(true);
		
		m_aircraft->differentialSteeringMixBox->setHidden(true);			
		
		m_aircraft->gvThrottleCurve1GroupBox->setTitle("Front throttle curve");
		m_aircraft->gvThrottleCurve2GroupBox->setTitle("Rear throttle curve");
	}
}



/**
 Helper function to update the UI widget objects
 */
QString ConfigVehicleTypeWidget::updateGroundVehicleObjectsFromWidgets()
{
	QString airframeType = "GroundVehicleCar";
	
	// Save the curve (common to all ground vehicle frames)
	UAVDataObject *obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
	
	// Remove Feed Forward, it is pointless on a ground vehicle:
	UAVObjectField* field = obj->getField(QString("FeedForward"));
	field->setDouble(0);
	
	field = obj->getField("ThrottleCurve1");
	QList<double> curve = m_aircraft->groundVehicleThrottle1->getCurve();
	for (int i=0;i<curve.length();i++) {
		field->setValue(curve.at(i),i);
	}

	field = obj->getField("ThrottleCurve2");
	curve = m_aircraft->groundVehicleThrottle2->getCurve();
	for (int i=0;i<curve.length();i++) {
		field->setValue(curve.at(i),i);
	}
	
	//All airframe types must start with "GroundVehicle"
	if (m_aircraft->groundVehicleType->currentText() == "Turnable (car)" ) {
		airframeType = "GroundVehicleCar";
		setupGroundVehicleCar(airframeType);
	} else if (m_aircraft->groundVehicleType->currentText() == "Differential (tank)") {
		airframeType = "GroundVehicleDifferential";
		setupGroundVehicleDifferential(airframeType);
	} else { // "Motorcycle"
		airframeType = "GroundVehicleMotorcycle";
		setupGroundVehicleMotorcycle(airframeType);
	}
		
	// Now reflect those settings in the "Custom" panel as well
	updateCustomAirframeUI();
	
	return airframeType;
}



/**
 Helper function to refresh the UI widget values
 */
void ConfigVehicleTypeWidget::refreshGroundVehicleWidgetsValues(QString frameType)
{
	
	UAVDataObject*	obj;
	UAVObjectField *field;
	
	//THIS SECTION STILL NEEDS WORK. FOR THE MOMENT, USE THE FIXED-WING ONBOARD SETTING IN ORDER TO MINIMIZE CHANCES OF BOLLOXING REAL CODE
	// Retrieve channel setup values
	obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
	Q_ASSERT(obj);
	field = obj->getField(QString("FixedWingThrottle"));
	Q_ASSERT(field);
	m_aircraft->gvEngineChannelBox->setCurrentIndex(m_aircraft->gvEngineChannelBox->findText(field->getValue().toString()));
	
	field = obj->getField(QString("FixedWingRoll1"));
	Q_ASSERT(field);
	m_aircraft->gvAileron1ChannelBox->setCurrentIndex(m_aircraft->gvAileron1ChannelBox->findText(field->getValue().toString()));
	
	field = obj->getField(QString("FixedWingRoll2"));
	Q_ASSERT(field);
	m_aircraft->gvAileron2ChannelBox->setCurrentIndex(m_aircraft->gvAileron2ChannelBox->findText(field->getValue().toString()));
	
	field = obj->getField(QString("GroundVehicleThrottle1"));
	Q_ASSERT(field);
	m_aircraft->gvMotor1ChannelBox->setCurrentIndex(m_aircraft->gvMotor1ChannelBox->findText(field->getValue().toString()));
	
	field = obj->getField(QString("GroundVehicleThrottle2"));
	Q_ASSERT(field);
	m_aircraft->gvMotor2ChannelBox->setCurrentIndex(m_aircraft->gvMotor2ChannelBox->findText(field->getValue().toString()));
	
	field = obj->getField(QString("GroundVehicleSteering1"));
	Q_ASSERT(field);
	m_aircraft->gvSteering1ChannelBox->setCurrentIndex(m_aircraft->gvSteering1ChannelBox->findText(field->getValue().toString()));
	
	field = obj->getField(QString("GroundVehicleSteering2"));
	Q_ASSERT(field);
	m_aircraft->gvSteering2ChannelBox->setCurrentIndex(m_aircraft->gvSteering2ChannelBox->findText(field->getValue().toString()));
	
	if (frameType == "GroundVehicleDifferential") {
		//CURRENTLY BROKEN UNTIL WE DECIDE HOW DIFFERENTIAL SHOULD BEHAVE
		// If the vehicle type is "differential", restore the slider setting
		
		// Find the channel number for Motor1 
		obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
		Q_ASSERT(obj);
		int chMixerNumber = m_aircraft->gvMotor1ChannelBox->currentIndex()-1;
		if (chMixerNumber >= 0) { // If for some reason the actuators were incoherent, we might fail here, hence the check.
			field = obj->getField(mixerVectors.at(chMixerNumber));
			int ti = field->getElementNames().indexOf("Roll");
			m_aircraft->differentialSteeringSlider1->setValue(field->getDouble(ti)*100);
			
			ti = field->getElementNames().indexOf("Pitch");
			m_aircraft->differentialSteeringSlider2->setValue(field->getDouble(ti)*100);
		}
	}
	if (frameType == "GroundVehicleMotorcycle") {
		//CURRENTLY BROKEN UNTIL WE DECIDE HOW MOTORCYCLE SHOULD BEHAVE
//		obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
//		Q_ASSERT(obj);
//		int chMixerNumber = m_aircraft->gvMotor1ChannelBox->currentIndex()-1;
//		if (chMixerNumber >=0) {
//			field = obj->getField(mixerVectors.at(chMixerNumber));
//			int ti = field->getElementNames().indexOf("Yaw");
//			m_aircraft->differentialSteeringSlider1->setValue(field->getDouble(ti)*100);
//			
//			ti = field->getElementNames().indexOf("Pitch");
//			m_aircraft->differentialSteeringSlider2->setValue(field->getDouble(ti)*100);
//		}
	}
}




/**
 Setup balancing ground vehicle.
 
 Returns False if impossible to create the mixer.
 */
bool ConfigVehicleTypeWidget::setupGroundVehicleMotorcycle(QString airframeType){
	// Check coherence:
	//Show any config errors in GUI
	throwGroundVehicleChannelConfigError(airframeType);
	
    // - Motor, steering, and balance
    if (m_aircraft->gvMotor1ChannelBox->currentText() == "None" ||
        (m_aircraft->gvSteering1ChannelBox->currentText() == "None" ||
		 m_aircraft->gvSteering2ChannelBox->currentText() == "None") )
	{
		return false;
	}
	
	
	// Now setup the channels:
    resetActuators();
	
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
	
    // Left motor
    UAVObjectField *field = obj->getField("GroundVehicleThrottle1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->gvMotor1ChannelBox->currentText());
    
    // Right motor
	field = obj->getField("GroundVehicleThrottle2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->gvMotor2ChannelBox->currentText());
	
	obj->updated();
	
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    // ... and compute the matrix:
    // In order to make code a bit nicer, we assume:
    // - Channel dropdowns start with 'None', then 0 to 7
	
    // 1. Assign the servo/motor/none for each channel
	
	int tmpVal, ti;
	
	// Disable all output channels
    foreach(QString mixer, mixerTypes) {
        field = obj->getField(mixer);
        Q_ASSERT(field);
		
		//Disable output channel
        field->setValue("Disabled");
		
	}
	
	// Set all mixer values to zero
    foreach(QString mixer, mixerVectors) {
		field = obj->getField(mixer);
		resetField(field);
		
		ti = field->getElementNames().indexOf("ThrottleCurve1");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("ThrottleCurve2");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("Yaw");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("Pitch");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("Roll");
		field->setValue(0, ti);
	}
	
    // Motor
    // Setup motor
    tmpVal = m_aircraft->gvMotor2ChannelBox->currentIndex()-1;
	field = obj->getField(mixerTypes.at(tmpVal));
	field->setValue("Servo"); //Set motor mixer type to Servo
	field = obj->getField(mixerVectors.at(tmpVal));
	resetField(field);
	ti = field->getElementNames().indexOf("ThrottleCurve1"); //Set motor to full forward
	field->setValue(127, ti);
	
	//Steering
    // Setup steering
    tmpVal = m_aircraft->gvSteering1ChannelBox->currentIndex()-1;
	field = obj->getField(mixerTypes.at(tmpVal));
	field->setValue("Servo"); //Set motor mixer type to Servo	
	field = obj->getField(mixerVectors.at(tmpVal));
	resetField(field);
	ti = field->getElementNames().indexOf("Yaw"); //Set steering response to roll
	field->setValue(-127, ti);
	ti = field->getElementNames().indexOf("Roll"); //Set steering response to roll
	field->setValue(-127, ti);

	//Balancing
    // Setup balancing servo
    tmpVal = m_aircraft->gvSteering2ChannelBox->currentIndex()-1;
	field = obj->getField(mixerTypes.at(tmpVal));
	field->setValue("Servo"); //Set motor mixer type to Servo	
	field = obj->getField(mixerVectors.at(tmpVal));
	resetField(field);
	ti = field->getElementNames().indexOf("Yaw"); //Set balance response to yaw
	field->setValue(127, ti);
	ti = field->getElementNames().indexOf("Roll"); //Set balance response to roll
	field->setValue(127, ti);
	
    obj->updated();
	
	//Output success message
	m_aircraft->gvStatusLabel->setText("Mixer generated");
	
    return true;
}



/**
 Setup differentially steered ground vehicle.
 
 Returns False if impossible to create the mixer.
 */
bool ConfigVehicleTypeWidget::setupGroundVehicleDifferential(QString airframeType){
	// Check coherence:
	//Show any config errors in GUI
	throwGroundVehicleChannelConfigError(airframeType);
	
    // - Left and right steering
    if ( m_aircraft->gvMotor2ChannelBox->currentText() == "None" ||
		m_aircraft->gvSteering1ChannelBox->currentText() == "None") 
	{
		return false;
	}

	
	// Now setup the channels:
    resetActuators();
	
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
	
    // Left motor
    UAVObjectField *field = obj->getField("GroundVehicleThrottle1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->gvMotor1ChannelBox->currentText());
    
    // Right motor
	field = obj->getField("GroundVehicleThrottle2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->gvMotor2ChannelBox->currentText());
	
	obj->updated();
	
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    // ... and compute the matrix:
    // In order to make code a bit nicer, we assume:
    // - Channel dropdowns start with 'None', then 0 to 7
	
    // 1. Assign the servo/motor/none for each channel
	
	int tmpVal, ti;
	
	// Disable all output channels
    foreach(QString mixer, mixerTypes) {
        field = obj->getField(mixer);
        Q_ASSERT(field);
		
		//Disable output channel
        field->setValue("Disabled");
		
	}
	
	// Set all mixer values to zero
    foreach(QString mixer, mixerVectors) {
		field = obj->getField(mixer);
		resetField(field);
		
		ti = field->getElementNames().indexOf("ThrottleCurve1");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("ThrottleCurve2");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("Yaw");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("Pitch");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("Roll");
		field->setValue(0, ti);
	}
	
    // Motor
    // Setup left motor
    tmpVal = m_aircraft->gvMotor1ChannelBox->currentIndex()-1;
	field = obj->getField(mixerTypes.at(tmpVal));
	field->setValue("Servo"); //Set motor mixer type to Servo
	field = obj->getField(mixerVectors.at(tmpVal));
	resetField(field);
	ti = field->getElementNames().indexOf("ThrottleCurve1"); //Set motor to full forward
	field->setValue(127, ti);
	ti = field->getElementNames().indexOf("Yaw"); //Set motor to turn right with increasing throttle
	field->setValue(127, ti);
	
    // Setup right motor
    tmpVal = m_aircraft->gvMotor2ChannelBox->currentIndex()-1;
	field = obj->getField(mixerTypes.at(tmpVal));
	field->setValue("Servo"); //Set motor mixer type to Servo	
	field = obj->getField(mixerVectors.at(tmpVal));
	resetField(field);
	ti = field->getElementNames().indexOf("ThrottleCurve2"); //Set motor to full forward
	field->setValue(127, ti);
	ti = field->getElementNames().indexOf("Yaw"); //Set motor to turn left with increasing throttle
	field->setValue(-127, ti);
	
    obj->updated();

	//Output success message
	m_aircraft->gvStatusLabel->setText("Mixer generated");
	
    return true;
	
}



/**
 Setup steerable ground vehicle.
 
 Returns False if impossible to create the mixer.
 */
bool ConfigVehicleTypeWidget::setupGroundVehicleCar(QString airframeType)
{
    // Check coherence:
	//Show any config errors in GUI
	throwGroundVehicleChannelConfigError(airframeType);
	
    // - At least one motor and one steering servo
    if ((m_aircraft->gvMotor1ChannelBox->currentText() == "None" &&
		 m_aircraft->gvMotor2ChannelBox->currentText() == "None") ||
        (m_aircraft->gvSteering1ChannelBox->currentText() == "None" &&
		 m_aircraft->gvSteering2ChannelBox->currentText() == "None")) 
	{
		return false;
	}
//	else{
//		//		m_aircraft->gvStatusLabel->setText("Mixer generated");
//		QTextEdit* htmlText=new QTextEdit(m_aircraft->gvSteering1Label->text());  // HtmlText is any QString with html tags.
//		m_aircraft->gvSteering1Label->setText(htmlText->toPlainText());
//		delete htmlText;
//		
//		htmlText=new QTextEdit(m_aircraft->gvSteering2Label->text());  // HtmlText is any QString with html tags.
//		m_aircraft->gvSteering2Label->setText(htmlText->toPlainText());
//		delete htmlText;
//		
//		htmlText=new QTextEdit(m_aircraft->gvMotor1Label->text());  // HtmlText is any QString with html tags.
//		m_aircraft->gvMotor1Label->setText(htmlText->toPlainText());
//		delete htmlText;
//		
//		htmlText=new QTextEdit(m_aircraft->gvMotor2Label->text());  // HtmlText is any QString with html tags.
//		m_aircraft->gvMotor2Label->setText(htmlText->toPlainText());
//	}
	
	// Now setup the channels:
    resetActuators();
	
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
	
    // Front motor
    UAVObjectField *field = obj->getField("GroundVehicleThrottle1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->gvMotor1ChannelBox->currentText());
    
    // Rear motor
	field = obj->getField("GroundVehicleThrottle2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->gvMotor2ChannelBox->currentText());

//    // Aileron
//	field = obj->getField("FixedWingRoll1");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwAileron1ChannelBox->currentText());
//
//    field = obj->getField("FixedWingRoll2");
//    Q_ASSERT(field);
//    field->setValue(m_aircraft->fwAileron2ChannelBox->currentText());

    // Front steering
	field = obj->getField("GroundVehicleSteering1");
    Q_ASSERT(field);
    field->setValue(m_aircraft->gvSteering1ChannelBox->currentText());

    // Rear steering
	field = obj->getField("GroundVehicleSteering2");
    Q_ASSERT(field);
    field->setValue(m_aircraft->gvSteering2ChannelBox->currentText());
	
    obj->updated();
	
    obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(obj);
    // ... and compute the matrix:
    // In order to make code a bit nicer, we assume:
    // - Channel dropdowns start with 'None', then 0 to 7

    // 1. Assign the servo/motor/none for each channel

	int tmpVal, ti;

	// Disable all output channels
    foreach(QString mixer, mixerTypes) {
        field = obj->getField(mixer);
        Q_ASSERT(field);
		
		//Disable output channel
        field->setValue("Disabled");

	}
	
	// Set all mixer values to zero
    foreach(QString mixer, mixerVectors) {
		field = obj->getField(mixer);
		resetField(field);
		
		ti = field->getElementNames().indexOf("ThrottleCurve1");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("ThrottleCurve2");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("Yaw");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("Pitch");
		field->setValue(0, ti);
		ti = field->getElementNames().indexOf("Roll");
		field->setValue(0, ti);
	}

    // Steering
    // Only set front steering if it is defined
    tmpVal = m_aircraft->gvSteering1ChannelBox->currentIndex()-1;
    // tmpVal will be -1 if steering is set to "None"
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
        resetField(field);
        ti = field->getElementNames().indexOf("Yaw");
        field->setValue(127, ti);
    } // Else: we have no front steering. We're fine with it as long as we have rear steering
	
    // Only set rear steering if it is defined
    tmpVal = m_aircraft->gvSteering2ChannelBox->currentIndex()-1;
    // tmpVal will be -1 if steering is set to "None"
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
        resetField(field);
        ti = field->getElementNames().indexOf("Yaw");
        field->setValue(-127, ti);
    } // Else: we have no rear steering. We're fine with it as long as we have front steering

    // Motor
    // Only set front motor if it is defined
    tmpVal = m_aircraft->gvMotor1ChannelBox->currentIndex()-1;
    if (tmpVal > -1) {
		field = obj->getField(mixerTypes.at(tmpVal));
		field->setValue("Servo");
		field = obj->getField(mixerVectors.at(tmpVal));
		resetField(field);
		ti = field->getElementNames().indexOf("ThrottleCurve1");
		field->setValue(127, ti);
	}
	
    // Only set rear motor if it is defined
    tmpVal = m_aircraft->gvMotor2ChannelBox->currentIndex()-1;
    if (tmpVal > -1) {
        field = obj->getField(mixerTypes.at(tmpVal));
        field->setValue("Servo");
        field = obj->getField(mixerVectors.at(tmpVal));
        resetField(field);
        ti = field->getElementNames().indexOf("ThrottleCurve2");
        field->setValue(127, ti);
    }
	
    obj->updated();

	//Output success message
    m_aircraft->gvStatusLabel->setText("Mixer generated");
	
    return true;
}

/**
 This function displays text and color formatting in order to help the user understand what channels have not yet been configured.
 */
void ConfigVehicleTypeWidget::throwGroundVehicleChannelConfigError(QString airframeType)
{
	//Initialize configuration error flag
	bool error=false;

	
	//Create a red block. All combo boxes are the same size, so any one should do as a model
	int size = m_aircraft->gvEngineChannelBox->style()->pixelMetric(QStyle::PM_SmallIconSize);
	QPixmap pixmap(size,size);
	pixmap.fill(QColor("red"));
	
	if (airframeType == "GroundVehicleCar" ) { //Car
		if(m_aircraft->gvMotor1ChannelBox->currentText() == "None" && m_aircraft->gvMotor2ChannelBox->currentText() == "None"){
			pixmap.fill(QColor("green"));
			m_aircraft->gvMotor1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			m_aircraft->gvMotor2ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
//			m_aircraft->gvMotor1Label->setText("<font color='red'>" + m_aircraft->gvMotor1Label->text() + "</font>");
//			m_aircraft->gvMotor2Label->setText("<font color='red'>" + m_aircraft->gvMotor2Label->text() + "</font>");
			error=true;

		}
		else{
			m_aircraft->gvMotor1ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
			m_aircraft->gvMotor2ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
//			QTextEdit* htmlText=new QTextEdit(m_aircraft->gvMotor1Label->text());  // HtmlText is any QString with html tags.
//			m_aircraft->gvMotor1Label->setText(htmlText->toPlainText());
//			delete htmlText;
//			
//			htmlText=new QTextEdit(m_aircraft->gvMotor2Label->text());  // HtmlText is any QString with html tags.
//			m_aircraft->gvMotor2Label->setText(htmlText->toPlainText());
		}
		
		if (m_aircraft->gvSteering1ChannelBox->currentText() == "None" && m_aircraft->gvSteering2ChannelBox->currentText() == "None") {
			m_aircraft->gvSteering1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			m_aircraft->gvSteering2ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
//			m_aircraft->gvStatusLabel->setText("<font color='red'>ERROR: check steering channel assignment</font>");
//			m_aircraft->gvSteering1Label->setText("<font color='red'>" + m_aircraft->gvSteering1Label->text() + "</font>");
//			m_aircraft->gvSteering2Label->setText("<font color='red'>" + m_aircraft->gvSteering2Label->text() + "</font>");
			error=true;
		}
		else{
			m_aircraft->gvSteering1ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
			m_aircraft->gvSteering2ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
//			QTextEdit* htmlText=new QTextEdit(m_aircraft->gvSteering1Label->text());  // HtmlText is any QString with html tags.
//			m_aircraft->gvSteering1Label->setText(htmlText->toPlainText());
//			delete htmlText;
//			
//			htmlText=new QTextEdit(m_aircraft->gvSteering2Label->text());  // HtmlText is any QString with html tags.
//			m_aircraft->gvSteering2Label->setText(htmlText->toPlainText());
		}
		
	} else if (airframeType == "GroundVehicleDifferential"){ //Tank
		if(m_aircraft->gvMotor1ChannelBox->currentText() == "None" || m_aircraft->gvMotor2ChannelBox->currentText() == "None"){
			m_aircraft->gvMotor1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			m_aircraft->gvMotor2ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 			
			error=true;
		}
		else{
			m_aircraft->gvMotor1ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
			m_aircraft->gvMotor2ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}
		
		//Always reset
		m_aircraft->gvSteering1ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		m_aircraft->gvSteering2ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
	} else if ( airframeType == "GroundVehicleMotorcycle"){ //Motorcycle
		if(m_aircraft->gvMotor1ChannelBox->currentText() == "None" && m_aircraft->gvMotor2ChannelBox->currentText() == "None"){
			m_aircraft->gvMotor2ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 			
			error=true;
		}
		else{
			m_aircraft->gvMotor2ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}
		
		if (m_aircraft->gvSteering1ChannelBox->currentText() == "None" && m_aircraft->gvSteering2ChannelBox->currentText() == "None") {
			m_aircraft->gvSteering1ChannelBox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes 
			error=true;
		}
		else{
			m_aircraft->gvSteering1ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		}
		
		//Always reset
		m_aircraft->gvMotor1ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
		m_aircraft->gvSteering2ChannelBox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes 
	}
		
	if (error){
		m_aircraft->gvStatusLabel->setText(QString("<font color='red'>ERROR: Assign all necessary channels</font>"));
	}
}

