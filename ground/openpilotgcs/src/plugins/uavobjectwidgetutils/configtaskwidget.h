/**
 ******************************************************************************
 *
 * @file       configtaskwidget.h
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
#include "uavobjectwidgetutils_global.h"
#include <QDesktopServices>
#include <QUrl>
#include <QEvent>

class UAVOBJECTWIDGETUTILS_EXPORT ConfigTaskWidget: public QWidget
{
    Q_OBJECT

public:
    struct shadow
    {
        QWidget * widget;
        double scale;
        bool isLimited;
    };
    struct objectToWidget
    {
        UAVObject * object;
        UAVObjectField * field;
        QWidget * widget;
        int index;
        double scale;
        bool isLimited;
        QList<shadow *> shadowsList;
    };

    struct temphelper
    {
        quint32 objid;
        quint32 objinstid;
        bool operator==(const temphelper& lhs)
        {
            return (lhs.objid==this->objid && lhs.objinstid==this->objinstid);
        }
    };

    enum buttonTypeEnum {none,save_button,apply_button,reload_button,default_button,help_button};
    struct uiRelationAutomation
    {
        QString objname;
        QString fieldname;
        QString element;
        QString url;
        double scale;
        bool haslimits;
        buttonTypeEnum buttonType;
        QList<int> buttonGroup;
    };

    ConfigTaskWidget(QWidget *parent = 0);
    virtual ~ConfigTaskWidget();

    void disableMouseWheelEvents();
    bool eventFilter( QObject * obj, QEvent * evt );

    void saveObjectToSD(UAVObject *obj);
    UAVObjectManager* getObjectManager();
    static double listMean(QList<double> list);

    void addUAVObject(QString objectName, QList<int> *reloadGroups=NULL);
    void addUAVObject(UAVObject * objectName, QList<int> *reloadGroups=NULL);

    void addWidget(QWidget * widget);

    void addUAVObjectToWidgetRelation(QString object,QString field,QWidget * widget,int index=0,double scale=1,bool isLimited=false,QList<int>* defaultReloadGroups=0,quint32 instID=0);
    void addUAVObjectToWidgetRelation(UAVObject *obj, UAVObjectField * field, QWidget *widget, int index=0, double scale=1, bool isLimited=false, QList<int> *defaultReloadGroups=0, quint32 instID=0);

    void addUAVObjectToWidgetRelation(QString object,QString field,QWidget * widget,QString element,double scale,bool isLimited=false,QList<int>* defaultReloadGroups=0,quint32 instID=0);
    void addUAVObjectToWidgetRelation(UAVObject *obj, UAVObjectField * field,QWidget * widget,QString element,double scale,bool isLimited=false,QList<int>* defaultReloadGroups=0,quint32 instID=0);

    void addUAVObjectToWidgetRelation(QString object, QString field, QWidget *widget, QString index);
    void addUAVObjectToWidgetRelation(UAVObject *obj, UAVObjectField * field, QWidget *widget, QString index);

    //BUTTONS//
    void addApplySaveButtons(QPushButton * update,QPushButton * save);
    void addReloadButton(QPushButton * button,int buttonGroup);
    void addDefaultButton(QPushButton * button,int buttonGroup);
    //////////

    void addWidgetToDefaultReloadGroups(QWidget * widget, QList<int> *groups);

    bool addShadowWidget(QWidget * masterWidget, QWidget * shadowWidget,double shadowScale=1,bool shadowIsLimited=false);
    bool addShadowWidget(QString object,QString field,QWidget * widget,int index=0,double scale=1,bool isLimited=false, QList<int> *defaultReloadGroups=NULL, quint32 instID=0);

    void autoLoadWidgets();

    bool isDirty();
    void setDirty(bool value);

    bool allObjectsUpdated();
    void setOutOfLimitsStyle(QString style){outOfLimitsStyle=style;}
    void addHelpButton(QPushButton * button,QString url);
    void forceShadowUpdates();
    void forceConnectedState();
public slots:
    void onAutopilotDisconnect();
    void onAutopilotConnect();
    void invalidateObjects();
    void apply();
    void save();
signals:
    //fired when a widgets contents changes
    void widgetContentsChanged(QWidget * widget);
    //fired when the framework requests that the widgets values be populated, use for custom behaviour
    void populateWidgetsRequested();
    //fired when the framework requests that the widgets values be refreshed, use for custom behaviour
    void refreshWidgetsValuesRequested();
    //fired when the framework requests that the UAVObject values be updated from the widgets value, use for custom behaviour
    void updateObjectsFromWidgetsRequested();
    //fired when the autopilot connects
    void autoPilotConnected();
    //fired when the autopilot disconnects
    void autoPilotDisconnected();
    void defaultRequested(int group);
private slots:
    void objectUpdated(UAVObject*);
    void defaultButtonClicked();
    void reloadButtonClicked();
private:
    int currentBoard;
    bool isConnected;
    bool allowWidgetUpdates;
    QStringList objectsList;
    QList <objectToWidget*> objOfInterest;
    ExtensionSystem::PluginManager *pm;
    UAVObjectManager *objManager;
    UAVObjectUtilManager* utilMngr;
    smartSaveButton *smartsave;
    QMap<UAVObject *,bool> objectUpdates;
    QMap<int,QList<objectToWidget*> *> defaultReloadGroups;
    QMap<QWidget *,objectToWidget*> shadowsList;
    QMap<QPushButton *,QString> helpButtonList;
    QList<QPushButton *> reloadButtonList;
    bool dirty;
    bool setFieldFromWidget(QWidget *widget, UAVObjectField *field, int index, double scale);
    bool setWidgetFromField(QWidget *widget, UAVObjectField *field, int index, double scale, bool hasLimits);
    QVariant getVariantFromWidget(QWidget *widget, double scale);
    bool setWidgetFromVariant(QWidget *widget,QVariant value,double scale);
    void connectWidgetUpdatesToSlot(QWidget *widget, const char *function);
    void disconnectWidgetUpdatesToSlot(QWidget *widget, const char *function);
    void loadWidgetLimits(QWidget *widget, UAVObjectField *field, int index, bool hasLimits, double sclale);
    QString outOfLimitsStyle;
    QTimer * timeOut;
protected slots:
    virtual void disableObjUpdates();
    virtual void enableObjUpdates();
    virtual void clearDirty();
    virtual void widgetsContentsChanged();
    virtual void populateWidgets();
    virtual void refreshWidgetsValues(UAVObject * obj=NULL);
    virtual void updateObjectsFromWidgets();
    virtual void helpButtonPressed();
protected:
    virtual void enableControls(bool enable);
    void checkWidgetsLimits(QWidget *widget, UAVObjectField *field, int index, bool hasLimits, QVariant value, double scale);
};

#endif // CONFIGTASKWIDGET_H
