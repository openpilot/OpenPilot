/**
 ******************************************************************************
 *
 * @file       smartsavebutton.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectWidgetUtils Plugin
 * @{
 * @brief Utility plugin for UAVObject to Widget relation management
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
#include <QDebug>
class smartSaveButton:public QObject
{
public:
    Q_OBJECT
public:
    smartSaveButton(QPushButton * update,QPushButton * save);
    void setObjects(QList<UAVDataObject *>);
    void addObject(UAVDataObject *);
    void clearObjects();
    void removeObject(UAVDataObject *obj);
    void removeAllObjects();
signals:
    void preProcessOperations();
    void saveSuccessfull();
    void beginOp();
    void endOp();
private slots:
    void processClick();
    void transaction_finished(UAVObject* obj, bool result);
    void saving_finished(int,bool);

private:
    QPushButton *bupdate;
    QPushButton *bsave;
    quint32 current_objectID;
    UAVDataObject * current_object;
    bool up_result;
    bool sv_result;
    QEventLoop loop;
    QList<UAVDataObject *> objects;
protected:
public slots:
    void enableControls(bool value);

};


#endif // SMARTSAVEBUTTON_H
