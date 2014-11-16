/**
 ******************************************************************************
 *
 * @file       vehicleconfigurationhelper.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup VehicleConfigurationHelper
 * @{
 * @brief
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

#ifndef VEHICLECONFIGURATIONHELPER_H
#define VEHICLECONFIGURATIONHELPER_H

#include <QList>
#include <QPair>
#include "vehicleconfigurationsource.h"
#include "uavobjectmanager.h"
#include "systemsettings.h"
#include "cfg_vehicletypes/vehicleconfig.h"
#include "actuatorsettings.h"

struct mixerChannelSettings {
    int type;
    int throttle1;
    int throttle2;
    int roll;
    int pitch;
    int yaw;

    mixerChannelSettings() : type(), throttle1(), throttle2(), roll(), pitch(), yaw() {}

    mixerChannelSettings(int t, int th1, int th2, int r, int p, int y)
        : type(t), throttle1(th1), throttle2(th2), roll(r), pitch(p), yaw(y) {}
};

class VehicleConfigurationHelper : public QObject {
    Q_OBJECT

public:
    VehicleConfigurationHelper(VehicleConfigurationSource *configSource);
    bool setupVehicle(bool save = true);
    bool setupHardwareSettings(bool save = true);
    static const qint16 LEGACY_ESC_FREQUENCY    = 50;
    static const qint16 RAPID_ESC_FREQUENCY     = 490;
    static const qint16 ANALOG_SERVO_FREQUENCY  = 50;
    static const qint16 DIGITAL_SERVO_FREQUENCY = 333;
    static const int MIXER_TYPE_NONE = 0;
    static const int MIXER_TYPE_MOTOR = 1;
    static const int MIXER_TYPE_REVERSABLEMOTOR = 2;
    static const int MIXER_TYPE_SERVO = 3;


signals:
    void saveProgress(int total, int current, QString description);

private:
    static const float DEFAULT_ENABLED_ACCEL_TAU = 0.1;

    VehicleConfigurationSource *m_configSource;
    UAVObjectManager *m_uavoManager;

    QList<QPair<UAVDataObject *, QString> * > m_modifiedObjects;
    void addModifiedObject(UAVDataObject *object, QString description);
    void clearModifiedObjects();

    void applyHardwareConfiguration();
    void applyVehicleConfiguration();
    void applyActuatorConfiguration();
    void applyFlightModeConfiguration();
    void applySensorBiasConfiguration();
    void applyStabilizationConfiguration();
    void applyManualControlDefaults();
    void applyTemplateSettings();

    bool isApplicable(UAVObject *dataObj);

    void applyMixerConfiguration(mixerChannelSettings channels[]);

    GUIConfigDataUnion getGUIConfigData();
    void applyMultiGUISettings(SystemSettings::AirframeTypeOptions airframe, GUIConfigDataUnion guiConfig);

    bool saveChangesToController(bool save);
    QEventLoop m_eventLoop;
    bool m_transactionOK;
    bool m_transactionTimeout;
    int m_currentTransactionObjectID;
    int m_progress;

    void resetVehicleConfig();
    void resetGUIData();

    void setupTriCopter();
    void setupQuadCopter();
    void setupHexaCopter();
    void setupOctoCopter();
    void setupElevon();
    void setupDualAileron();
    void setupAileron();
    void setupVtail();

private slots:
    void uAVOTransactionCompleted(UAVObject *object, bool success);
    void uAVOTransactionCompleted(int oid, bool success);
    void saveChangesTimeout();
};

#endif // VEHICLECONFIGURATIONHELPER_H
