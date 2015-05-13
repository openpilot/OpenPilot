/**
 ******************************************************************************
 *
 * @file       setupwizard.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Setup Wizard  Plugin
 * @{
 * @brief A Wizard to make the initial setup easy for everyone.
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

#include "setupwizard.h"
#include "pages/opstartpage.h"
#include "pages/opendpage.h"
#include "pages/controllerpage.h"
#include "pages/vehiclepage.h"
#include "pages/multipage.h"
#include "pages/fixedwingpage.h"
#include "pages/airspeedpage.h"
#include "pages/gpspage.h"
#include "pages/helipage.h"
#include "pages/surfacepage.h"
#include "pages/inputpage.h"
#include "pages/escpage.h"
#include "pages/servopage.h"
#include "pages/biascalibrationpage.h"
#include "pages/esccalibrationpage.h"
#include "pages/summarypage.h"
#include "pages/savepage.h"
#include "pages/notyetimplementedpage.h"
#include "pages/outputcalibrationpage.h"
#include "pages/revocalibrationpage.h"
#include "pages/airframeinitialtuningpage.h"
#include "extensionsystem/pluginmanager.h"
#include "vehicleconfigurationhelper.h"
#include "actuatorsettings.h"
#include "pages/autoupdatepage.h"
#include "uploader/uploadergadgetfactory.h"

SetupWizard::SetupWizard(QWidget *parent) : QWizard(parent), VehicleConfigurationSource(),
    m_controllerType(CONTROLLER_UNKNOWN),
    m_vehicleType(VEHICLE_UNKNOWN), m_inputType(INPUT_UNKNOWN),
    m_escType(ESC_UNKNOWN), m_servoType(SERVO_UNKNOWN),
    m_airspeedType(AIRSPEED_DISABLED), m_gpsType(GPS_DISABLED),
    m_vehicleTemplate(NULL), m_calibrationPerformed(false),
    m_restartNeeded(false), m_connectionManager(NULL)
{
    setWindowTitle(tr("OpenPilot Setup Wizard"));
    setOption(QWizard::IndependentPages, false);
    for (quint16 i = 0; i < ActuatorSettings::CHANNELMAX_NUMELEM; i++) {
        m_actuatorSettings << actuatorChannelSettings();
    }
    setWizardStyle(QWizard::ModernStyle);
    setMinimumSize(780, 600);
    resize(780, 600);
    createPages();
}

SetupWizard::~SetupWizard()
{
    if (m_vehicleTemplate != NULL) {
        delete m_vehicleTemplate;
        m_vehicleTemplate = NULL;
    }
}

int SetupWizard::nextId() const
{
    switch (currentId()) {
    case PAGE_START:
        if (canAutoUpdate()) {
            return PAGE_UPDATE;
        } else {
            return PAGE_CONTROLLER;
        }
    case PAGE_UPDATE:
        return PAGE_CONTROLLER;

    case PAGE_CONTROLLER:
    {
        switch (getControllerType()) {
        case CONTROLLER_REVO:
        case CONTROLLER_NANO:
        case CONTROLLER_DISCOVERYF4:
            return PAGE_INPUT;

        case CONTROLLER_OPLINK:
        default:
            return PAGE_NOTYETIMPLEMENTED;
        }
    }
    case PAGE_VEHICLES:
    {
        switch (getVehicleType()) {
        case VEHICLE_MULTI:
            return PAGE_MULTI;

        case VEHICLE_FIXEDWING:
            return PAGE_FIXEDWING;

        case VEHICLE_HELI:
            return PAGE_HELI;

        case VEHICLE_SURFACE:
            return PAGE_SURFACE;

        default:
            return PAGE_NOTYETIMPLEMENTED;
        }
    }
    case PAGE_MULTI:
        return PAGE_ESC;

    case PAGE_FIXEDWING:
    case PAGE_SURFACE:
        if (getVehicleSubType() == GROUNDVEHICLE_DIFFERENTIAL) {
            return PAGE_ESC;
        } else {
            return PAGE_SERVO;
        }

    case PAGE_INPUT:
        if (isRestartNeeded()) {
            saveHardwareSettings();
            reboot();
        }
        return PAGE_VEHICLES;

    case PAGE_ESC:
        if (getVehicleSubType() == MULTI_ROTOR_TRI_Y) {
            return PAGE_SERVO;
        } else {
            switch (getControllerType()) {
            case CONTROLLER_REVO:
            case CONTROLLER_NANO:
                return PAGE_GPS;

            default:
                return PAGE_SUMMARY;
            }
        }

    case PAGE_SERVO:
    {
        switch (getControllerType()) {
        case CONTROLLER_REVO:
        case CONTROLLER_NANO:
            return PAGE_GPS;

        default:
            return PAGE_SUMMARY;
        }
    }

    case PAGE_BIAS_CALIBRATION:
        if (getVehicleType() == VEHICLE_MULTI) {
            return PAGE_ESC_CALIBRATION;
        } else {
            return PAGE_OUTPUT_CALIBRATION;
        }

    case PAGE_ESC_CALIBRATION:
        return PAGE_OUTPUT_CALIBRATION;

    case PAGE_OUTPUT_CALIBRATION:
        return PAGE_AIRFRAME_INITIAL_TUNING;

    case PAGE_AIRFRAME_INITIAL_TUNING:
        return PAGE_SAVE;

    case PAGE_GPS:
        switch (getVehicleType()) {
        case VEHICLE_FIXEDWING:
            if (getGpsType() != GPS_DISABLED) {
                return PAGE_AIRSPEED;
            } else {
                return PAGE_SUMMARY;
            }
        default:
            return PAGE_SUMMARY;
        }

    case PAGE_AIRSPEED:
        return PAGE_SUMMARY;

    case PAGE_SUMMARY:
    {
        switch (getControllerType()) {
        case CONTROLLER_REVO:
        case CONTROLLER_NANO:
        case CONTROLLER_DISCOVERYF4:
            switch (getVehicleType()) {
            case VEHICLE_FIXEDWING:
                return PAGE_OUTPUT_CALIBRATION;

            default:
                return PAGE_BIAS_CALIBRATION;
            }
        default:
            return PAGE_NOTYETIMPLEMENTED;
        }
    }
    case PAGE_SAVE:
        return PAGE_END;

    case PAGE_NOTYETIMPLEMENTED:
        return PAGE_END;

    default:
        return -1;
    }
}

QString SetupWizard::getSummaryText()
{
    QString summary = "";

    summary.append("<b>").append(tr("Controller type: ")).append("</b>");
    switch (getControllerType()) {
    case CONTROLLER_REVO:
        summary.append(tr("OpenPilot Revolution"));
        break;
    case CONTROLLER_NANO:
        summary.append(tr("OpenPilot Nano"));
        break;
    case CONTROLLER_OPLINK:
        summary.append(tr("OpenPilot OPLink Radio Modem"));
        break;
    case CONTROLLER_DISCOVERYF4:
        summary.append(tr("OpenPilot DiscoveryF4 Development Board"));
        break;
    default:
        summary.append(tr("Unknown"));
        break;
    }

    summary.append("<br>");
    summary.append("<b>").append(tr("Vehicle type: ")).append("</b>");
    switch (getVehicleType()) {
    case VEHICLE_MULTI:
        summary.append(tr("Multirotor"));

        summary.append("<br>");
        summary.append("<b>").append(tr("Vehicle sub type: ")).append("</b>");
        switch (getVehicleSubType()) {
        case SetupWizard::MULTI_ROTOR_TRI_Y:
            summary.append(tr("Tricopter"));
            break;
        case SetupWizard::MULTI_ROTOR_QUAD_X:
            summary.append(tr("Quadcopter X"));
            break;
        case SetupWizard::MULTI_ROTOR_QUAD_PLUS:
            summary.append(tr("Quadcopter +"));
            break;
        case SetupWizard::MULTI_ROTOR_HEXA:
            summary.append(tr("Hexacopter"));
            break;
        case SetupWizard::MULTI_ROTOR_HEXA_COAX_Y:
            summary.append(tr("Hexacopter Coax (Y6)"));
            break;
        case SetupWizard::MULTI_ROTOR_HEXA_H:
            summary.append(tr("Hexacopter H"));
            break;
        case SetupWizard::MULTI_ROTOR_HEXA_X:
            summary.append(tr("Hexacopter X"));
            break;
        case SetupWizard::MULTI_ROTOR_OCTO:
            summary.append(tr("Octocopter"));
            break;
        case SetupWizard::MULTI_ROTOR_OCTO_COAX_X:
            summary.append(tr("Octocopter Coax X"));
            break;
        case SetupWizard::MULTI_ROTOR_OCTO_COAX_PLUS:
            summary.append(tr("Octocopter Coax +"));
            break;
        case SetupWizard::MULTI_ROTOR_OCTO_V:
            summary.append(tr("Octocopter V"));
            break;
        default:
            summary.append(tr("Unknown"));
            break;
        }

        break;
    case VEHICLE_FIXEDWING:
        summary.append(tr("Fixed wing"));

        summary.append("<br>");
        summary.append("<b>").append(tr("Vehicle sub type: ")).append("</b>");
        switch (getVehicleSubType()) {
        case SetupWizard::FIXED_WING_DUAL_AILERON:
            summary.append(tr("Dual Aileron"));
            break;
        case SetupWizard::FIXED_WING_AILERON:
            summary.append(tr("Aileron"));
            break;
        case SetupWizard::FIXED_WING_ELEVON:
            summary.append(tr("Elevon"));
            break;
        default:
            summary.append(tr("Unknown"));
            break;
        }

        break;
    case VEHICLE_HELI:
        summary.append(tr("Helicopter"));
        break;
    case VEHICLE_SURFACE:
        summary.append(tr("Surface vehicle"));

        summary.append("<br>");
        summary.append("<b>").append(tr("Vehicle sub type: ")).append("</b>");
        switch (getVehicleSubType()) {
        case SetupWizard::GROUNDVEHICLE_CAR:
            summary.append(tr("Car"));
            break;
        case SetupWizard::GROUNDVEHICLE_DIFFERENTIAL:
            summary.append(tr("Tank"));
            break;
        case SetupWizard::GROUNDVEHICLE_MOTORCYCLE:
            summary.append(tr("Motorcycle"));
            break;
        default:
            summary.append(tr("Unknown"));
            break;
        }

        break;
    default:
        summary.append(tr("Unknown"));
    }

    summary.append("<br>");
    summary.append("<b>").append(tr("Input type: ")).append("</b>");
    switch (getInputType()) {
    case INPUT_PWM:
        summary.append(tr("PWM (One cable per channel)"));
        break;
    case INPUT_PPM:
        summary.append(tr("PPM (One cable for all channels)"));
        break;
    case INPUT_SBUS:
        summary.append(tr("Futaba S.Bus"));
        break;
    case INPUT_DSM:
        summary.append(tr("Spektrum Satellite"));
        break;
    default:
        summary.append(tr("Unknown"));
    }

    summary.append("<br>");
    summary.append("<b>").append(tr("Speed Controller (ESC) type: ")).append("</b>");
    switch (getEscType()) {
    case ESC_STANDARD:
        summary.append(tr("Standard ESC (%1 Hz)").arg(VehicleConfigurationHelper::LEGACY_ESC_FREQUENCY));
        break;
    case ESC_RAPID:
        summary.append(tr("Rapid ESC (%1 Hz)").arg(VehicleConfigurationHelper::RAPID_ESC_FREQUENCY));
        break;
    case ESC_SYNCHED:
        summary.append(tr("Synched ESC"));
        break;
    case ESC_ONESHOT:
        summary.append(tr("Oneshot ESC"));
        break;
    default:
        summary.append(tr("Unknown"));
    }

    // If Tricopter show tail servo speed
    if (getVehicleSubType() == MULTI_ROTOR_TRI_Y || getVehicleType() == VEHICLE_FIXEDWING
        || getVehicleSubType() == GROUNDVEHICLE_MOTORCYCLE || getVehicleSubType() == GROUNDVEHICLE_CAR) {
        summary.append("<br>");
        summary.append("<b>").append(tr("Servo type: ")).append("</b>");
        switch (getServoType()) {
        case SERVO_ANALOG:
            summary.append(tr("Analog Servos (50 Hz)"));
            break;
        case SERVO_DIGITAL:
            summary.append(tr("Digital Servos (333 Hz)"));
            break;
        default:
            summary.append(tr("Unknown"));
        }
    }

    // Show GPS Type
    if (getControllerType() == CONTROLLER_REVO || getControllerType() == CONTROLLER_NANO) {
        summary.append("<br>");
        summary.append("<b>").append(tr("GPS type: ")).append("</b>");
        switch (getGpsType()) {
        case GPS_PLATINUM:
            summary.append(tr("OpenPilot Platinum"));
            break;
        case GPS_UBX:
            summary.append(tr("OpenPilot v8 or Generic UBLOX GPS"));
            break;
        case GPS_NMEA:
            summary.append(tr("Generic NMEA GPS"));
            break;
        default:
            summary.append(tr("None"));
        }
    }

    // Show Airspeed sensor type
    if ((getControllerType() == CONTROLLER_REVO || getControllerType() == CONTROLLER_NANO) && getVehicleType() == VEHICLE_FIXEDWING) {
        summary.append("<br>");
        summary.append("<b>").append(tr("Airspeed Sensor: ")).append("</b>");
        switch (getAirspeedType()) {
        case AIRSPEED_ESTIMATE:
            summary.append(tr("Software Estimated"));
            break;
        case AIRSPEED_EAGLETREE:
            summary.append(tr("EagleTree on Flexi-Port"));
            break;
        case AIRSPEED_MS4525:
            summary.append(tr("MS4525 based on Flexi-Port"));
            break;
        default:
            summary.append(tr("Unknown"));
        }
    }
    return summary;
}

void SetupWizard::createPages()
{
    setPage(PAGE_START, new OPStartPage(this));
    setPage(PAGE_UPDATE, new AutoUpdatePage(this));
    setPage(PAGE_CONTROLLER, new ControllerPage(this));
    setPage(PAGE_VEHICLES, new VehiclePage(this));
    setPage(PAGE_MULTI, new MultiPage(this));
    setPage(PAGE_FIXEDWING, new FixedWingPage(this));
    setPage(PAGE_AIRSPEED, new AirSpeedPage(this));
    setPage(PAGE_GPS, new GpsPage(this));
    setPage(PAGE_HELI, new HeliPage(this));
    setPage(PAGE_SURFACE, new SurfacePage(this));
    setPage(PAGE_INPUT, new InputPage(this));
    setPage(PAGE_ESC, new EscPage(this));
    setPage(PAGE_SERVO, new ServoPage(this));
    setPage(PAGE_BIAS_CALIBRATION, new BiasCalibrationPage(this));
    setPage(PAGE_ESC_CALIBRATION, new EscCalibrationPage(this));
    setPage(PAGE_OUTPUT_CALIBRATION, new OutputCalibrationPage(this));
    setPage(PAGE_SUMMARY, new SummaryPage(this));
    setPage(PAGE_SAVE, new SavePage(this));
    setPage(PAGE_NOTYETIMPLEMENTED, new NotYetImplementedPage(this));
    setPage(PAGE_AIRFRAME_INITIAL_TUNING, new AirframeInitialTuningPage(this));
    setPage(PAGE_END, new OPEndPage(this));

    setStartId(PAGE_START);

    connect(button(QWizard::CustomButton1), SIGNAL(clicked()), this, SLOT(customBackClicked()));
    setButtonText(QWizard::CustomButton1, buttonText(QWizard::BackButton));
    QList<QWizard::WizardButton> button_layout;
    button_layout << QWizard::Stretch << QWizard::CustomButton1 << QWizard::NextButton << QWizard::CancelButton << QWizard::FinishButton;
    setButtonLayout(button_layout);
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));
}

void SetupWizard::customBackClicked()
{
    if (currentId() == PAGE_OUTPUT_CALIBRATION) {
        static_cast<OutputCalibrationPage *>(currentPage())->customBackClicked();
    } else {
        back();
        if (currentId() == PAGE_OUTPUT_CALIBRATION) {
            static_cast<OutputCalibrationPage *>(currentPage())->customBackClicked();
        }
    }
}

void SetupWizard::pageChanged(int currId)
{
    button(QWizard::CustomButton1)->setVisible(currId != PAGE_START);
    button(QWizard::CancelButton)->setVisible(currId != PAGE_END);
}

void SetupWizard::reboot() const
{
    SetupWizard *wiz = const_cast<SetupWizard *>(this);

    wiz->setWindowFlags(wiz->windowFlags() & ~Qt::WindowStaysOnTopHint);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    UploaderGadgetFactory *uploader    = pm->getObject<UploaderGadgetFactory>();
    Q_ASSERT(uploader);
    uploader->reboot();

    wiz->setRestartNeeded(false);
    wiz->setWindowFlags(wiz->windowFlags() | Qt::WindowStaysOnTopHint);
    wiz->show();
}

bool SetupWizard::saveHardwareSettings() const
{
    VehicleConfigurationHelper helper(const_cast<SetupWizard *>(this));

    return helper.setupHardwareSettings();
}

bool SetupWizard::canAutoUpdate() const
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();

    Q_ASSERT(pm);
    UploaderGadgetFactory *uploader    = pm->getObject<UploaderGadgetFactory>();
    Q_ASSERT(uploader);
    return uploader->isAutoUpdateCapable();
}
