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
#include "gcscontrolgadgetconfiguration.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <QDebug>

#define JOYSTICK_UPDATE_RATE 50

GCSControlGadget::GCSControlGadget(QString classId, GCSControlGadgetWidget *widget, QWidget *parent, QObject *plugin) :
        IUAVGadget(classId, parent),
        m_widget(widget)
{
    connect(getManualControlCommand(),SIGNAL(objectUpdated(UAVObject*)),this,SLOT(manualControlCommandUpdated(UAVObject*)));
    connect(widget,SIGNAL(sticksChanged(double,double,double,double)),this,SLOT(sticksChangedLocally(double,double,double,double)));
    connect(this,SIGNAL(sticksChangedRemotely(double,double,double,double)),widget,SLOT(updateSticks(double,double,double,double)));

    manualControlCommandUpdated(getManualControlCommand());

    joystickTime.start();
    GCSControlPlugin *pl = dynamic_cast<GCSControlPlugin*>(plugin);
    connect(pl->sdlGamepad,SIGNAL(gamepads(quint8)),this,SLOT(gamepads(quint8)));
    connect(pl->sdlGamepad,SIGNAL(buttonState(ButtonNumber,bool)),this,SLOT(buttonState(ButtonNumber,bool)));
    connect(pl->sdlGamepad,SIGNAL(axesValues(QListInt16)),this,SLOT(axesValues(QListInt16)));

}

GCSControlGadget::~GCSControlGadget()
{
    delete m_widget;
}

void GCSControlGadget::loadConfiguration(IUAVGadgetConfiguration* config)
{
    GCSControlGadgetConfiguration *GCSControlConfig = qobject_cast< GCSControlGadgetConfiguration*>(config);

    QList<int> ql = GCSControlConfig->getChannelsMapping();
    rollChannel = ql.at(0);
    pitchChannel = ql.at(1);
    yawChannel = ql.at(2);
    throttleChannel = ql.at(3);

    controlsMode = GCSControlConfig->getControlsMode();

    int i;
    for (i=0;i<8;i++)
    {
        buttonSettings[i].ActionID=GCSControlConfig->getbuttonSettings(i).ActionID;
        buttonSettings[i].FunctionID=GCSControlConfig->getbuttonSettings(i).FunctionID;
        buttonSettings[i].Amount=GCSControlConfig->getbuttonSettings(i).Amount;
        buttonSettings[i].Amount=GCSControlConfig->getbuttonSettings(i).Amount;
        channelReverse[i]=GCSControlConfig->getChannelsReverse().at(i);
    }

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
    // Remap RPYT to left X/Y and right X/Y depending on mode
    switch (controlsMode) {
    case 1:
        // Mode 1: LeftX = Yaw, LeftY = Pitch, RightX = Roll, RightY = Throttle
        emit sticksChangedRemotely(yaw,-pitch,roll,throttle);
        break;
    case 2:
        // Mode 2: LeftX = Yaw, LeftY = Throttle, RightX = Roll, RightY = Pitch
        emit sticksChangedRemotely(yaw,throttle,roll,-pitch);
        break;
    case 3:
        // Mode 3: LeftX = Roll, LeftY = Pitch, RightX = Yaw, RightY = Throttle
        emit sticksChangedRemotely(roll,-pitch,yaw,throttle);
        break;
    case 4:
        // Mode 4: LeftX = Roll, LeftY = Throttle, RightX = Yaw, RightY = Pitch;
        emit sticksChangedRemotely(roll,throttle,yaw,-pitch);
        break;
    }
}

/**
  Update the manual commands - maps depending on mode
  */
