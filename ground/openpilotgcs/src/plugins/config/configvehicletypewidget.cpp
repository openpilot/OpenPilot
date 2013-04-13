/**
 ******************************************************************************
 *
 * @file       configvehicletypewidget.cpp
 * @author     E. Lafargue, K. Sebesta & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012
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
#include "configvehicletypewidget.h"
#include "systemsettings.h"
#include "actuatorsettings.h"

#include "cfg_vehicletypes/configccpmwidget.h"
#include "cfg_vehicletypes/configfixedwingwidget.h"
#include "cfg_vehicletypes/configgroundvehiclewidget.h"
#include "cfg_vehicletypes/configmultirotorwidget.h"
#include "cfg_vehicletypes/configcustomwidget.h"

#include <QDebug>
#include <QStringList>
#include <QTimer>
#include <QtGui/QWidget>
#include <QDesktopServices>
#include <QUrl>

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/generalsettings.h>

/**
  Static function to get currently assigned channelDescriptions
  for all known vehicle types;  instantiates the appropriate object
  then asks it to supply channel descs
  */
QStringList ConfigVehicleTypeWidget::getChannelDescriptions()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr);

    // get an instance of systemsettings
    SystemSettings *systemSettings = SystemSettings::GetInstance(objMngr);
    Q_ASSERT(systemSettings);
    SystemSettings::DataFields systemSettingsData = systemSettings->getData();

    QStringList channelDesc;
    switch (systemSettingsData.AirframeType) {
    case SystemSettings::AIRFRAMETYPE_FIXEDWING:
    case SystemSettings::AIRFRAMETYPE_FIXEDWINGELEVON:
    case SystemSettings::AIRFRAMETYPE_FIXEDWINGVTAIL:
        // fixed wing
        channelDesc = ConfigFixedWingWidget::getChannelDescriptions();
        break;
    case SystemSettings::AIRFRAMETYPE_HELICP:
        // helicp
        channelDesc = ConfigCcpmWidget::getChannelDescriptions();
        break;
    case SystemSettings::AIRFRAMETYPE_VTOL:
    case SystemSettings::AIRFRAMETYPE_TRI:
    case SystemSettings::AIRFRAMETYPE_QUADX:
    case SystemSettings::AIRFRAMETYPE_QUADP:
    case SystemSettings::AIRFRAMETYPE_OCTOV:
    case SystemSettings::AIRFRAMETYPE_OCTOCOAXX:
    case SystemSettings::AIRFRAMETYPE_OCTOCOAXP:
    case SystemSettings::AIRFRAMETYPE_OCTO:
    case SystemSettings::AIRFRAMETYPE_HEXAX:
    case SystemSettings::AIRFRAMETYPE_HEXACOAX:
    case SystemSettings::AIRFRAMETYPE_HEXA:
        // multirotor
        channelDesc = ConfigMultiRotorWidget::getChannelDescriptions();
        break;
    case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLECAR:
    case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLEDIFFERENTIAL:
    case SystemSettings::AIRFRAMETYPE_GROUNDVEHICLEMOTORCYCLE:
        // ground
        channelDesc = ConfigGroundVehicleWidget::getChannelDescriptions();
        break;
    default:
        channelDesc = ConfigCustomWidget::getChannelDescriptions();
        break;
    }

    return channelDesc;
}

