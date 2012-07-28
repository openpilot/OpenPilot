/**
 ******************************************************************************
 *
 * @file       configmultirotorwidget.cpp
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
#include "configmultirotorwidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QBrush>
#include <math.h>
#include <QMessageBox>

#include "mixersettings.h"
#include "systemsettings.h"
#include "actuatorsettings.h"
#include "actuatorcommand.h"

//#define  Pi 3.14159265358979323846


/**
 Constructor
 */
ConfigMultiRotorWidget::ConfigMultiRotorWidget(Ui_AircraftWidget *aircraft, QWidget *parent) : VehicleConfig(parent)
{
    m_aircraft = aircraft;
}

/**
 Destructor
 */
ConfigMultiRotorWidget::~ConfigMultiRotorWidget()
{
   // Do nothing
}


void ConfigMultiRotorWidget::setupUI(QString frameType)
{
    Q_ASSERT(m_aircraft);
    Q_ASSERT(uiowner);
    Q_ASSERT(quad);

    int i;

    // set aircraftType to Multirotor, disable triyaw channel
    setComboCurrentIndex(m_aircraft->aircraftType, m_aircraft->aircraftType->findText("Multirotor"));
    m_aircraft->triYawChannelBox->setEnabled(false);

    // disable all motor channel boxes
    for (i=1; i <=8; i++) {
        // do it manually so we can turn off any error decorations
        QComboBox *combobox = qFindChild<QComboBox*>(uiowner, "multiMotorChannelBox" + QString::number(i));
        if (combobox) {
            combobox->setEnabled(false);
            combobox->setItemData(0, 0, Qt::DecorationRole);
        }
    }

    if (frameType == "Tri" || frameType == "Tricopter Y") {
        setComboCurrentIndex( m_aircraft->multirotorFrameType, m_aircraft->multirotorFrameType->findText("Tricopter Y"));
		quad->setElementId("tri");

		//Enable all necessary motor channel boxes...
        for (i=1; i <=3; i++) {
            enableComboBox(uiowner, QString("multiMotorChannelBox%0").arg(i), true);
		}
		
        m_aircraft->mrRollMixLevel->setValue(100);
        m_aircraft->mrPitchMixLevel->setValue(100);
        m_aircraft->mrYawMixLevel->setValue(50);

        m_aircraft->triYawChannelBox->setEnabled(true);
    }
    else if (frameType == "QuadX" || frameType == "Quad X") {
        setComboCurrentIndex( m_aircraft->multirotorFrameType, m_aircraft->multirotorFrameType->findText("Quad X"));
        quad->setElementId("quad-x");
		
		//Enable all necessary motor channel boxes...
        for (i=1; i <=4; i++) {
            enableComboBox(uiowner, QString("multiMotorChannelBox%0").arg(i), true);
		}

        // init mixer levels
		m_aircraft->mrRollMixLevel->setValue(50);
		m_aircraft->mrPitchMixLevel->setValue(50);
		m_aircraft->mrYawMixLevel->setValue(50);
    }
    else if (frameType == "QuadP" || frameType == "Quad +") {
        setComboCurrentIndex( m_aircraft->multirotorFrameType, m_aircraft->multirotorFrameType->findText("Quad +"));
		quad->setElementId("quad-plus");
		
		//Enable all necessary motor channel boxes...
        for (i=1; i <=4; i++) {
            enableComboBox(uiowner, QString("multiMotorChannelBox%0").arg(i), true);
		}

		m_aircraft->mrRollMixLevel->setValue(100);
		m_aircraft->mrPitchMixLevel->setValue(100);
		m_aircraft->mrYawMixLevel->setValue(50);
    }
    else if (frameType == "Hexa" || frameType == "Hexacopter")
    {
        setComboCurrentIndex( m_aircraft->multirotorFrameType, m_aircraft->multirotorFrameType->findText("Hexacopter"));
		quad->setElementId("quad-hexa");
		
		//Enable all necessary motor channel boxes...
        for (i=1; i <=6; i++) {
            enableComboBox(uiowner, QString("multiMotorChannelBox%0").arg(i), true);
		}

		m_aircraft->mrRollMixLevel->setValue(50);
		m_aircraft->mrPitchMixLevel->setValue(33);
		m_aircraft->mrYawMixLevel->setValue(33);
    }
    else if (frameType == "HexaX" || frameType == "Hexacopter X" ) {
        setComboCurrentIndex( m_aircraft->multirotorFrameType, m_aircraft->multirotorFrameType->findText("Hexacopter X"));
		quad->setElementId("quad-hexa-H");
		
		//Enable all necessary motor channel boxes...
        for (i=1; i <=6; i++) {
            enableComboBox(uiowner, QString("multiMotorChannelBox%0").arg(i), true);
		}

		m_aircraft->mrRollMixLevel->setValue(33);
		m_aircraft->mrPitchMixLevel->setValue(50);
		m_aircraft->mrYawMixLevel->setValue(33);
		
    }
    else if (frameType == "HexaCoax" || frameType == "Hexacopter Y6")
    {
        setComboCurrentIndex( m_aircraft->multirotorFrameType, m_aircraft->multirotorFrameType->findText("Hexacopter Y6"));
		quad->setElementId("hexa-coax");
		
		//Enable all necessary motor channel boxes...
        for (i=1; i <=6; i++) {
            enableComboBox(uiowner, QString("multiMotorChannelBox%0").arg(i), true);
		}

		m_aircraft->mrRollMixLevel->setValue(100);
		m_aircraft->mrPitchMixLevel->setValue(50);
		m_aircraft->mrYawMixLevel->setValue(66);
		
    }
    else if (frameType == "Octo" || frameType == "Octocopter")
    {
        setComboCurrentIndex( m_aircraft->multirotorFrameType, m_aircraft->multirotorFrameType->findText("Octocopter"));
		quad->setElementId("quad-octo");
		
		//Enable all necessary motor channel boxes
        for (i=1; i <=8; i++) {
            enableComboBox(uiowner, QString("multiMotorChannelBox%0").arg(i), true);
		}		

		m_aircraft->mrRollMixLevel->setValue(33);
		m_aircraft->mrPitchMixLevel->setValue(33);
		m_aircraft->mrYawMixLevel->setValue(25);
    }
    else if (frameType == "OctoV" || frameType == "Octocopter V")
    {
        setComboCurrentIndex( m_aircraft->multirotorFrameType, m_aircraft->multirotorFrameType->findText("Octocopter V"));
		quad->setElementId("quad-octo-v");

		//Enable all necessary motor channel boxes
        for (i=1; i <=8; i++) {
            enableComboBox(uiowner, QString("multiMotorChannelBox%0").arg(i), true);
		}		

		m_aircraft->mrRollMixLevel->setValue(25);
		m_aircraft->mrPitchMixLevel->setValue(25);
		m_aircraft->mrYawMixLevel->setValue(25);
		
    }
    else if (frameType == "OctoCoaxP" || frameType == "Octo Coax +")
    {
        setComboCurrentIndex( m_aircraft->multirotorFrameType, m_aircraft->multirotorFrameType->findText("Octo Coax +"));
		quad->setElementId("octo-coax-P");

		//Enable all necessary motor channel boxes
		for (int i=1; i <=8; i++) {            
            enableComboBox(uiowner, QString("multiMotorChannelBox%0").arg(i), true);
		}		

		m_aircraft->mrRollMixLevel->setValue(100);
		m_aircraft->mrPitchMixLevel->setValue(100);
		m_aircraft->mrYawMixLevel->setValue(50);
		
    }
    else if (frameType == "OctoCoaxX" || frameType == "Octo Coax X")
    {
        setComboCurrentIndex( m_aircraft->multirotorFrameType, m_aircraft->multirotorFrameType->findText("Octo Coax X"));
		quad->setElementId("octo-coax-X");

		//Enable all necessary motor channel boxes
		for (int i=1; i <=8; i++) {            
            enableComboBox(uiowner, QString("multiMotorChannelBox%0").arg(i), true);
		}		

		m_aircraft->mrRollMixLevel->setValue(50);
		m_aircraft->mrPitchMixLevel->setValue(50);
		m_aircraft->mrYawMixLevel->setValue(50);
				
	} 	
}

