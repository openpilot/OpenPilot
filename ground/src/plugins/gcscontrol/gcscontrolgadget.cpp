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

    connect(this, SIGNAL(aboutToQuit()), &sdlGamepad, SLOT(quit()));
    if(sdlGamepad.init()) {
        qDebug() << "SDL Initialized";
        sdlGamepad.start();
        qRegisterMetaType<QListInt16>("QListInt16");
        qRegisterMetaType<ButtonNumber>("ButtonNumber");

        connect(&sdlGamepad,SIGNAL(gamepads(quint8)),this,SLOT(gamepads(quint8)));
        connect(&sdlGamepad,SIGNAL(buttonState(ButtonNumber,bool)),this,SLOT(buttonState(ButtonNumber,bool)));
        connect(&sdlGamepad,SIGNAL(axesValues(QListInt16)),this,SLOT(axesValues(QListInt16)));
    }
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
    double oldRoll = obj->getField("Roll")->getDouble();
    double oldPitch = obj->getField("Pitch")->getDouble();
    double oldYaw = obj->getField("Yaw")->getDouble();
    double oldThrottle = obj->getField("Throttle")->getDouble();

    double newRoll = rightX;
    double newPitch = -leftY;
    double newYaw = leftX;
    double newThrottle = rightY;

    if((newThrottle != oldThrottle) || (newPitch != oldPitch) || (newYaw != oldYaw) || (newRoll != oldRoll)) {
        obj->getField("Roll")->setDouble(newRoll);
        obj->getField("Pitch")->setDouble(newPitch);
        obj->getField("Yaw")->setDouble(newYaw);
        obj->getField("Throttle")->setDouble(newThrottle);
        obj->updated();
    }
}

void GCSControlGadget::gamepads(quint8 count)
{
    sdlGamepad.setGamepad(0);
    sdlGamepad.setTickRate(40);
}

void GCSControlGadget::buttonState(ButtonNumber number, bool pressed)
{
}

void GCSControlGadget::axesValues(QListInt16 values)
{
    double leftX = values[0];
    double leftY = values[1];
    double rightX = values[2];
    double rightY = values[3];
    double max = 32767;

    sticksChangedLocally(leftX/max,-leftY/max,rightX/max,-rightY/max);
}
