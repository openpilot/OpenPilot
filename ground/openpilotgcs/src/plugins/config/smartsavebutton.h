#ifndef SMARTSAVEBUTTON_H
#define SMARTSAVEBUTTON_H

#include "uavtalk/telemetrymanager.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <QPushButton>
#include <QList>
#include <QEventLoop>
#include "uavobjectutilmanager.h"
#include <QObject>
class smartSaveButton:public QObject
{
public:
    Q_OBJECT
public:
    smartSaveButton(QPushButton * update,QPushButton * save);
    void setObjects(QList<UAVObject *>);
    void addObject(UAVObject *);
    void clearObjects();
signals:
    void preProcessOperations();
private slots:
    void processClick();
    void transaction_finished(UAVObject* obj, bool result);
    void saving_finished(int,bool);

private:
    QPushButton *bupdate;
    QPushButton *bsave;
    quint32 current_objectID;
    UAVObject * current_object;
    bool up_result;
    bool sv_result;
    QEventLoop loop;
    QList<UAVObject *> objects;
protected:
public slots:
    void enableControls(bool value);

};


#endif // SMARTSAVEBUTTON_H