ConfigVehicleTypeWidget::ConfigVehicleTypeWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_aircraft = new Ui_AircraftWidget();
    m_aircraft->setupUi(this);
    
    ExtensionSystem::PluginManager *pm=ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings * settings=pm->getObject<Core::Internal::GeneralSettings>();
    if (!settings->useExpertMode()) {
        m_aircraft->saveAircraftToRAM->setVisible(false);
    }

    addApplySaveButtons(m_aircraft->saveAircraftToRAM, m_aircraft->saveAircraftToSD);

    addUAVObject("SystemSettings");
    addUAVObject("MixerSettings");
    addUAVObject("ActuatorSettings");

    ffTuningInProgress = false;
    ffTuningPhase = false;

    QStringList airframeTypes;
    airframeTypes << "Fixed Wing" << "Multirotor" << "Helicopter" << "Ground" << "Custom";
    m_aircraft->aircraftType->addItems(airframeTypes);

    // *****************************************************************************************************************

    // create and setup a FixedWing config widget
    qDebug() << "create fixedwing ui";
    m_fixedwing = new ConfigFixedWingWidget();
    m_aircraft->airframesWidget->addWidget(m_fixedwing);

    // create and setup a MultiRotor config widget
    m_multirotor = new ConfigMultiRotorWidget();
    m_aircraft->airframesWidget->addWidget(m_multirotor);

    // create and setup a Helicopter config widget
    m_heli = new ConfigCcpmWidget();
    m_aircraft->airframesWidget->addWidget(m_heli);

    // create and setup a GroundVehicle config widget
    m_groundvehicle = new ConfigGroundVehicleWidget();
    m_aircraft->airframesWidget->addWidget(m_groundvehicle);

    // create and setup a custom config widget
    m_custom = new ConfigCustomWidget();
    m_aircraft->airframesWidget->addWidget(m_custom);

    // *****************************************************************************************************************

    // Set default vehicle to MultiRotor
    m_aircraft->aircraftType->setCurrentIndex(1);
    // Force the tab index to match
    m_aircraft->airframesWidget->setCurrentIndex(1);

    // Generate lists of mixerTypeNames, mixerVectorNames, channelNames
    channelNames << "None";
    for (int i = 0; i < (int) ActuatorSettings::CHANNELADDR_NUMELEM; i++) {
        mixerTypes << QString("Mixer%1Type").arg(i + 1);
        mixerVectors << QString("Mixer%1Vector").arg(i + 1);
        channelNames << QString("Channel%1").arg(i + 1);
    }

    // NEW STYLE: Loop through the widgets looking for all widgets that have "ChannelBox" in their name
    // The upshot of this is that ALL new ComboBox widgets for selecting the output channel must have "ChannelBox" in their name
    // FOR WHATEVER REASON, THIS DOES NOT WORK WITH ChannelBox. ChannelBo is sufficiently accurate
    QList<QComboBox *> l = findChildren<QComboBox*>(QRegExp("\\S+ChannelBo\\S+"));
    foreach(QComboBox *combobox, l) {
        combobox->addItems(channelNames);
    }

	// Connect aircraft type selection dropbox to callback function
    connect(m_aircraft->aircraftType, SIGNAL(currentIndexChanged(int)), m_aircraft->airframesWidget, SLOT(setCurrentIndex(int)));
	
    // Connect the three feed forward test checkboxes
    connect(m_aircraft->ffTestBox1, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));
    connect(m_aircraft->ffTestBox2, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));
    connect(m_aircraft->ffTestBox3, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));

    // Connect the help pushbutton
    connect(m_aircraft->airframeHelp, SIGNAL(clicked()), this, SLOT(openHelp()));

    enableControls(false);

    refreshWidgetsValues();

    disableMouseWheelEvents();
}

/**
 Destructor
 */
ConfigVehicleTypeWidget::~ConfigVehicleTypeWidget()
{
   // Do nothing
}

/**
  Slot for switching the airframe type. We do it explicitely
  rather than a signal in the UI, because we want to force a fitInView of the quad shapes.
  This is because this method (fitinview) only works when the widget is shown.
  */
//void ConfigVehicleTypeWidget::switchAirframeType(int index)
//{
//    m_aircraft->airframesWidget->setCurrentIndex(index);
//}

/**
  Refreshes the current value of the SystemSettings which holds the aircraft type
  */