void GCSControlGadget::sticksChangedLocally(double leftX, double leftY, double rightX, double rightY) {
    ManualControlCommand * obj = getManualControlCommand();
    double oldRoll = obj->getField("Roll")->getDouble();
    double oldPitch = obj->getField("Pitch")->getDouble();
    double oldYaw = obj->getField("Yaw")->getDouble();
    double oldThrottle = obj->getField("Throttle")->getDouble();

    double newRoll;
    double newPitch;
    double newYaw;
    double newThrottle;

    // Remap left X/Y and right X/Y to RPYT depending on mode
    switch (controlsMode) {
    case 1:
        // Mode 1: LeftX = Yaw, LeftY = Pitch, RightX = Roll, RightY = Throttle
        newRoll = rightX;
        newPitch = -leftY;
        newYaw = leftX;
        newThrottle = rightY;
        break;
    case 2:
        // Mode 2: LeftX = Yaw, LeftY = Throttle, RightX = Roll, RightY = Pitch
        newRoll = rightX;
        newPitch = -rightY;
        newYaw = leftX;
        newThrottle = leftY;
        break;
    case 3:
        // Mode 3: LeftX = Roll, LeftY = Pitch, RightX = Yaw, RightY = Throttle
        newRoll = leftX;
        newPitch = -leftY;
        newYaw = rightX;
        newThrottle = rightY;
        break;
    case 4:
        // Mode 4: LeftX = Roll, LeftY = Throttle, RightX = Yaw, RightY = Pitch;
        newRoll = leftX;
        newPitch = -rightY;
        newYaw = rightX;
        newThrottle = leftY;
        break;
    }

    //check if buttons have control over this axis... if so don't update it
    int buttonRollControl=0;
    int buttonPitchControl=0;
    int buttonYawControl=0;
    int buttonThrottleControl=0;
    for (int i=0;i<8;i++)
    {
        if ((buttonSettings[i].FunctionID==1)&&((buttonSettings[i].ActionID==1)||(buttonSettings[i].ActionID==2)))buttonRollControl=1;
        if ((buttonSettings[i].FunctionID==2)&&((buttonSettings[i].ActionID==1)||(buttonSettings[i].ActionID==2)))buttonPitchControl=1;
        if ((buttonSettings[i].FunctionID==3)&&((buttonSettings[i].ActionID==1)||(buttonSettings[i].ActionID==2)))buttonYawControl=1;
        if ((buttonSettings[i].FunctionID==4)&&((buttonSettings[i].ActionID==1)||(buttonSettings[i].ActionID==2)))buttonThrottleControl=1;
    }

    //if we are not in local gcs control mode, ignore the joystick input
    if (((GCSControlGadgetWidget *)m_widget)->getGCSControl()==false)return;

    if((newThrottle != oldThrottle) || (newPitch != oldPitch) || (newYaw != oldYaw) || (newRoll != oldRoll)) {
        if (buttonRollControl==0)obj->getField("Roll")->setDouble(newRoll);
        if (buttonPitchControl==0)obj->getField("Pitch")->setDouble(newPitch);
        if (buttonYawControl==0)obj->getField("Yaw")->setDouble(newYaw);
        if (buttonThrottleControl==0)obj->getField("Throttle")->setDouble(newThrottle);
        obj->updated();
    }
}

void GCSControlGadget::gamepads(quint8 count)
{
//    sdlGamepad.setGamepad(0);
//    sdlGamepad.setTickRate(JOYSTICK_UPDATE_RATE);
}

