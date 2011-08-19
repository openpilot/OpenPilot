/**
 ******************************************************************************
 *
 * @file       configtaskwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware (task widget)
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
#ifndef CONFIGTASKWIDGET_H
#define CONFIGTASKWIDGET_H


#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "uavobjectutilmanager.h"
#include <QQueue>
#include <QtGui/QWidget>
#include <QList>
#include <QLabel>
#include "smartsavebutton.h"
#include "mixercurvewidget.h"
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
class ConfigTaskWidget: public QWidget
{
    Q_OBJECT

public:
    struct objectToWidget
    {
        UAVObject * object;
        UAVObjectField * field;
        QWidget * widget;
    };

    ConfigTaskWidget(QWidget *parent = 0);
    ~ConfigTaskWidget();
    void saveObjectToSD(UAVObject *obj);
    UAVObjectManager* getObjectManager();
    static double listMean(QList<double> list);
    void addUAVObject(QString objectName);
    void addWidget(QWidget * widget);
    void addUAVObjectToWidgetRelation(QString object,QString field,QWidget * widget);
    void setupButtons(QPushButton * update,QPushButton * save);
    bool isDirty();
public slots:
    void onAutopilotDisconnect();
    void onAutopilotConnect();

private slots:
    virtual void refreshValues();
    virtual void updateObjectsFromWidgets();
private:
    QList <objectToWidget*> objOfInterest;
    ExtensionSystem::PluginManager *pm;
    UAVObjectManager *objManager;
    smartSaveButton *smartsave;
    bool dirty;
protected slots:
    virtual void disableObjUpdates();
    virtual void enableObjUpdates();
    virtual void clearDirty();
    virtual void widgetsContentsChanged();
    virtual void populateWidgets();
    virtual void refreshWidgetsValues();
protected:
    virtual void enableControls(bool enable);

};

#endif // CONFIGTASKWIDGET_H