void ConfigMultiRotorWidget::ResetActuators(GUIConfigDataUnion* configData)
{
    configData->multi.VTOLMotorN = 0;
    configData->multi.VTOLMotorNE = 0;
    configData->multi.VTOLMotorE = 0;
    configData->multi.VTOLMotorSE = 0;
    configData->multi.VTOLMotorS = 0;
    configData->multi.VTOLMotorSW = 0;
    configData->multi.VTOLMotorW = 0;
    configData->multi.VTOLMotorNW = 0;
    configData->multi.TRIYaw = 0;
}

QStringList ConfigMultiRotorWidget::getChannelDescriptions()
{
    int i;
    QStringList channelDesc;

    // init a channel_numelem list of channel desc defaults
    for (i=0; i < (int)(ConfigMultiRotorWidget::CHANNEL_NUMELEM); i++)
    {
        channelDesc.append(QString("-"));
    }

    // get the gui config data
    GUIConfigDataUnion configData = GetConfigData();
    multiGUISettingsStruct multi = configData.multi;

    if (multi.VTOLMotorN > 0 && multi.VTOLMotorN < ConfigMultiRotorWidget::CHANNEL_NUMELEM)
        channelDesc[multi.VTOLMotorN-1] = QString("VTOLMotorN");
    if (multi.VTOLMotorNE > 0 && multi.VTOLMotorNE < ConfigMultiRotorWidget::CHANNEL_NUMELEM)
        channelDesc[multi.VTOLMotorNE-1] = QString("VTOLMotorNE");
    if (multi.VTOLMotorNW > 0 && multi.VTOLMotorNW < ConfigMultiRotorWidget::CHANNEL_NUMELEM)
        channelDesc[multi.VTOLMotorNW-1] = QString("VTOLMotorNW");
    if (multi.VTOLMotorS > 0 && multi.VTOLMotorS < ConfigMultiRotorWidget::CHANNEL_NUMELEM)
        channelDesc[multi.VTOLMotorS-1] = QString("VTOLMotorS");
    if (multi.VTOLMotorSE > 0 && multi.VTOLMotorSE < ConfigMultiRotorWidget::CHANNEL_NUMELEM)
        channelDesc[multi.VTOLMotorSE-1] = QString("VTOLMotorSE");
    if (multi.VTOLMotorSW > 0 && multi.VTOLMotorSW < ConfigMultiRotorWidget::CHANNEL_NUMELEM)
        channelDesc[multi.VTOLMotorSW-1] = QString("VTOLMotorSW");
    if (multi.VTOLMotorW > 0 && multi.VTOLMotorW < ConfigMultiRotorWidget::CHANNEL_NUMELEM)
        channelDesc[multi.VTOLMotorW-1] = QString("VTOLMotorW");
    if (multi.VTOLMotorE > 0 && multi.VTOLMotorE < ConfigMultiRotorWidget::CHANNEL_NUMELEM)
        channelDesc[multi.VTOLMotorE-1] = QString("VTOLMotorE");
    if (multi.TRIYaw > 0 && multi.TRIYaw <= ConfigMultiRotorWidget::CHANNEL_NUMELEM)
        channelDesc[multi.TRIYaw-1] = QString("Tri-Yaw");

    return channelDesc;
}




