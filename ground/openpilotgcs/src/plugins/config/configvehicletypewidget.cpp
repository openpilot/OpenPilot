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
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <math.h>
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
        channelDesc = ConfigFixedWingWidget::getChannelDescriptions();
        break;
    case SystemSettings::AIRFRAMETYPE_FIXEDWINGELEVON:
        channelDesc = ConfigFixedWingWidget::getChannelDescriptions();
        break;
    case SystemSettings::AIRFRAMETYPE_FIXEDWINGVTAIL:
	// do nothing for vtail support for the time being. 
        // channelDesc = ConfigFixedWingWidget::getChannelDescriptions();
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
    case SystemSettings::AIRFRAMETYPE_OCTOX:
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

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings *settings = pm->getObject<Core::Internal::GeneralSettings>();
    if (!settings->useExpertMode()) {
        m_aircraft->saveAircraftToRAM->setVisible(false);
    }

    addApplySaveButtons(m_aircraft->saveAircraftToRAM, m_aircraft->saveAircraftToSD);

    addUAVObject("SystemSettings");
    addUAVObject("MixerSettings");
    addUAVObject("ActuatorSettings");

    m_ffTuningInProgress = false;
    m_ffTuningPhase = false;

    // The order of the tabs is important since they correspond with the AirframCategory enum
    m_aircraft->aircraftType->addTab(tr("Multirotor"));
    m_aircraft->aircraftType->addTab(tr("Fixed Wing"));
    m_aircraft->aircraftType->addTab(tr("Helicopter"));
    m_aircraft->aircraftType->addTab(tr("Ground"));
    m_aircraft->aircraftType->addTab(tr("Custom"));

    // Connect aircraft type selection dropbox to callback function
    connect(m_aircraft->aircraftType, SIGNAL(currentChanged(int)), this, SLOT(switchAirframeType(int)));

    // Connect the three feed forward test checkboxes
    connect(m_aircraft->ffTestBox1, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));
    connect(m_aircraft->ffTestBox2, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));
    connect(m_aircraft->ffTestBox3, SIGNAL(clicked(bool)), this, SLOT(enableFFTest()));

    // Connect the help pushbutton
    connect(m_aircraft->airframeHelp, SIGNAL(clicked()), this, SLOT(openHelp()));

    refreshWidgetsValues();

    // register FF widgets for dirty state management
    addWidget(m_aircraft->feedForwardSlider);
    addWidget(m_aircraft->accelTime);
    addWidget(m_aircraft->decelTime);
    addWidget(m_aircraft->maxAccelSlider);
    addWidget(m_aircraft->ffTestBox1);
    addWidget(m_aircraft->ffTestBox2);
    addWidget(m_aircraft->ffTestBox3);

    disableMouseWheelEvents();
    updateEnableControls();
}

/**
   Destructor
 */
ConfigVehicleTypeWidget::~ConfigVehicleTypeWidget()
{
    // Do nothing
}

void ConfigVehicleTypeWidget::switchAirframeType(int index)
{
    m_aircraft->airframesWidget->setCurrentWidget(getVehicleConfigWidget(index));
}

/**
   Refreshes the current value of the SystemSettings which holds the aircraft type
   Note: The default behavior of ConfigTaskWidget is bypassed.
   Therefore no automatic synchronization of UAV Objects to UI is done.
 */
void ConfigVehicleTypeWidget::refreshWidgetsValues(UAVObject *o)
{
    Q_UNUSED(o);

    if (!allObjectsUpdated()) {
        return;
    }

    bool dirty = isDirty();

    // Get the Airframe type from the system settings:
    UAVDataObject *system = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("SystemSettings")));
    Q_ASSERT(system);

    UAVObjectField *field = system->getField(QString("AirframeType"));
    Q_ASSERT(field);

    // At this stage, we will need to have some hardcoded settings in this code
    QString frameType = field->getValue().toString();

    int category = frameCategory(frameType);
    m_aircraft->aircraftType->setCurrentIndex(category);

    VehicleConfig *vehicleConfig = getVehicleConfigWidget(category);
    if (vehicleConfig) {
        vehicleConfig->refreshWidgetsValues(frameType);
    }

    updateFeedForwardUI();

    setDirty(dirty);
}

