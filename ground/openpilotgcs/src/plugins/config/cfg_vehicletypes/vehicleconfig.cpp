
/**
 ******************************************************************************
 *
 * @file       vehicleconfig.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief bit storage of config ui settings
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
#include "cfg_vehicletypes/vehicleconfig.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"

#include "systemsettings.h"

#include <QDebug>

VehicleConfig::VehicleConfig(QWidget *parent) : ConfigTaskWidget(parent)
{
    //Generate lists of mixerTypeNames, mixerVectorNames, channelNames
    channelNames << "None";
    for (int i = 0; i < (int)(VehicleConfig::CHANNEL_NUMELEM); i++) {
        mixerTypes << QString("Mixer%1Type").arg(i+1);
        mixerVectors << QString("Mixer%1Vector").arg(i+1);
        channelNames << QString("Channel%1").arg(i+1);
    }

//    typedef enum { MIXERTYPE_DISABLED=0, MIXERTYPE_MOTOR=1, MIXERTYPE_SERVO=2,
    //MIXERTYPE_CAMERAROLL=3, MIXERTYPE_CAMERAPITCH=4, MIXERTYPE_CAMERAYAW=5,
    //MIXERTYPE_ACCESSORY0=6, MIXERTYPE_ACCESSORY1=7, MIXERTYPE_ACCESSORY2=8,
    //MIXERTYPE_ACCESSORY3=9, MIXERTYPE_ACCESSORY4=10, MIXERTYPE_ACCESSORY5=11 } MixerTypeElem;

    mixerTypeDescriptions << "Disabled" << "Motor" << "Servo" << "CameraRoll" << "CameraPitch"
                          << "CameraYaw" << "Accessory0" << "Accessory1" << "Accessory2"
                          << "Accessory3" << "Accessory4" << "Accessory5";

    // This is needed because new style tries to compact things as much as possible in grid
    // and on OSX the widget sizes of PushButtons is reported incorrectly:
    // https://bugreports.qt-project.org/browse/QTBUG-14591
    foreach( QPushButton * btn, findChildren<QPushButton*>() ) {
        btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    }
}

VehicleConfig::~VehicleConfig()
{
   // Do nothing
}

GUIConfigDataUnion VehicleConfig::GetConfigData() {

    int i;
    GUIConfigDataUnion configData;

    // get an instance of systemsettings
    SystemSettings * systemSettings = SystemSettings::GetInstance(getUAVObjectManager());
    Q_ASSERT(systemSettings);
    SystemSettings::DataFields systemSettingsData = systemSettings->getData();

    // copy systemsettings -> local configData
    for(i = 0; i < (int)(SystemSettings::GUICONFIGDATA_NUMELEM); i++)
        configData.UAVObject[i]=systemSettingsData.GUIConfigData[i];

    // sanity check
    Q_ASSERT(SystemSettings::GUICONFIGDATA_NUMELEM ==
             (sizeof(configData.UAVObject) / sizeof(configData.UAVObject[0])));

    return configData;
}

void VehicleConfig::SetConfigData(GUIConfigDataUnion configData) {

    int i;

    // sanity check
    Q_ASSERT(SystemSettings::GUICONFIGDATA_NUMELEM ==
             (sizeof(configData.UAVObject) / sizeof(configData.UAVObject[0])));

    // get an instance of systemsettings
    SystemSettings * systemSettings = SystemSettings::GetInstance(getUAVObjectManager());
    Q_ASSERT(systemSettings);
    SystemSettings::DataFields systemSettingsData = systemSettings->getData();

    // copy parameter configData -> systemsettings
    for (i = 0; i < (int)(SystemSettings::GUICONFIGDATA_NUMELEM); i++)
        systemSettingsData.GUIConfigData[i] = configData.UAVObject[i];

    systemSettings->setData(systemSettingsData);
    systemSettings->updated();

    //emit ConfigurationChanged();
}

void VehicleConfig::ResetActuators(GUIConfigDataUnion* configData)
{
}
QStringList VehicleConfig::getChannelDescriptions()
{
    QStringList channelDesc;

    // init a channel_numelem list of channel desc defaults
    for (int i=0; i < (int)(VehicleConfig::CHANNEL_NUMELEM); i++)
    {
        channelDesc.append(QString("-"));
    }    
    return channelDesc;
}

/**
  Helper function:
  Sets the current index on supplied combobox to index
  if it is within bounds 0 <= index < combobox.count()
 */
