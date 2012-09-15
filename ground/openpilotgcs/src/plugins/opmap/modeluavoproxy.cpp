/**
 ******************************************************************************
 *
 * @file       modeluavproxy.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin OpenPilot Map Plugin
 * @{
 * @brief The OpenPilot Map plugin
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
#include "modeluavoproxy.h"
#include "extensionsystem/pluginmanager.h"
#include <math.h>
modelUavoProxy::modelUavoProxy(QObject *parent,flightDataModel * model):QObject(parent),myModel(model)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm != NULL);
    objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager != NULL);
    waypointObj = Waypoint::GetInstance(objManager);
    Q_ASSERT(waypointObj != NULL);
    pathactionObj=PathAction::GetInstance(objManager);
    Q_ASSERT(pathactionObj != NULL);
}
void modelUavoProxy::modelToObjects()
{
    PathAction * act=NULL;
    Waypoint * wp=NULL;
    QModelIndex index;
    double distance;
    double bearing;
    double altitude;
    int lastaction=-1;
    for(int x=0;x<myModel->rowCount();++x)
    {
        int instances=objManager->getNumInstances(waypointObj->getObjID());
        if(x>instances-1)
        {
            wp=new Waypoint;
            wp->initialize(x,wp->getMetaObject());
            objManager->registerObject(wp);
        }
        else
        {
            wp=Waypoint::GetInstance(objManager,x);
        }
        act=new PathAction;
        Q_ASSERT(act);
        Q_ASSERT(wp);
        Waypoint::DataFields waypoint = wp->getData();
        PathAction::DataFields action = act->getData();

        ///Waypoint object data
        index=myModel->index(x,flightDataModel::DISRELATIVE);
        distance=myModel->data(index).toDouble();
        index=myModel->index(x,flightDataModel::BEARELATIVE);
        bearing=myModel->data(index).toDouble();
        index=myModel->index(x,flightDataModel::ALTITUDERELATIVE);
        altitude=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::VELOCITY);
        waypoint.Velocity=myModel->data(index).toFloat();

        waypoint.Position[Waypoint::POSITION_NORTH]=distance*cos(bearing/180*M_PI);
        waypoint.Position[Waypoint::POSITION_EAST]=distance*sin(bearing/180*M_PI);
        waypoint.Position[Waypoint::POSITION_DOWN]=(-1.0f)*altitude;

        ///PathAction object data
        index=myModel->index(x,flightDataModel::MODE);
        action.Mode=myModel->data(index).toInt();
        index=myModel->index(x,flightDataModel::MODE_PARAMS0);
        action.ModeParameters[0]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::MODE_PARAMS1);
        action.ModeParameters[1]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::MODE_PARAMS2);
        action.ModeParameters[2]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::MODE_PARAMS3);
        action.ModeParameters[3]=myModel->data(index).toFloat();

        index=myModel->index(x,flightDataModel::CONDITION);
        action.EndCondition=myModel->data(index).toInt();
        index=myModel->index(x,flightDataModel::CONDITION_PARAMS0);
        action.ConditionParameters[0]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::CONDITION_PARAMS1);
        action.ConditionParameters[1]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::CONDITION_PARAMS2);
        action.ConditionParameters[2]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::CONDITION_PARAMS3);
        action.ConditionParameters[3]=myModel->data(index).toFloat();

        index=myModel->index(x,flightDataModel::COMMAND);
        action.Command=myModel->data(index).toInt();
        index=myModel->index(x,flightDataModel::JUMPDESTINATION);
        action.JumpDestination=myModel->data(index).toInt()-1;
        index=myModel->index(x,flightDataModel::ERRORDESTINATION);
        action.ErrorDestination=myModel->data(index).toInt()-1;

        int actionNumber=addAction(act,action,lastaction);
        if(actionNumber>lastaction)
            lastaction=actionNumber;
        waypoint.Action=actionNumber;
        wp->setData(waypoint);
        wp->updated();
    }
}

void modelUavoProxy::objectsToModel()
{
    Waypoint * wp;
    Waypoint::DataFields wpfields;
    PathAction * action;
    QModelIndex index;
    double distance;
    double bearing;

    PathAction::DataFields actionfields;

    myModel->removeRows(0,myModel->rowCount());
    for(int x=0;x<objManager->getNumInstances(waypointObj->getObjID());++x)
    {
        wp=Waypoint::GetInstance(objManager,x);
        Q_ASSERT(wp);
        if(!wp)
            continue;
        wpfields=wp->getData();
        myModel->insertRow(x);
        index=myModel->index(x,flightDataModel::VELOCITY);
        myModel->setData(index,wpfields.Velocity);
        distance=sqrt(wpfields.Position[Waypoint::POSITION_NORTH]*wpfields.Position[Waypoint::POSITION_NORTH]+
                      wpfields.Position[Waypoint::POSITION_EAST]*wpfields.Position[Waypoint::POSITION_EAST]);
        bearing=acos(wpfields.Position[Waypoint::POSITION_NORTH]/wpfields.Position[Waypoint::POSITION_EAST])*180/M_PI;
        if(bearing!=bearing)
            bearing=0;
        index=myModel->index(x,flightDataModel::DISRELATIVE);
        myModel->setData(index,distance);
        index=myModel->index(x,flightDataModel::BEARELATIVE);
        myModel->setData(index,bearing);
        index=myModel->index(x,flightDataModel::ALTITUDERELATIVE);
        myModel->setData(index,(-1.0f)*wpfields.Position[Waypoint::POSITION_DOWN]);

        action=PathAction::GetInstance(objManager,wpfields.Action);
        Q_ASSERT(action);
        if(!action)
            continue;
        actionfields=action->getData();

        index=myModel->index(x,flightDataModel::ISRELATIVE);
        myModel->setData(index,true);

        index=myModel->index(x,flightDataModel::COMMAND);
        myModel->setData(index,actionfields.Command);

        index=myModel->index(x,flightDataModel::CONDITION_PARAMS0);
        myModel->setData(index,actionfields.ConditionParameters[0]);
        index=myModel->index(x,flightDataModel::CONDITION_PARAMS1);
        myModel->setData(index,actionfields.ConditionParameters[1]);
        index=myModel->index(x,flightDataModel::CONDITION_PARAMS2);
        myModel->setData(index,actionfields.ConditionParameters[2]);
        index=myModel->index(x,flightDataModel::CONDITION_PARAMS3);
        myModel->setData(index,actionfields.ConditionParameters[3]);

        index=myModel->index(x,flightDataModel::CONDITION);
        myModel->setData(index,actionfields.EndCondition);

        index=myModel->index(x,flightDataModel::ERRORDESTINATION);
        myModel->setData(index,actionfields.ErrorDestination+1);

        index=myModel->index(x,flightDataModel::JUMPDESTINATION);
        myModel->setData(index,actionfields.JumpDestination+1);

        index=myModel->index(x,flightDataModel::MODE);
        myModel->setData(index,actionfields.Mode);

        index=myModel->index(x,flightDataModel::MODE_PARAMS0);
        myModel->setData(index,actionfields.ModeParameters[0]);
        index=myModel->index(x,flightDataModel::MODE_PARAMS1);
        myModel->setData(index,actionfields.ModeParameters[1]);
        index=myModel->index(x,flightDataModel::MODE_PARAMS2);
        myModel->setData(index,actionfields.ModeParameters[2]);
        index=myModel->index(x,flightDataModel::MODE_PARAMS3);
        myModel->setData(index,actionfields.ModeParameters[3]);
    }
}
int modelUavoProxy::addAction(PathAction * actionObj,PathAction::DataFields actionFields,int lastaction)
{
    //check if a similar action already exhists
    int instances=objManager->getNumInstances(pathactionObj->getObjID());
    for(int x=0;x<lastaction+1;++x)
    {
        PathAction * action=PathAction::GetInstance(objManager,x);
        Q_ASSERT(action);
        if(!action)
            continue;
        PathAction::DataFields fields=action->getData();
        if(fields.Command==actionFields.Command
                && fields.ConditionParameters[0]==actionFields.ConditionParameters[0]
                && fields.ConditionParameters[1]==actionFields.ConditionParameters[1]
                && fields.ConditionParameters[2]==actionFields.ConditionParameters[2]
                &&fields.EndCondition==actionFields.EndCondition
                &&fields.ErrorDestination==actionFields.ErrorDestination
                &&fields.JumpDestination==actionFields.JumpDestination
                &&fields.Mode==actionFields.Mode
                &&fields.ModeParameters[0]==actionFields.ModeParameters[0]
                &&fields.ModeParameters[1]==actionFields.ModeParameters[1]
                &&fields.ModeParameters[2]==actionFields.ModeParameters[2])
        {
            qDebug()<<"ModelUAVProxy:"<<"found similar action instance:"<<x;
            actionObj->deleteLater();
            return x;
        }
    }
    //if we get here it means no similar action was found, we have to create it
    if(instances<lastaction+2)
    {
        actionObj->initialize(instances,actionObj->getMetaObject());
        objManager->registerObject(actionObj);
        actionObj->setData(actionFields);
        actionObj->updated();
        qDebug()<<"ModelUAVProxy:"<<"created new action instance:"<<instances;
        return lastaction+1;
    }
    else
    {
        PathAction * action=PathAction::GetInstance(objManager,lastaction+1);
        Q_ASSERT(action);
        action->setData(actionFields);
        action->updated();
        actionObj->deleteLater();
        qDebug()<<"ModelUAVProxy:"<<"reused action instance:"<<lastaction+1;
        return lastaction+1;
    }
    return -1;//error we should never get here
}