/**
   Sends the config to the board (airframe type)

   We do all the tasks common to all airframes, or family of airframes, and
   we call additional methods for specific frames, so that we do not have a code
   that is too heavy.

   Note: The default behavior of ConfigTaskWidget is bypassed.
   Therefore no automatic synchronization of UI to UAV Objects is done.
 */
void ConfigVehicleTypeWidget::updateObjectsFromWidgets()
{
    // Airframe type defaults to Custom
    QString airframeType = "Custom";

    VehicleConfig *vehicleConfig = (VehicleConfig *)m_aircraft->airframesWidget->currentWidget();

    if (vehicleConfig) {
        airframeType = vehicleConfig->updateConfigObjectsFromWidgets();
    }

    // set the airframe type
    UAVDataObject *system = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("SystemSettings")));
    Q_ASSERT(system);

    QPointer<UAVObjectField> field = system->getField(QString("AirframeType"));
    if (field) {
        field->setValue(airframeType);
    }

    // Update feed forward settings
    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
    Q_ASSERT(mixer);

    QPointer<VehicleConfig> vconfig = new VehicleConfig();

    vconfig->setMixerValue(mixer, "FeedForward", m_aircraft->feedForwardSlider->value() / 100.0);
    vconfig->setMixerValue(mixer, "AccelTime", m_aircraft->accelTime->value());
    vconfig->setMixerValue(mixer, "DecelTime", m_aircraft->decelTime->value());
    vconfig->setMixerValue(mixer, "MaxAccel", m_aircraft->maxAccelSlider->value());

    // TODO call refreshWidgetsValues() to reflect actual saved values ?
    updateFeedForwardUI();
}

int ConfigVehicleTypeWidget::frameCategory(QString frameType)
{
    if (frameType == "FixedWing" || frameType == "Aileron Dual Servo" || frameType == "Aileron Single Servo"
        || frameType == "FixedWingElevon" || frameType == "Elevon" || frameType == "FixedWingVtail"
        || frameType == "Vtail") {
        return ConfigVehicleTypeWidget::FIXED_WING;
    } else if (frameType == "Tri" || frameType == "Tricopter Y" || frameType == "QuadX" || frameType == "Quad X"
               || frameType == "QuadP" || frameType == "Quad +" || frameType == "Hexa" || frameType == "Hexacopter"
               || frameType == "HexaX" || frameType == "Hexacopter X" || frameType == "HexaCoax"
               || frameType == "HexaH" || frameType == "Hexacopter H" || frameType == "Hexacopter Y6"
               || frameType == "Octo" || frameType == "Octocopter"
               || frameType == "OctoX" || frameType == "Octocopter X"
               || frameType == "OctoV" || frameType == "Octocopter V"
               || frameType == "OctoCoaxP" || frameType == "Octo Coax +"
               || frameType == "OctoCoaxX" || frameType == "Octo Coax X") {
        return ConfigVehicleTypeWidget::MULTIROTOR;
    } else if (frameType == "HeliCP") {
        return ConfigVehicleTypeWidget::HELICOPTER;
    } else if (frameType == "GroundVehicleCar" || frameType == "Turnable (car)"
               || frameType == "GroundVehicleDifferential" || frameType == "Differential (tank)"
               || frameType == "GroundVehicleMotorcyle" || frameType == "Motorcycle") {
        return ConfigVehicleTypeWidget::GROUND;
    } else {
        return ConfigVehicleTypeWidget::CUSTOM;
    }
}