void GCSControlGadget::buttonState(ButtonNumber number, bool pressed)
{
    int state;
    if ((buttonSettings[number].ActionID>0)&&(buttonSettings[number].FunctionID>0)&&(pressed))
    {//this button is configured
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(QString("ManualControlCommand")) );
        bool currentCGSControl = ((GCSControlGadgetWidget *)m_widget)->getGCSControl();

        switch (buttonSettings[number].ActionID)
        {
        case 1://increase
            if (currentCGSControl)
            {
                switch (buttonSettings[number].FunctionID)
                {
                case 1://Roll
                        obj->getField("Roll")->setValue(bound(obj->getField("Roll")->getValue().toDouble()+buttonSettings[number].Amount));
                    break;
                case 2://Pitch
                        obj->getField("Pitch")->setValue(bound(obj->getField("Pitch")->getValue().toDouble()+buttonSettings[number].Amount));
                     break;
                case 3://Yaw
                        obj->getField("Yaw")->setValue(wrap(obj->getField("Yaw")->getValue().toDouble()+buttonSettings[number].Amount));
                    break;
                case 4://Throttle
                        obj->getField("Throttle")->setValue(bound(obj->getField("Throttle")->getValue().toDouble()+buttonSettings[number].Amount));
                    break;
                }
            }
            break;
        case 2://decrease
            if (currentCGSControl)
            {
                switch (buttonSettings[number].FunctionID)
                {
                case 1://Roll
                        obj->getField("Roll")->setValue(bound(obj->getField("Roll")->getValue().toDouble()-buttonSettings[number].Amount));
                    break;
                case 2://Pitch
                        obj->getField("Pitch")->setValue(bound(obj->getField("Pitch")->getValue().toDouble()-buttonSettings[number].Amount));
                     break;
                case 3://Yaw
                        obj->getField("Yaw")->setValue(wrap(obj->getField("Yaw")->getValue().toDouble()-buttonSettings[number].Amount));
                    break;
                case 4://Throttle
                        obj->getField("Throttle")->setValue(bound(obj->getField("Throttle")->getValue().toDouble()-buttonSettings[number].Amount));
                    break;
                }
            }
            break;
        case 3://toggle
                switch (buttonSettings[number].FunctionID)
                {
                case 1://Armed
                    if (currentCGSControl)
                    {
                        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
                        UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
                        UAVDataObject* obj = dynamic_cast<UAVDataObject*>( objManager->getObject(QString("FlightStatus")) );

                        if(obj->getField("Armed")->getValue().toString().compare("Armed")==0)
                        {
                            obj->getField("Armed")->setValue("Disarmed");
                        }
                        else
                        {
                            obj->getField("Armed")->setValue("Armed");
                        }
                    }
                break;
            case 2://GCS Control
                //Toggle the GCS Control checkbox, its built in signalling will handle the update to OP
                ((GCSControlGadgetWidget *)m_widget)->setGCSControl(!currentCGSControl);

               break;
            }

            break;
        }

        obj->updated();
    }
        //buttonSettings[number].ActionID NIDT
        //buttonSettings[number].FunctionID -RPYTAC
        //buttonSettings[number].Amount
}

void GCSControlGadget::axesValues(QListInt16 values)
{
    int chMax = values.length();
    if (rollChannel > chMax || pitchChannel > chMax ||
            yawChannel > chMax || throttleChannel > chMax ) {
        qDebug() << "GCSControl: configuration is inconsistent with current joystick! Aborting update.";
        return;
    }

    double rValue = (rollChannel > -1) ? values[rollChannel] : 0;
    double pValue = (pitchChannel > -1) ? values[pitchChannel] : 0;
    double yValue = (yawChannel > -1) ? values[yawChannel] : 0;
    double tValue = (throttleChannel > -1) ? values[throttleChannel] : 0;
    double max = 32767;

    if (rollChannel > -1) if(channelReverse[rollChannel]==true)rValue = -rValue;
    if (pitchChannel > -1) if(channelReverse[pitchChannel]==true)pValue = -pValue;
    if (yawChannel > -1) if(channelReverse[yawChannel]==true)yValue = -yValue;
    if (throttleChannel > -1) if(channelReverse[throttleChannel]==true)tValue = -tValue;


     if(joystickTime.elapsed() > JOYSTICK_UPDATE_RATE) {
        joystickTime.restart();
        // Remap RPYT to left X/Y and right X/Y depending on mode
        // Mode 1: LeftX = Yaw, LeftY = Pitch, RightX = Roll, RightY = Throttle
        // Mode 2: LeftX = Yaw, LeftY = THrottle, RightX = Roll, RightY = Pitch
        // Mode 3: LeftX = Roll, LeftY = Pitch, RightX = Yaw, RightY = Throttle
        // Mode 4: LeftX = Roll, LeftY = Throttle, RightX = Yaw, RightY = Pitch;
        switch (controlsMode) {
        case 1:
            sticksChangedLocally(yValue/max,-pValue/max,rValue/max,-tValue/max);
            break;
        case 2:
            sticksChangedLocally(yValue/max,-tValue/max,rValue/max,-pValue/max);
            break;
        case 3:
            sticksChangedLocally(rValue/max,-pValue/max,yValue/max,-tValue/max);
            break;
        case 4:
            sticksChangedLocally(rValue/max,-tValue/max,yValue/max,-pValue/max);
            break;
        }
    }
}


double GCSControlGadget::bound(double input)
{
    if (input > 1.0)return 1.0;
    if (input <-1.0)return -1.0;
    return input;
}

double GCSControlGadget::wrap(double input)
{
    while (input > 1.0)input -= 2.0;
    while (input <-1.0)input += 2.0;
    return input;
}
