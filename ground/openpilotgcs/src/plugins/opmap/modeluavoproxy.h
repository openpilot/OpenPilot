#ifndef MODELUAVOPROXY_H
#define MODELUAVOPROXY_H

#include <QObject>
#include "flightdatamodel.h"
#include "pathaction.h"
#include "waypoint.h"

class modelUavoProxy:public QObject
{
    Q_OBJECT
public:
    explicit modelUavoProxy(QObject *parent, flightDataModel *model);
    int addAction(PathAction *actionObj, PathAction::DataFields actionFields,int lastaction);
public slots:
    void modelToObjects();
    void objectsToModel();
private:
    UAVObjectManager *objManager;
    Waypoint * waypointObj;
    PathAction * pathactionObj;
    flightDataModel * myModel;
};

#endif // MODELUAVOPROXY_H