void VehicleConfig::setComboCurrentIndex(QComboBox* box, int index)
{
    Q_ASSERT(box);

    if (index >= 0 && index < box->count())
        box->setCurrentIndex(index);
}

/**
  Helper function:
  enables/disables the named combobox within supplied uiowner
 */
void VehicleConfig::enableComboBox(QWidget* owner, QString boxName, bool enable)
{
    QComboBox* box = qFindChild<QComboBox*>(owner, boxName);
    if (box)
        box->setEnabled(enable);
}

QString VehicleConfig::getMixerType(UAVDataObject* mixer, int channel)
{
    Q_ASSERT(mixer);

    QString mixerType = mixerTypeDescriptions[0];  //default to disabled

    if (channel >= 0 && channel < mixerTypes.count()) {
        UAVObjectField *field = mixer->getField(mixerTypes.at(channel));
        Q_ASSERT(field);

        if (field)
        mixerType = field->getValue().toString();
    }

    return mixerType;
}

void VehicleConfig::setMixerType(UAVDataObject* mixer, int channel, MixerTypeElem mixerType)
{
    Q_ASSERT(mixer);

    qDebug() << QString("setMixerType channel %0, type %1").arg(channel).arg(mixerType);

    if (channel >= 0 && channel < mixerTypes.count()) {
        UAVObjectField *field = mixer->getField(mixerTypes.at(channel));
        Q_ASSERT(field);

        if (field) {
            if (mixerType >= 0 && mixerType < mixerTypeDescriptions.count())
            {
                field->setValue(mixerTypeDescriptions[mixerType]);
                mixer->updated();
            }
        }
    }
}

void VehicleConfig::resetMixerVector(UAVDataObject* mixer, int channel)
{
    Q_ASSERT(mixer);

    if (channel >= 0 && channel < mixerVectors.count()) {
        setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE1, 0);
        setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_THROTTLECURVE2, 0);
        setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_PITCH, 0);
        setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_ROLL, 0);
        setMixerVectorValue(mixer, channel, VehicleConfig::MIXERVECTOR_YAW, 0);
    }
}

double VehicleConfig::getMixerVectorValue(UAVDataObject* mixer, int channel, MixerVectorElem elementName)
{
    Q_ASSERT(mixer);

    double value = 0;

    if (channel >= 0 && channel < mixerVectors.count()) {
        UAVObjectField *field = mixer->getField(mixerVectors.at(channel));
        Q_ASSERT(field);

        if (field) {
            value = field->getDouble(elementName);
        }
    }
    return value;
}

void VehicleConfig::setMixerVectorValue(UAVDataObject* mixer, int channel, MixerVectorElem elementName, double value)
{
    Q_ASSERT(mixer);

    qDebug() << QString("setMixerVectorValue channel %0, name %1, value %2").arg(channel).arg(elementName).arg(value);

    if (channel >= 0 && channel < mixerVectors.count()) {
        UAVObjectField *field = mixer->getField(mixerVectors.at(channel));
        Q_ASSERT(field);

        if (field) {
            field->setDouble(value, elementName);
            mixer->updated();
        }
    }
}

/**
  Reset the contents of a field
  */
void VehicleConfig::resetField(UAVObjectField * field)
{
    for (unsigned int i=0;i<field->getNumElements();i++) {
        field->setValue(0,i);
    }
}


/**
 * Util function to get a pointer to the object manager
 * @return pointer to the UAVObjectManager
 */
UAVObjectManager* VehicleConfig::getUAVObjectManager() {
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr);
    return objMngr;
}