VehicleConfig *ConfigVehicleTypeWidget::getVehicleConfigWidget(int frameCategory)
{
    VehicleConfig *vehiculeConfig;

    if (!m_vehicleIndexMap.contains(frameCategory)) {
        // create config widget
        vehiculeConfig = createVehicleConfigWidget(frameCategory);
        // bind config widget "field" to this ConfigTaskWodget
        // this is necessary to get "dirty" state management
        vehiculeConfig->registerWidgets(*this);

        // add config widget to UI
        int index = m_aircraft->airframesWidget->insertWidget(m_aircraft->airframesWidget->count(), vehiculeConfig);
        m_vehicleIndexMap[frameCategory] = index;
        updateEnableControls();
    }
    int index = m_vehicleIndexMap.value(frameCategory);
    vehiculeConfig = (VehicleConfig *)m_aircraft->airframesWidget->widget(index);
    return vehiculeConfig;
}

VehicleConfig *ConfigVehicleTypeWidget::createVehicleConfigWidget(int frameCategory)
{
    if (frameCategory == ConfigVehicleTypeWidget::FIXED_WING) {
        return new ConfigFixedWingWidget();
    } else if (frameCategory == ConfigVehicleTypeWidget::MULTIROTOR) {
        return new ConfigMultiRotorWidget();
    } else if (frameCategory == ConfigVehicleTypeWidget::HELICOPTER) {
        return new ConfigCcpmWidget();
    } else if (frameCategory == ConfigVehicleTypeWidget::GROUND) {
        return new ConfigGroundVehicleWidget();
    } else if (frameCategory == ConfigVehicleTypeWidget::CUSTOM) {
        return new ConfigCustomWidget();
    }
    return NULL;
}

/**
   Enables and runs feed forward testing
 */
void ConfigVehicleTypeWidget::enableFFTest()
{
    // Role:
    // - Check if all three checkboxes are checked
    // - Every other timer event: toggle engine from 45% to 55%
    // - Every other time event: send FF settings to flight FW
    if (m_aircraft->ffTestBox1->isChecked() && m_aircraft->ffTestBox2->isChecked()
        && m_aircraft->ffTestBox3->isChecked()) {
        if (!m_ffTuningInProgress) {
            // Initiate tuning:
            UAVDataObject *obj = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(
                                                                   QString("ManualControlCommand")));
            UAVObject::Metadata mdata = obj->getMetadata();
            m_accInitialData = mdata;
            UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READONLY);
            obj->setMetadata(mdata);
        }
        // Depending on phase, either move actuator or send FF settings:
        if (m_ffTuningPhase) {
            // Send FF settings to the board
            UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));
            Q_ASSERT(mixer);

            QPointer<VehicleConfig> vconfig = new VehicleConfig();

            // Update feed forward settings
            vconfig->setMixerValue(mixer, "FeedForward", m_aircraft->feedForwardSlider->value() / 100.0);
            vconfig->setMixerValue(mixer, "AccelTime", m_aircraft->accelTime->value());
            vconfig->setMixerValue(mixer, "DecelTime", m_aircraft->decelTime->value());
            vconfig->setMixerValue(mixer, "MaxAccel", m_aircraft->maxAccelSlider->value());
            mixer->updated();
        } else {
            // Toggle motor state
            UAVDataObject *obj = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(
                                                                   QString("ManualControlCommand")));
            double value  = obj->getField("Throttle")->getDouble();
            double target = (value < 0.5) ? 0.55 : 0.45;
            obj->getField("Throttle")->setValue(target);
            obj->updated();
        }
        m_ffTuningPhase = !m_ffTuningPhase;
        m_ffTuningInProgress = true;
        QTimer::singleShot(1000, this, SLOT(enableFFTest()));
    } else {
        // - If no: disarm timer, restore actuatorcommand metadata
        // Disarm!
        if (m_ffTuningInProgress) {
            m_ffTuningInProgress = false;
            UAVDataObject *obj = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(
                                                                   QString("ManualControlCommand")));
            UAVObject::Metadata mdata = obj->getMetadata();
            mdata = m_accInitialData; // Restore metadata
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
    UAVDataObject *mixer = dynamic_cast<UAVDataObject *>(getObjectManager()->getObject(QString("MixerSettings")));

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
    QDesktopServices::openUrl(QUrl(tr("http://wiki.openpilot.org/x/44Cf"), QUrl::StrictMode));
}