void ConfigVehicleTypeWidget::refreshWidgetsValues(UAVObject *o)
{
    Q_UNUSED(o);

    qDebug() << "ConfigVehicleTypeWidget::refreshWidgetsValues - begin";

    if (!allObjectsUpdated()) {
        return;
    }
	
    bool dirty = isDirty();
    qDebug() << "ConfigVehicleTypeWidget::refreshWidgetsValues - isDirty:" << dirty;
	
    // Get the Airframe type from the system settings:
    UAVDataObject *system = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("SystemSettings")));
    Q_ASSERT(system);

    UAVObjectField *field = system->getField(QString("AirframeType"));
    Q_ASSERT(field);

    // At this stage, we will need to have some hardcoded settings in this code, this
    // is not ideal, but there you go.
    QString frameType = field->getValue().toString();
    qDebug() << "ConfigVehicleTypeWidget::refreshWidgetsValues - frame type:" << frameType;
    //setupAirframeUI(frameType);

    QString category = "FixedWing";//frameCategory(frameType);
    if (category == "FixedWing") {
        // Retrieve fixed wing settings
        setComboCurrentIndex(m_aircraft->aircraftType, m_aircraft->aircraftType->findText("Fixed Wing"));
        m_fixedwing->refreshWidgetsValues(frameType);
    } else if (category == "Multirotor") {
        // Retrieve multirotor settings
        setComboCurrentIndex(m_aircraft->aircraftType, m_aircraft->aircraftType->findText("Multirotor"));
        m_multirotor->refreshWidgetsValues(frameType);
    } else if (category == "Helicopter") {
        setComboCurrentIndex(m_aircraft->aircraftType, m_aircraft->aircraftType->findText("Helicopter"));
        m_heli->refreshWidgetsValues(frameType);
    } else if (category == "Ground") {
        // Retrieve ground vehicle settings
        setComboCurrentIndex(m_aircraft->aircraftType, m_aircraft->aircraftType->findText("Ground"));
        m_groundvehicle->refreshWidgetsValues(frameType);
    } else if (category == "Custom") {
        // Retrieve custom settings
        setComboCurrentIndex(m_aircraft->aircraftType, m_aircraft->aircraftType->findText("Custom"));
        m_custom->refreshWidgetsValues(frameType);
	}	
	
    updateFeedForwardUI();

    setDirty(dirty);

    qDebug() << "ConfigVehicleTypeWidget::refreshWidgetsValues - end";
}

//QString ConfigVehicleTypeWidget::frameCategory1(QString frameType)
//{
//    QString category;
//    if (frameType.startsWith("FixedWing")) {
//        category = "FixedWing";
//    } else if (frameType == "Tri" || frameType == "QuadX" || frameType == "QuadP" || frameType == "Hexa"
//            || frameType == "HexaCoax" || frameType == "HexaX" || frameType == "Octo" || frameType == "OctoV"
//            || frameType == "OctoCoaxP" || frameType == "OctoCoaxX") {
//        category = "Multirotor";
//    } else if (frameType == "HeliCP") {
//        category = "Helicopter";
//    } else if (frameType.startsWith("GroundVehicle")) {
//        category = "Ground";
//    } else {
//        category = "Custom";
//    }
//    return category;
//}


QString ConfigVehicleTypeWidget::frameCategory(QString frameType)
{
    QString category;
    if (frameType == "FixedWing" || frameType == "Elevator aileron rudder" || frameType == "FixedWingElevon"
            || frameType == "Elevon" || frameType == "FixedWingVtail" || frameType == "Vtail") {
        category = "FixedWing";
    } else if (frameType == "Tri" || frameType == "Tricopter Y" || frameType == "QuadX" || frameType == "Quad X"
            || frameType == "QuadP" || frameType == "Quad +" || frameType == "Hexa" || frameType == "Hexacopter"
            || frameType == "HexaX" || frameType == "Hexacopter X" || frameType == "HexaCoax"
            || frameType == "Hexacopter Y6" || frameType == "Octo" || frameType == "Octocopter" || frameType == "OctoV"
            || frameType == "Octocopter V" || frameType == "OctoCoaxP" || frameType == "Octo Coax +"
            || frameType == "OctoCoaxX" || frameType == "Octo Coax X") {
        category = "Multirotor";
    } else if (frameType == "HeliCP") {
        category = "Helicopter";
    } else if (frameType == "GroundVehicleCar" || frameType == "Turnable (car)"
            || frameType == "GroundVehicleDifferential" || frameType == "Differential (tank)"
            || frameType == "GroundVehicleMotorcyle" || frameType == "Motorcycle") {
        category = "Ground";
    } else {
        category = "Custom";
    }
    return category;
}