/**
 Helper function to update the UI widget objects
 */
QString ConfigMultiRotorWidget::updateConfigObjectsFromWidgets()
{
    QString airframeType;
    QList<QString> motorList;

    UAVDataObject* mixerObj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixerObj);

    // Curve is also common to all quads:
    setThrottleCurve(mixerObj, VehicleConfig::MIXER_THROTTLECURVE1, m_aircraft->multiThrottleCurve->getCurve() );

    if (m_aircraft->multirotorFrameType->currentText() == "Quad +") {
        airframeType = "QuadP";
        setupQuad(true);
    } else if (m_aircraft->multirotorFrameType->currentText() == "Quad X") {
        airframeType = "QuadX";
        setupQuad(false);
    } else if (m_aircraft->multirotorFrameType->currentText() == "Hexacopter") {
        airframeType = "Hexa";
        setupHexa(true);
    } else if (m_aircraft->multirotorFrameType->currentText() == "Hexacopter X") {
        airframeType = "HexaX";
        setupHexa(false);
    } else if (m_aircraft->multirotorFrameType->currentText() == "Hexacopter Y6") {
        airframeType = "HexaCoax";

        //Show any config errors in GUI
        if (throwConfigError(6)) {
            return airframeType;
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
        setupMultiRotorMixer(mixer);
        m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");

    } else if (m_aircraft->multirotorFrameType->currentText() == "Octocopter") {
        airframeType = "Octo";

        //Show any config errors in GUI
        if (throwConfigError(8)) {
            return airframeType;

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
        setupMultiRotorMixer(mixer);
        m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");

    } else if (m_aircraft->multirotorFrameType->currentText() == "Octocopter V") {
        airframeType = "OctoV";

        //Show any config errors in GUI
        if (throwConfigError(8)) {
            return airframeType;
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
        setupMultiRotorMixer(mixer);
        m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");

    } else if (m_aircraft->multirotorFrameType->currentText() == "Octo Coax +") {
        airframeType = "OctoCoaxP";

        //Show any config errors in GUI
        if (throwConfigError(8)) {
            return airframeType;
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
        setupMultiRotorMixer(mixer);
        m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");

    } else if (m_aircraft->multirotorFrameType->currentText() == "Octo Coax X") {
        airframeType = "OctoCoaxX";

        //Show any config errors in GUI
        if (throwConfigError(8)) {
            return airframeType;
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
        setupMultiRotorMixer(mixer);
        m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");

    } else if (m_aircraft->multirotorFrameType->currentText() == "Tricopter Y") {
        airframeType = "Tri";

        //Show any config errors in GUI
        if (throwConfigError(3)) {
            return airframeType;

        }
        if (m_aircraft->triYawChannelBox->currentText() == "None") {
            m_aircraft->mrStatusLabel->setText("<font color='red'>Error: Assign a Yaw channel</font>");
            return airframeType;
        }
        motorList << "VTOLMotorNW" << "VTOLMotorNE" << "VTOLMotorS" << "" << "" << "" << "" << "" << "VTOLYaw";
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
        setupMultiRotorMixer(mixer);

        //tell the mixer about tricopter yaw channel

        int channel = m_aircraft->triYawChannelBox->currentIndex()-1;
        if (channel > -1){
            setMixerType(mixerObj, channel, VehicleConfig::MIXERTYPE_SERVO);
            setMixerVectorValue(mixerObj, channel, VehicleConfig::MIXERVECTOR_YAW, 127);
        }
		
		m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
		
    }
	
	return airframeType;
}



/**
 Helper function to refresh the UI widget values
 */
void ConfigMultiRotorWidget::refreshWidgetsValues(QString frameType)
{
    int channel;
    double value;

    GUIConfigDataUnion config = GetConfigData();
    multiGUISettingsStruct multi = config.multi;

    UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

	if (frameType == "QuadP") {
        // Motors 1/2/3/4 are: N / E / S / W
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox1,multi.VTOLMotorN);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox2,multi.VTOLMotorE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox3,multi.VTOLMotorS);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox4,multi.VTOLMotorW);

		// Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
		// This assumes that all vectors are identical - if not, the user should use the
		// "custom" setting.

        channel = m_aircraft->multiMotorChannelBox1->currentIndex() - 1;
        if (channel > -1)
        {
            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH);
            m_aircraft->mrPitchMixLevel->setValue( value/1.27 );

            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW);
            m_aircraft->mrYawMixLevel->setValue( 1-value/1.27 );

            channel = m_aircraft->multiMotorChannelBox2->currentIndex() - 1;
            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL);
            m_aircraft->mrRollMixLevel->setValue( -value/1.27);

        }
	} else if (frameType == "QuadX") {
        // Motors 1/2/3/4 are: NW / NE / SE / SW
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox1,multi.VTOLMotorNW);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox2,multi.VTOLMotorNE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox3,multi.VTOLMotorSE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox4,multi.VTOLMotorSW);

        // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
        // This assumes that all vectors are identical - if not, the user should use the
        // "custom" setting.
        channel = m_aircraft->multiMotorChannelBox1->currentIndex() - 1;
        if (channel > -1)
        {
            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH);
            m_aircraft->mrPitchMixLevel->setValue( value/1.27 );

            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW);
            m_aircraft->mrYawMixLevel->setValue( 1-value/1.27 );

            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL);
            m_aircraft->mrRollMixLevel->setValue( value/1.27);

        }

	} else if (frameType == "Hexa") {
		// Motors 1/2/3 4/5/6 are: N / NE / SE / S / SW / NW

        setComboCurrentIndex(m_aircraft->multiMotorChannelBox1,multi.VTOLMotorN);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox2,multi.VTOLMotorNE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox3,multi.VTOLMotorSE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox4,multi.VTOLMotorS);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox5,multi.VTOLMotorSW);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox6,multi.VTOLMotorNW);

        // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
        // This assumes that all vectors are identical - if not, the user should use the
        // "custom" setting.

        channel = m_aircraft->multiMotorChannelBox1->currentIndex() - 1;
        if (channel > -1)
        {
            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH);
            m_aircraft->mrPitchMixLevel->setValue( floor(value/1.27) );

            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW);
            m_aircraft->mrYawMixLevel->setValue( floor(-value/1.27) );

            //change channels
            channel = m_aircraft->multiMotorChannelBox2->currentIndex() - 1;
            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL);
            m_aircraft->mrRollMixLevel->setValue( floor(1-value/1.27) );

        }


	} else if (frameType == "HexaX") {
        // Motors 1/2/3 4/5/6 are: NE / E / SE / SW / W / NW

        setComboCurrentIndex(m_aircraft->multiMotorChannelBox1,multi.VTOLMotorNE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox2,multi.VTOLMotorE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox3,multi.VTOLMotorSE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox4,multi.VTOLMotorSW);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox5,multi.VTOLMotorW);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox6,multi.VTOLMotorNW);

        // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
        // This assumes that all vectors are identical - if not, the user should use the
        // "custom" setting.

        channel = m_aircraft->multiMotorChannelBox1->currentIndex() - 1;
        if (channel > -1)
        {
            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH);
            m_aircraft->mrPitchMixLevel->setValue( floor(value/1.27) );

            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW);
            m_aircraft->mrYawMixLevel->setValue( floor(-value/1.27) );

            channel = m_aircraft->multiMotorChannelBox2->currentIndex() - 1;
            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL);
            m_aircraft->mrRollMixLevel->setValue( floor(1-value/1.27) );
        }
	} else if (frameType == "HexaCoax") {
        // Motors 1/2/3 4/5/6 are: NW/W NE/E S/SE

        setComboCurrentIndex(m_aircraft->multiMotorChannelBox1,multi.VTOLMotorNW);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox2,multi.VTOLMotorW);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox3,multi.VTOLMotorNE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox4,multi.VTOLMotorE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox5,multi.VTOLMotorS);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox6,multi.VTOLMotorSE);

        // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
        // This assumes that all vectors are identical - if not, the user should use the
        // "custom" setting.
        channel = m_aircraft->multiMotorChannelBox1->currentIndex() - 1;
        if (channel > -1)
        {
            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH);
            m_aircraft->mrPitchMixLevel->setValue( value/1.27 );

            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW);
            m_aircraft->mrYawMixLevel->setValue( value/1.27 );

            channel = m_aircraft->multiMotorChannelBox2->currentIndex() - 1;
            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL);
            m_aircraft->mrRollMixLevel->setValue( value/1.27);
        }
	}  else if (frameType == "Octo" || frameType == "OctoV" ||
				frameType == "OctoCoaxP") {
        // Motors 1 to 8 are N / NE / E / etc

        setComboCurrentIndex(m_aircraft->multiMotorChannelBox1,multi.VTOLMotorN);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox2,multi.VTOLMotorNE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox3,multi.VTOLMotorE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox4,multi.VTOLMotorSE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox5,multi.VTOLMotorS);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox6,multi.VTOLMotorSW);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox7,multi.VTOLMotorW);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox8,multi.VTOLMotorNW);

        // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
        // This assumes that all vectors are identical - if not, the user should use the
        // "custom" setting.
        channel = m_aircraft->multiMotorChannelBox1->currentIndex() - 1;
        if (channel > -1)
        {
            if (frameType == "Octo") {
                value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH);
                m_aircraft->mrPitchMixLevel->setValue( floor(value/1.27) );

                value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW);
                m_aircraft->mrYawMixLevel->setValue( floor(-value/1.27) );

                //change channels
                channel = m_aircraft->multiMotorChannelBox2->currentIndex() - 1;
                value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL);
                m_aircraft->mrRollMixLevel->setValue( floor(-value/1.27) );
            }
            else if (frameType == "OctoV") {
                value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH);
                m_aircraft->mrPitchMixLevel->setValue( floor(value/1.27) );

                value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW);
                m_aircraft->mrYawMixLevel->setValue( floor(-value/1.27) );

                //change channels
                channel = m_aircraft->multiMotorChannelBox2->currentIndex() - 1;
                value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL);
                m_aircraft->mrRollMixLevel->setValue( floor(-value/1.27) );
            }
            else if (frameType == "OctoCoaxP") {
                value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH);
                m_aircraft->mrPitchMixLevel->setValue( floor(value/1.27) );

                value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW);
                m_aircraft->mrYawMixLevel->setValue( floor(-value/1.27) );

                //change channels
                channel = m_aircraft->multiMotorChannelBox3->currentIndex() - 1;
                value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL);
                m_aircraft->mrRollMixLevel->setValue( floor(-value/1.27) );
            }

        }
	} else if (frameType == "OctoCoaxX") {
        // Motors 1 to 8 are N / NE / E / etc

        setComboCurrentIndex(m_aircraft->multiMotorChannelBox1,multi.VTOLMotorNW);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox2,multi.VTOLMotorN);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox3,multi.VTOLMotorNE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox4,multi.VTOLMotorE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox5,multi.VTOLMotorSE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox6,multi.VTOLMotorS);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox7,multi.VTOLMotorSW);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox8,multi.VTOLMotorW);

        // Now, read the 1st mixer R/P/Y levels and initialize the mix sliders.
        // This assumes that all vectors are identical - if not, the user should use the
        // "custom" setting.
        channel = m_aircraft->multiMotorChannelBox1->currentIndex() - 1;
        if (channel > -1)
        {
            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH);
            m_aircraft->mrPitchMixLevel->setValue( floor(value/1.27) );

            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW);
            m_aircraft->mrYawMixLevel->setValue( floor(-value/1.27) );

            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL);
            m_aircraft->mrRollMixLevel->setValue( floor(value/1.27) );
        }
    } else if (frameType == "Tri") {
        // Motors 1 to 8 are N / NE / E / etc

        setComboCurrentIndex(m_aircraft->multiMotorChannelBox1,multi.VTOLMotorNW);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox2,multi.VTOLMotorNE);
        setComboCurrentIndex(m_aircraft->multiMotorChannelBox3,multi.VTOLMotorS);
        setComboCurrentIndex(m_aircraft->triYawChannelBox,multi.TRIYaw);

        channel = m_aircraft->multiMotorChannelBox1->currentIndex() - 1;
        if (channel > -1)
        {
            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH);
            m_aircraft->mrPitchMixLevel->setValue( floor(2*value/1.27) );

            value = getMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL);
            m_aircraft->mrRollMixLevel->setValue( floor(value/1.27) );

        }
	}
}



