#include "modeluavoproxy.h"
#include "extensionsystem/pluginmanager.h"

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
/*WPDESCRITPTION,LATPOSITION,LNGPOSITION,DISRELATIVE,BEARELATIVE,ISRELATIVE,ALTITUDE,
            VELOCITY,MODE,MODE_PARAMS0,MODE_PARAMS1,MODE_PARAMS2,MODE_PARAMS3,
            CONDITION,CONDITION_PARAMS0,CONDITION_PARAMS1,CONDITION_PARAMS2,CONDITION_PARAMS3,
            COMMAND,JUMPDESTINATION,ERRORDESTINATION,LOCKED
            */
void modelUavoProxy::modelToObjects()
{
    PathAction * act;
    act=new PathAction;
    Waypoint * wp;
    wp=new Waypoint;
    Q_ASSERT(act);
    Q_ASSERT(wp);
    Waypoint::DataFields waypoint = wp->getData();
    PathAction::DataFields action = act->getData();
    QModelIndex index;
    double distance;
    double bearing;
    double altitude;
    float velocity;
    int mode;
    int mode_param[4];
    int condition;
    int cond_param[4];
    int command;
    int jump;
    int error;
    for(int x=0;x<myModel->rowCount();++x)
    {
        index=myModel->index(x,flightDataModel::DISRELATIVE);
        distance=myModel->data(index).toDouble();
        index=myModel->index(x,flightDataModel::BEARELATIVE);
        bearing=myModel->data(index).toDouble();
        index=myModel->index(x,flightDataModel::VELOCITY);
        velocity=myModel->data(index).toFloat();


        index=myModel->index(x,flightDataModel::MODE);
        mode=myModel->data(index).toInt();
        index=myModel->index(x,flightDataModel::MODE_PARAMS0);
        mode_param[0]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::MODE_PARAMS1);
        mode_param[1]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::MODE_PARAMS2);
        mode_param[2]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::MODE_PARAMS3);
        mode_param[3]=myModel->data(index).toFloat();

        index=myModel->index(x,flightDataModel::CONDITION);
        condition=myModel->data(index).toInt();
        index=myModel->index(x,flightDataModel::CONDITION_PARAMS0);
        cond_param[0]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::CONDITION_PARAMS1);
        cond_param[1]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::CONDITION_PARAMS2);
        cond_param[2]=myModel->data(index).toFloat();
        index=myModel->index(x,flightDataModel::CONDITION_PARAMS3);
        cond_param[3]=myModel->data(index).toFloat();

        index=myModel->index(x,flightDataModel::COMMAND);
        command=myModel->data(index).toInt();
        index=myModel->index(x,flightDataModel::JUMPDESTINATION);
        jump=myModel->data(index).toInt();
        index=myModel->index(x,flightDataModel::ERRORDESTINATION);
        error=myModel->data(index).toInt();


    }
}

void modelUavoProxy::objectsToModel()
{
}