/**
  Sends the config to the board (airframe type)

  We do all the tasks common to all airframes, or family of airframes, and
  we call additional methods for specific frames, so that we do not have a code
  that is too heavy.
*/
void ConfigVehicleTypeWidget::updateObjectsFromWidgets()
{
    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    QPointer<VehicleConfig> vconfig = new VehicleConfig();

    // Update feed forward settings
    vconfig->setMixerValue(mixer, "FeedForward", m_aircraft->feedForwardSlider->value() / 100.0);
    vconfig->setMixerValue(mixer, "AccelTime", m_aircraft->accelTime->value());
    vconfig->setMixerValue(mixer, "DecelTime", m_aircraft->decelTime->value());
    vconfig->setMixerValue(mixer, "MaxAccel", m_aircraft->maxAccelSlider->value());

    // Sets airframe type default to "Custom"
    QString airframeType = "Custom";
    if (m_aircraft->aircraftType->currentText() == "Fixed Wing") {
        airframeType = m_fixedwing->updateConfigObjectsFromWidgets();
    }
    else if (m_aircraft->aircraftType->currentText() == "Multirotor") {
         airframeType = m_multirotor->updateConfigObjectsFromWidgets();
    }
    else if (m_aircraft->aircraftType->currentText() == "Helicopter") {
         airframeType = m_heli->updateConfigObjectsFromWidgets();
    }
    else if (m_aircraft->aircraftType->currentText() == "Ground") {
         airframeType = m_groundvehicle->updateConfigObjectsFromWidgets();
    }
    else {
        airframeType = m_custom->updateConfigObjectsFromWidgets();
    }

    // set the airframe type
    UAVDataObject *system = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("SystemSettings")));
    Q_ASSERT(system);

    QPointer<UAVObjectField> field = system->getField(QString("AirframeType"));
    if (field) {
        field->setValue(airframeType);
    }

    updateFeedForwardUI();
}

/**
  Reset the contents of a field
  */
//void ConfigVehicleTypeWidget::resetField(UAVObjectField *field)
//{
//    for (unsigned int i = 0; i < field->getNumElements(); i++) {
//        field->setValue(0, i);
//    }
//}

/**
  Enables and runs feed forward testing
  */
void ConfigVehicleTypeWidget::enableFFTest()
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
            UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READONLY);
            obj->setMetadata(mdata);
        }
        // Depending on phase, either move actuator or send FF settings:
        if (ffTuningPhase) {
            // Send FF settings to the board
            UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
            Q_ASSERT(mixer);

            QPointer<VehicleConfig> vconfig = new VehicleConfig();

            // Update feed forward settings
            vconfig->setMixerValue(mixer, "FeedForward", m_aircraft->feedForwardSlider->value() / 100.0);
            vconfig->setMixerValue(mixer, "AccelTime", m_aircraft->accelTime->value());
            vconfig->setMixerValue(mixer, "DecelTime", m_aircraft->decelTime->value());
            vconfig->setMixerValue(mixer, "MaxAccel", m_aircraft->maxAccelSlider->value());
            mixer->updated();
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
  Updates the custom airframe settings based on the current airframe.

  Note: does NOT ask for an object refresh itself!
  */
void ConfigVehicleTypeWidget::updateFeedForwardUI()
{
    UAVDataObject* mixer = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    QPointer<VehicleConfig> vconfig = new VehicleConfig();

    // Update feed forward settings
    m_aircraft->feedForwardSlider->setValue(vconfig->getMixerValue(mixer, "FeedForward") * 100);
    m_aircraft->accelTime->setValue(vconfig->getMixerValue(mixer, "AccelTime"));
    m_aircraft->decelTime->setValue(vconfig->getMixerValue(mixer, "DecelTime"));
    m_aircraft->maxAccelSlider->setValue(vconfig->getMixerValue(mixer, "MaxAccel"));
}

/**
 Opens the wiki from the user's default browser
 */
void ConfigVehicleTypeWidget::openHelp()
{
    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/x/44Cf", QUrl::StrictMode) );
}

/**
  Helper function:
  Sets the current index on supplied combobox to index
  if it is within bounds 0 <= index < combobox.count()
 */
void ConfigVehicleTypeWidget::setComboCurrentIndex(QComboBox* box, int index)
{
    if (index >= 0 && index < box->count()) {
        box->setCurrentIndex(index);
    }
}
