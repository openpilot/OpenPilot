/**
 ******************************************************************************
 *
 * @file       GCSControlgadget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A gadget to control the UAV, either from the keyboard or a joystick
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
#include "gcscontrolgadget.h"
#include "gcscontrolgadgetwidget.h"

#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"
#include <QDebug>

GCSControlGadget::GCSControlGadget(QString classId, GCSControlGadgetWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
        m_widget(widget)
{
    connect(getManualControlCommand(),SIGNAL(objectUpdated(UAVObject*)),this,SLOT(manualControlCommandUpdated(UAVObject*)));
    connect(widget,SIGNAL(sticksChanged(double,double,double,double)),this,SLOT(sticksChangedLocally(double,double,double,double)));
    connect(this,SIGNAL(sticksChangedRemotely(double,double,double,double)),widget,SLOT(updateSticks(double,double,double,double)));

    manualControlCommandUpdated(getManualControlCommand());
}

GCSControlGadget::~GCSControlGadget()
{

}

void GCSControlGadget::loadConfiguration(IUAVGadgetConfiguration* config)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(QString("ManualControlCommand")) );
}

ManualControlCommand* GCSControlGadget::getManualControlCommand() {
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    return dynamic_cast<ManualControlCommand*>( objManager->getObject(QString("ManualControlCommand")) );
}

void GCSControlGadget::manualControlCommandUpdated(UAVObject * obj) {
    double roll = obj->getField("Roll")->getDouble();
    double pitch = obj->getField("Pitch")->getDouble();
    double yaw = obj->getField("Yaw")->getDouble();
    double throttle = obj->getField("Throttle")->getDouble();
    emit sticksChangedRemotely(yaw,-pitch,roll,throttle);
}

void GCSControlGadget::sticksChangedLocally(double leftX, double leftY, double rightX, double rightY) {
    ManualControlCommand * obj = getManualControlCommand();
    obj->getField("Roll")->setDouble(rightX);
    obj->getField("Pitch")->setDouble(-leftY);
    obj->getField("Yaw")->setDouble(leftX);
    obj->getField("Throttle")->setDouble(rightY);
    obj->updated();
}