/**
 Helper function: setupQuadMotor
 */
void ConfigMultiRotorWidget::setupQuadMotor(int channel, double pitch, double roll, double yaw)
{
    qDebug()<<QString("Setup quad motor channel=%0 pitch=%1 roll=%2 yaw=%3").arg(channel).arg(pitch).arg(roll).arg(yaw);

    UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    setMixerType(mixer, channel, VehicleConfig::MIXERTYPE_MOTOR);

    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE1, 127);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE2, 0);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL, roll*127);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH, pitch*127);
    setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW, yaw*127);
}



/**
 Helper function: setup motors. Takes a list of channel names in input.
 */
void ConfigMultiRotorWidget::setupMotors(QList<QString> motorList)
{
    QList<QComboBox*> mmList;
    mmList << m_aircraft->multiMotorChannelBox1 << m_aircraft->multiMotorChannelBox2 << m_aircraft->multiMotorChannelBox3
           << m_aircraft->multiMotorChannelBox4 << m_aircraft->multiMotorChannelBox5 << m_aircraft->multiMotorChannelBox6
           << m_aircraft->multiMotorChannelBox7 << m_aircraft->multiMotorChannelBox8 << m_aircraft->triYawChannelBox;

    GUIConfigDataUnion configData = GetConfigData();
    ResetActuators(&configData);

    int index;
    foreach (QString motor, motorList) {

        index = mmList.takeFirst()->currentIndex();

        //qDebug()<<QString("Setup motor: %0 = %1").arg(motor).arg(index);

        if (motor == QString("VTOLMotorN"))
            configData.multi.VTOLMotorN = index;
        else if (motor == QString("VTOLMotorNE"))
            configData.multi.VTOLMotorNE = index;
        else if (motor == QString("VTOLMotorE"))
            configData.multi.VTOLMotorE = index;
        else if (motor == QString("VTOLMotorSE"))
            configData.multi.VTOLMotorSE = index;
        else if (motor == QString( "VTOLMotorS"))
            configData.multi.VTOLMotorS = index;
        else if (motor == QString( "VTOLMotorSW"))
            configData.multi.VTOLMotorSW = index;
        else if (motor == QString( "VTOLMotorW"))
            configData.multi.VTOLMotorW = index;
        else if (motor == QString( "VTOLMotorNW"))
            configData.multi.VTOLMotorNW = index;
        else if (motor == QString( "VTOLYaw" ))
            configData.multi.TRIYaw = index;
    }
    SetConfigData(configData);

}



