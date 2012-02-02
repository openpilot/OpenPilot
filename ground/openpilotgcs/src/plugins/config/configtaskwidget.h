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
#include <QCheckBox>
#include <QPushButton>

class ConfigTaskWidget: public QWidget
{
    Q_OBJECT

public:
    struct objectToWidget
    {
        UAVObject * object;
        UAVObjectField * field;
        QWidget * widget;
        int index;
        float scale;
        bool isLimited;
        QList<QWidget *> shadows;
    };

    struct shadows
    {
        QWidget * widget;
        float scale;
        bool isLimited;
        QWidget * parent;
    };

    enum buttonTypeEnum {none,save_button,apply_button,reload_button,default_button};
    struct uiRelationAutomation
    {
        QString objname;
        QString fieldname;
        QString element;
        int scale;
        bool ismaster;
        bool haslimits;
        buttonTypeEnum buttonType;
        QList<int> buttonGroup;
    };

    ConfigTaskWidget(QWidget *parent = 0);
    ~ConfigTaskWidget();
    void saveObjectToSD(UAVObject *obj);
    UAVObjectManager* getObjectManager();
    static double listMean(QList<double> list);
    void addUAVObject(QString objectName);
    void addWidget(QWidget * widget);
    void addUAVObjectToWidgetRelation(QString object,QString field,QWidget * widget,int index=0,float scale=1,bool isLimited=false,QList<int>* defaultReloadGroups=0);
    void addUAVObjectToWidgetRelation(QString object,QString field,QWidget * widget,QString element,float scale,bool isLimited=false,QList<int>* defaultReloadGroups=0);

    void addApplySaveButtons(QPushButton * update,QPushButton * save);


    void addReloadButton(QPushButton * button,int buttonGroup);
    void addDefaultButton(QPushButton * button,int buttonGroup);
    void addWidgetToDefaultReloadGroups(QWidget * widget, QList<int> *groups);
    bool addShadowWidget(QWidget * masterWidget, QWidget * shadowWidget,float shadowScale=1,bool shadowIsLimited=false);
    bool addShadowWidget(QString object,QString field,QWidget * widget,int index=0,float scale=1,bool isLimited=false);

    void autoLoadWidgets();

    bool isDirty();
    void setDirty(bool value);
    void addUAVObjectToWidgetRelation(QString object, QString field, QWidget *widget, QString index);
    bool allObjectsUpdated();

public slots:
    void onAutopilotDisconnect();
    void onAutopilotConnect();
    void invalidateObjects();
    void removeObject(UAVObject*);
    void removeAllObjects();
signals:
    void objectAdded(UAVObject*);
    void objectRemoved(UAVObject*);
private slots:
    virtual void refreshValues();
    virtual void updateObjectsFromWidgets();
    void objectUpdated(UAVObject*);
    void defaultButtonClicked();
    void reloadButtonClicked();
private:
    bool isConnected;
    QStringList objectsList;
    QList <objectToWidget*> objOfInterest;
    ExtensionSystem::PluginManager *pm;
    UAVObjectManager *objManager;
    smartSaveButton *smartsave;
    QMap<UAVObject *,bool> objectUpdates;
    QMap<int,QList<objectToWidget*> *> defaultReloadGroups;
    QList <shadows*> shadowsList;
    bool dirty;
    bool setFieldFromWidget(QWidget *widget, UAVObjectField *field, int index, float scale);
    bool setWidgetFromField(QWidget *widget, UAVObjectField *field, int index, float scale);
    void connectWidgetUpdatesToSlot(QWidget *widget, const char *function);
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