/**
 Set up a Quad-X or Quad-P mixer
 */
bool ConfigMultiRotorWidget::setupQuad(bool pLayout)
{
    // Check coherence:
	
	//Show any config errors in GUI
    if (throwConfigError(4)) {
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
        setupMultiRotorMixer(pMixer);
    } else {
        setupMultiRotorMixer(xMixer);
    }
    m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
    return true;
}



/**
 Set up a Hexa-X or Hexa-P mixer
 */
bool ConfigMultiRotorWidget::setupHexa(bool pLayout)
{
    // Check coherence:
	//Show any config errors in GUI
    if (throwConfigError(6))
		return false;

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
		setupMultiRotorMixer(pMixer);
	} else {
		setupMultiRotorMixer(xMixer);
	}
	m_aircraft->mrStatusLabel->setText("SUCCESS: Mixer Saved OK");
	return true;
}


/**
 This function sets up the multirotor mixer values.
 */
bool ConfigMultiRotorWidget::setupMultiRotorMixer(double mixerFactors[8][3])
{
    qDebug()<<"Mixer factors";
    qDebug()<<mixerFactors[0][0]<<" "<<mixerFactors[0][1]<<" "<<mixerFactors[0][2];
    qDebug()<<mixerFactors[1][0]<<" "<<mixerFactors[1][1]<<" "<<mixerFactors[1][2];
    qDebug()<<mixerFactors[2][0]<<" "<<mixerFactors[2][1]<<" "<<mixerFactors[2][2];
    qDebug()<<mixerFactors[3][0]<<" "<<mixerFactors[3][1]<<" "<<mixerFactors[3][2];
    qDebug()<<mixerFactors[4][0]<<" "<<mixerFactors[4][1]<<" "<<mixerFactors[4][2];
    qDebug()<<mixerFactors[5][0]<<" "<<mixerFactors[5][1]<<" "<<mixerFactors[5][2];
    qDebug()<<mixerFactors[6][0]<<" "<<mixerFactors[6][1]<<" "<<mixerFactors[6][2];
    qDebug()<<mixerFactors[7][0]<<" "<<mixerFactors[7][1]<<" "<<mixerFactors[7][2];

    QList<QComboBox*> mmList;
    mmList << m_aircraft->multiMotorChannelBox1 << m_aircraft->multiMotorChannelBox2 << m_aircraft->multiMotorChannelBox3
	<< m_aircraft->multiMotorChannelBox4 << m_aircraft->multiMotorChannelBox5 << m_aircraft->multiMotorChannelBox6
	<< m_aircraft->multiMotorChannelBox7 << m_aircraft->multiMotorChannelBox8;

    UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    //disable all
    for (int channel=0; channel<VehicleConfig::CHANNEL_NUMELEM; channel++)
    {
        setMixerType(mixer,channel,VehicleConfig::MIXERTYPE_DISABLED);
        resetMixerVector(mixer, channel);
    }

    // and enable only the relevant channels:
    double pFactor = (double)m_aircraft->mrPitchMixLevel->value()/100;
    double rFactor = (double)m_aircraft->mrRollMixLevel->value()/100;
    double yFactor = (double)m_aircraft->mrYawMixLevel->value()/100;
    qDebug()<<QString("pFactor=%0 rFactor=%1 yFactor=%2").arg(pFactor).arg(rFactor).arg(yFactor);
    for (int i=0 ; i<8; i++) {
        if(mmList.at(i)->isEnabled())
        {
            int channel = mmList.at(i)->currentIndex()-1;
            if (channel > -1)
                setupQuadMotor(channel, mixerFactors[i][0]*pFactor,
							   rFactor*mixerFactors[i][1], yFactor*mixerFactors[i][2]);
        }
    }
	//    obj->updated();
    return true;
}


/**
 This function displays text and color formatting in order to help the user understand what channels have not yet been configured.
 */
bool ConfigMultiRotorWidget::throwConfigError(int numMotors)
{    
	//Initialize configuration error flag
	bool error=false;

	//Iterate through all instances of multiMotorChannelBox
	for (int i=0; i<numMotors; i++) {
		//Fine widgets with text "multiMotorChannelBox.x", where x is an integer
        QComboBox *combobox = qFindChild<QComboBox*>(uiowner, "multiMotorChannelBox" + QString::number(i+1));
        if (combobox){
            if (combobox->currentText() == "None") {
				int size = combobox->style()->pixelMetric(QStyle::PM_SmallIconSize);
				QPixmap pixmap(size,size);
				pixmap.fill(QColor("red"));
                combobox->setItemData(0, pixmap, Qt::DecorationRole);//Set color palettes
				error=true;
			}
			else {
                combobox->setItemData(0, 0, Qt::DecorationRole);//Reset color palettes
			}
		}
	}
	

	if (error){
		m_aircraft->mrStatusLabel->setText(QString("<font color='red'>ERROR: Assign all %1 motor channels</font>").arg(numMotors));
	}
    return error;
}


