/**
 ******************************************************************************
 *
 * @file       configtaskwidget.cpp
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
#include "configtaskwidget.h"
#include <QtGui/QWidget>
#include <QtGui/QLineEdit>
#include "uavsettingsimportexport/uavsettingsimportexportfactory.h"

/**
 * Constructor
 */
ConfigTaskWidget::ConfigTaskWidget(QWidget *parent) : QWidget(parent),isConnected(false),allowWidgetUpdates(true),smartsave(NULL),dirty(false),outOfLimitsStyle("background-color: rgb(255, 0, 0);"),timeOut(NULL)
{
    pm = ExtensionSystem::PluginManager::instance();
    objManager = pm->getObject<UAVObjectManager>();
    TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
    utilMngr = pm->getObject<UAVObjectUtilManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()),Qt::UniqueConnection);
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()),Qt::UniqueConnection);
    connect(telMngr, SIGNAL(connected()), this, SIGNAL(autoPilotConnected()),Qt::UniqueConnection);
    connect(telMngr, SIGNAL(disconnected()), this, SIGNAL(autoPilotDisconnected()),Qt::UniqueConnection);
    UAVSettingsImportExportFactory * importexportplugin =  pm->getObject<UAVSettingsImportExportFactory>();
    connect(importexportplugin,SIGNAL(importAboutToBegin()),this,SLOT(invalidateObjects()));
}

/**
 * Add a widget to the dirty detection pool
 * @param widget to add to the detection pool
 */
void ConfigTaskWidget::addWidget(QWidget * widget)
{
    addUAVObjectToWidgetRelation("","",widget);
}
/**
 * Add an object to the management system
 * @param objectName name of the object to add to the management system
 */
void ConfigTaskWidget::addUAVObject(QString objectName,QList<int> * reloadGroups)
{
    addUAVObjectToWidgetRelation(objectName,"",NULL,0,1,false,reloadGroups);
}

void ConfigTaskWidget::addUAVObject(UAVObject *objectName, QList<int> *reloadGroups)
{
    QString objstr;
    if(objectName)
        objstr=objectName->getName();
    addUAVObject(objstr, reloadGroups);

}
/**
 * Add an UAVObject field to widget relation to the management system
 * @param object name of the object to add
 * @param field name of the field to add
 * @param widget pointer to the widget whitch will display/define the field value
 * @param index index of the field element to add to this relation
 */
void ConfigTaskWidget::addUAVObjectToWidgetRelation(QString object, QString field, QWidget * widget, QString index)
{
    UAVObject *obj=NULL;
    UAVObjectField *_field=NULL;
    obj = objManager->getObject(QString(object));
    Q_ASSERT(obj);
    _field = obj->getField(QString(field));
    Q_ASSERT(_field);
    addUAVObjectToWidgetRelation(object,field,widget,_field->getElementNames().indexOf(index));
}

void ConfigTaskWidget::addUAVObjectToWidgetRelation(UAVObject *obj, UAVObjectField * field, QWidget *widget, QString index)
{
    QString objstr;
    QString fieldstr;
    if(obj)
        objstr=obj->getName();
    if(field)
        fieldstr=field->getName();
    addUAVObjectToWidgetRelation(objstr, fieldstr, widget, index);
}
/**
 * Add a UAVObject field to widget relation to the management system
 * @param object name of the object to add
 * @param field name of the field to add
 * @param widget pointer to the widget whitch will display/define the field value
 * @param element name of the element of the field element to add to this relation
 * @param scale scale value of this relation
 * @param isLimited bool to signal if this widget contents is limited in value
 * @param defaultReloadGroups default and reload groups this relation belongs to
 * @param instID instance ID of the object used on this relation
 */
void ConfigTaskWidget::addUAVObjectToWidgetRelation(QString object, QString field, QWidget *widget, QString element, double scale, bool isLimited, QList<int> *defaultReloadGroups, quint32 instID)
{
    UAVObject *obj=objManager->getObject(QString(object),instID);
    Q_ASSERT(obj);
    UAVObjectField *_field;
    int index=0;
    if(!field.isEmpty() && obj)
    {
        _field = obj->getField(QString(field));
        if(!element.isEmpty())
            index=_field->getElementNames().indexOf(QString(element));
    }
    addUAVObjectToWidgetRelation(object, field, widget,index,scale,isLimited,defaultReloadGroups,instID);
}

void ConfigTaskWidget::addUAVObjectToWidgetRelation(UAVObject *obj, UAVObjectField *field, QWidget *widget, QString element, double scale, bool isLimited, QList<int> *defaultReloadGroups, quint32 instID)
{
    QString objstr;
    QString fieldstr;
    if(obj)
        objstr=obj->getName();
    if(field)
        fieldstr=field->getName();
    addUAVObjectToWidgetRelation(objstr, fieldstr, widget, element, scale, isLimited, defaultReloadGroups, instID);
}
void ConfigTaskWidget::addUAVObjectToWidgetRelation(UAVObject * obj,UAVObjectField * field, QWidget * widget, int index,double scale,bool isLimited,QList<int>* defaultReloadGroups, quint32 instID)
{
    QString objstr;
    QString fieldstr;
    if(obj)
        objstr=obj->getName();
    if(field)
        fieldstr=field->getName();
    addUAVObjectToWidgetRelation(objstr,fieldstr,widget,index,scale,isLimited,defaultReloadGroups,instID);
}

/**
 * Add an UAVObject field to widget relation to the management system
 * @param object name of the object to add
 * @param field name of the field to add
 * @param widget pointer to the widget whitch will display/define the field value
 * @param index index of the element of the field to add to this relation
 * @param scale scale value of this relation
 * @param isLimited bool to signal if this widget contents is limited in value
 * @param defaultReloadGroups default and reload groups this relation belongs to
 * @param instID instance ID of the object used on this relation
 */
void ConfigTaskWidget::addUAVObjectToWidgetRelation(QString object, QString field, QWidget * widget, int index,double scale,bool isLimited,QList<int>* defaultReloadGroups, quint32 instID)
{
    if(addShadowWidget(object,field,widget,index,scale,isLimited,defaultReloadGroups,instID))
        return;

    UAVObject *obj=NULL;
    UAVObjectField *_field=NULL;
    if(!object.isEmpty())
    {
        obj = objManager->getObject(QString(object),instID);
        Q_ASSERT(obj);
        objectUpdates.insert(obj,true);
        connect(obj, SIGNAL(objectUpdated(UAVObject*)),this, SLOT(objectUpdated(UAVObject*)));
        connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshWidgetsValues(UAVObject*)), Qt::UniqueConnection);
    }
    if(!field.isEmpty() && obj)
        _field = obj->getField(QString(field));
    objectToWidget * ow=new objectToWidget();
    ow->field=_field;
    ow->object=obj;
    ow->widget=widget;
    ow->index=index;
    ow->scale=scale;
    ow->isLimited=isLimited;
    objOfInterest.append(ow);
    if(obj)
    {
        if(smartsave)
        {
            smartsave->addObject((UAVDataObject*)obj);
        }
    }
    if(widget==NULL)
    {
        if(defaultReloadGroups && obj)
        {
            foreach(int i,*defaultReloadGroups)
            {
                if(this->defaultReloadGroups.contains(i))
                {
                    this->defaultReloadGroups.value(i)->append(ow);
                }
                else
                {
                    this->defaultReloadGroups.insert(i,new QList<objectToWidget*>());
                    this->defaultReloadGroups.value(i)->append(ow);
                }
            }
        }
    }
    else
    {
        connectWidgetUpdatesToSlot(widget,SLOT(widgetsContentsChanged()));
        if(defaultReloadGroups)
            addWidgetToDefaultReloadGroups(widget,defaultReloadGroups);
        shadowsList.insert(widget,ow);
        loadWidgetLimits(widget,_field,index,isLimited,scale);
    }
}
/**
 * destructor
 */
ConfigTaskWidget::~ConfigTaskWidget()
{
    if(smartsave)
        delete smartsave;
    foreach(QList<objectToWidget*>* pointer,defaultReloadGroups.values())
    {
        if(pointer)
            delete pointer;
    }
    foreach (objectToWidget* oTw, objOfInterest)
    {
        if(oTw)
            delete oTw;
    }
}

void ConfigTaskWidget::saveObjectToSD(UAVObject *obj)
{
    // saveObjectToSD is now handled by the UAVUtils plugin in one
    // central place (and one central queue)
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager* utilMngr = pm->getObject<UAVObjectUtilManager>();
    utilMngr->saveObjectToSD(obj);
}

/**
 * Util function to get a pointer to the object manager
 * @return pointer to the UAVObjectManager
 */
UAVObjectManager* ConfigTaskWidget::getObjectManager() {
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr);
    return objMngr;
}
/**
 * Utility function which calculates the Mean value of a list of values
 * @param list list of double values
 * @returns Mean value of the list of parameter values
 */
double ConfigTaskWidget::listMean(QList<double> list)
{
    double accum = 0;
    for(int i = 0; i < list.size(); i++)
        accum += list[i];
    return accum / list.size();
}

// ************************************
// telemetry start/stop connect/disconnect signals

void ConfigTaskWidget::onAutopilotDisconnect()
{
    isConnected=false;
    enableControls(false);
    invalidateObjects();
}

void ConfigTaskWidget::forceConnectedState()//dynamic widgets don't recieve the connected signal. This should be called instead.
{
    isConnected=true;
    setDirty(false);
}

void ConfigTaskWidget::onAutopilotConnect()
{
    if (utilMngr)
        currentBoard = utilMngr->getBoardModel();//TODO REMEMBER TO ADD THIS TO FORCE CONNECTED FUNC ON CC3D_RELEASE
    invalidateObjects();
    isConnected=true;
    foreach(objectToWidget * ow,objOfInterest)
    {
        loadWidgetLimits(ow->widget,ow->field,ow->index,ow->isLimited,ow->scale);
    }
    setDirty(false);
    enableControls(true);
    refreshWidgetsValues();
}
/**
 * SLOT Function used to populate the widgets with the initial values
 * Overwrite this if you need to change the default behavior
 */
void ConfigTaskWidget::populateWidgets()
{
    bool dirtyBack=dirty;
    emit populateWidgetsRequested();
    foreach(objectToWidget * ow,objOfInterest)
    {
        if(ow->object==NULL || ow->field==NULL || ow->widget==NULL)
        {
            // do nothing
        }
        else
            setWidgetFromField(ow->widget,ow->field,ow->index,ow->scale,ow->isLimited);
    }
    setDirty(dirtyBack);
}
/**
 * SLOT function used to refresh the widgets contents of the widgets with relation to
 * object field added to the framework pool
 * Overwrite this if you need to change the default behavior
 */
void ConfigTaskWidget::refreshWidgetsValues(UAVObject * obj)
{
    if (!allowWidgetUpdates)
        return;

    bool dirtyBack=dirty;
    emit refreshWidgetsValuesRequested();
    foreach(objectToWidget * ow,objOfInterest)
    {
        if(ow->object==NULL || ow->field==NULL || ow->widget==NULL)
        {
            //do nothing
        }
        else
        {
            if(ow->object==obj || obj==NULL)
                setWidgetFromField(ow->widget,ow->field,ow->index,ow->scale,ow->isLimited);
        }

    }
    setDirty(dirtyBack);

}
/**
 * SLOT function used to update the uavobject fields from widgets with relation to
 * object field added to the framework pool
 * Overwrite this if you need to change the default behavior
 */
void ConfigTaskWidget::updateObjectsFromWidgets()
{
    emit updateObjectsFromWidgetsRequested();
    foreach(objectToWidget * ow,objOfInterest)
    {
        if(ow->object==NULL || ow->field==NULL)
        {

        }
        else
            setFieldFromWidget(ow->widget,ow->field,ow->index,ow->scale);

    }
}
/**
 * SLOT function used handle help button presses
 * Overwrite this if you need to change the default behavior
 */
void ConfigTaskWidget::helpButtonPressed()
{
    QString url=helpButtonList.value((QPushButton*)sender(),QString());
    if(!url.isEmpty())
        QDesktopServices::openUrl( QUrl(url, QUrl::StrictMode) );
}
/**
 * Add update and save buttons to the form
 * multiple buttons can be added for the same function
 * @param update pointer to the update button
 * @param save pointer to the save button
 */
void ConfigTaskWidget::addApplySaveButtons(QPushButton *update, QPushButton *save)
{
    if(!smartsave)
    {
        smartsave=new smartSaveButton();
        connect(smartsave,SIGNAL(preProcessOperations()), this, SLOT(updateObjectsFromWidgets()));
        connect(smartsave,SIGNAL(saveSuccessfull()),this,SLOT(clearDirty()));
        connect(smartsave,SIGNAL(beginOp()),this,SLOT(disableObjUpdates()));
        connect(smartsave,SIGNAL(endOp()),this,SLOT(enableObjUpdates()));
    }
    if(update && save)
        smartsave->addButtons(save,update);
    else if (update)
        smartsave->addApplyButton(update);
    else if (save)
        smartsave->addSaveButton(save);
    if(objOfInterest.count()>0)
    {
        foreach(objectToWidget * oTw,objOfInterest)
        {
            smartsave->addObject((UAVDataObject*)oTw->object);
        }
    }
    TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
    if(telMngr->isConnected())
        enableControls(true);
    else
        enableControls(false);
}
/**
 * SLOT function used the enable or disable the SAVE, UPLOAD and RELOAD buttons
 * @param enable set to true to enable the buttons or false to disable them
 * @param field name of the field to add
 */
void ConfigTaskWidget::enableControls(bool enable)
{
    if(smartsave)
        smartsave->enableControls(enable);
    foreach (QPushButton * button, reloadButtonList) {
        button->setEnabled(enable);
    }
}
/**
 * SLOT function called when on of the widgets contents added to the framework changes
 */
void ConfigTaskWidget::forceShadowUpdates()
{
    foreach(objectToWidget * oTw,objOfInterest)
    {
        foreach (shadow * sh, oTw->shadowsList)
        {
            disconnectWidgetUpdatesToSlot((QWidget*)sh->widget,SLOT(widgetsContentsChanged()));
            checkWidgetsLimits(sh->widget,oTw->field,oTw->index,sh->isLimited,getVariantFromWidget(oTw->widget,oTw->scale),sh->scale);
            setWidgetFromVariant(sh->widget,getVariantFromWidget(oTw->widget,oTw->scale),sh->scale);
            emit widgetContentsChanged((QWidget*)sh->widget);
            connectWidgetUpdatesToSlot((QWidget*)sh->widget,SLOT(widgetsContentsChanged()));

        }
    }
    setDirty(true);
}
/**
 * SLOT function called when one of the widgets contents added to the framework changes
 */
void ConfigTaskWidget::widgetsContentsChanged()
{
    emit widgetContentsChanged((QWidget*)sender());
    double scale;
    objectToWidget * oTw= shadowsList.value((QWidget*)sender(),NULL);
    if(oTw)
    {
        if(oTw->widget==(QWidget*)sender())
        {
            scale=oTw->scale;
            checkWidgetsLimits((QWidget*)sender(),oTw->field,oTw->index,oTw->isLimited,getVariantFromWidget((QWidget*)sender(),oTw->scale),oTw->scale);
        }
        else
        {
            foreach (shadow * sh, oTw->shadowsList)
            {
                if(sh->widget==(QWidget*)sender())
                {
                    scale=sh->scale;
                    checkWidgetsLimits((QWidget*)sender(),oTw->field,oTw->index,sh->isLimited,getVariantFromWidget((QWidget*)sender(),scale),scale);
                }
            }
        }
        if(oTw->widget!=(QWidget *)sender())
        {
            disconnectWidgetUpdatesToSlot((QWidget*)oTw->widget,SLOT(widgetsContentsChanged()));
            checkWidgetsLimits(oTw->widget,oTw->field,oTw->index,oTw->isLimited,getVariantFromWidget((QWidget*)sender(),scale),oTw->scale);
            setWidgetFromVariant(oTw->widget,getVariantFromWidget((QWidget*)sender(),scale),oTw->scale);
            emit widgetContentsChanged((QWidget*)oTw->widget);
            connectWidgetUpdatesToSlot((QWidget*)oTw->widget,SLOT(widgetsContentsChanged()));
        }
        foreach (shadow * sh, oTw->shadowsList)
        {
            if(sh->widget!=(QWidget*)sender())
            {
                disconnectWidgetUpdatesToSlot((QWidget*)sh->widget,SLOT(widgetsContentsChanged()));
                checkWidgetsLimits(sh->widget,oTw->field,oTw->index,sh->isLimited,getVariantFromWidget((QWidget*)sender(),scale),sh->scale);
                setWidgetFromVariant(sh->widget,getVariantFromWidget((QWidget*)sender(),scale),sh->scale);
                emit widgetContentsChanged((QWidget*)sh->widget);
                connectWidgetUpdatesToSlot((QWidget*)sh->widget,SLOT(widgetsContentsChanged()));
            }
        }
    }
    if(smartsave)
        smartsave->resetIcons();
    setDirty(true);
}
/**
 * SLOT function used clear the forms dirty status flag
 */
void ConfigTaskWidget::clearDirty()
{
    setDirty(false);
}
/**
 * Sets the form's dirty status flag
 * @param value
 */
void ConfigTaskWidget::setDirty(bool value)
{
    dirty=value;
}
/**
 * Checks if the form is dirty (unsaved changes)
 * @return true if the form has unsaved changes
 */
bool ConfigTaskWidget::isDirty()
{
    if(isConnected)
        return dirty;
    else
        return false;
}
/**
 * SLOT function used to disable widget contents changes when related object field changes
 */
void ConfigTaskWidget::disableObjUpdates()
{
    allowWidgetUpdates = false;
    foreach(objectToWidget * obj,objOfInterest)
    {
        if(obj->object)disconnect(obj->object, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshWidgetsValues(UAVObject*)));
    }
}
/**
 * SLOT function used to enable widget contents changes when related object field changes
 */
void ConfigTaskWidget::enableObjUpdates()
{
    allowWidgetUpdates = true;
    foreach(objectToWidget * obj,objOfInterest)
    {
        if(obj->object)
            connect(obj->object, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshWidgetsValues(UAVObject*)), Qt::UniqueConnection);
    }
}
/**
 * Called when an uav object is updated
 * @param obj pointer to the object whitch has just been updated
 */
void ConfigTaskWidget::objectUpdated(UAVObject *obj)
{
    objectUpdates[obj]=true;
}
/**
 * Checks if all objects added to the pool have already been updated
 * @return true if all objects added to the pool have already been updated
 */
bool ConfigTaskWidget::allObjectsUpdated()
{
    qDebug()<<"ConfigTaskWidge:allObjectsUpdated called";
    bool ret=true;
    foreach(UAVObject *obj, objectUpdates.keys())
    {
        ret=ret & objectUpdates[obj];
    }
    qDebug()<<"Returned:"<<ret;
    return ret;
}
/**
 * Adds a new help button
 * @param button pointer to the help button
 * @param url url to open in the browser when the help button is pressed
 */
void ConfigTaskWidget::addHelpButton(QPushButton *button, QString url)
{
    helpButtonList.insert(button,url);
    connect(button,SIGNAL(clicked()),this,SLOT(helpButtonPressed()));

}
/**
 * Invalidates all the uav objects "is updated" flag
 */
void ConfigTaskWidget::invalidateObjects()
{
    foreach(UAVObject *obj, objectUpdates.keys())
    {
        objectUpdates[obj]=false;
    }
}
/**
 * SLOT call this to apply changes to uav objects
 */
void ConfigTaskWidget::apply()
{
    if(smartsave)
        smartsave->apply();
}
/**
 * SLOT call this to save changes to uav objects
 */
void ConfigTaskWidget::save()
{
    if(smartsave)
        smartsave->save();
}
/**
 * Adds a new shadow widget
 * shadow widgets are widgets whitch have a relation to an object already present on the framework pool i.e. already added trough addUAVObjectToWidgetRelation
 * This function doesn't have to be used directly, addUAVObjectToWidgetRelation will call it if a previous relation exhists.
 * @return returns false if the shadow widget relation failed to be added (no previous relation exhisted)
 */
bool ConfigTaskWidget::addShadowWidget(QString object, QString field, QWidget *widget, int index, double scale, bool isLimited,QList<int>* defaultReloadGroups,quint32 instID)
{
    foreach(objectToWidget * oTw,objOfInterest)
    {
        if(!oTw->object || !oTw->widget || !oTw->field)
            continue;
        if(oTw->object->getName()==object && oTw->field->getName()==field && oTw->index==index && oTw->object->getInstID()==instID)
        {
            shadow * sh=NULL;
            //prefer anything else to QLabel
            if(qobject_cast<QLabel *>(oTw->widget) && !qobject_cast<QLabel *>(widget))
            {
                sh=new shadow;
                sh->isLimited=oTw->isLimited;
                sh->scale=oTw->scale;
                sh->widget=oTw->widget;
                oTw->isLimited=isLimited;
                oTw->scale=scale;
                oTw->widget=widget;
            }
            //prefer QDoubleSpinBox to anything else
            else if(!qobject_cast<QDoubleSpinBox *>(oTw->widget) && qobject_cast<QDoubleSpinBox *>(widget))
            {
                sh=new shadow;
                sh->isLimited=oTw->isLimited;
                sh->scale=oTw->scale;
                sh->widget=oTw->widget;
                oTw->isLimited=isLimited;
                oTw->scale=scale;
                oTw->widget=widget;
            }
            else
            {
                sh=new shadow;
                sh->isLimited=isLimited;
                sh->scale=scale;
                sh->widget=widget;
            }
            shadowsList.insert(widget,oTw);
            oTw->shadowsList.append(sh);
            connectWidgetUpdatesToSlot(widget,SLOT(widgetsContentsChanged()));
            if(defaultReloadGroups)
                addWidgetToDefaultReloadGroups(widget,defaultReloadGroups);
            loadWidgetLimits(widget,oTw->field,oTw->index,isLimited,scale);
            return true;
        }
    }
    return false;
}
/**
 * Auto loads widgets based on the Dynamic property named "objrelation"
 * Check the wiki for more information
 */
void ConfigTaskWidget::autoLoadWidgets()
{
    QPushButton * saveButtonWidget=NULL;
    QPushButton * applyButtonWidget=NULL;
    foreach(QWidget * widget,this->findChildren<QWidget*>())
    {
        QVariant info=widget->property("objrelation");
        if(info.isValid())
        {
            uiRelationAutomation uiRelation;
            uiRelation.buttonType=none;
            uiRelation.scale=1;
            uiRelation.element=QString();
            uiRelation.haslimits=false;
            foreach(QString str, info.toStringList())
            {
                QString prop=str.split(":").at(0);
                QString value=str.split(":").at(1);
                if(prop== "objname")
                    uiRelation.objname=value;
                else if(prop== "fieldname")
                    uiRelation.fieldname=value;
                else if(prop=="element")
                    uiRelation.element=value;
                else if(prop== "scale")
                {
                    if(value=="null")
                        uiRelation.scale=1;
                    else
                        uiRelation.scale=value.toDouble();
                }
                else if(prop== "haslimits")
                {
                    if(value=="yes")
                        uiRelation.haslimits=true;
                    else
                        uiRelation.haslimits=false;
                }
                else if(prop== "button")
                {
                    if(value=="save")
                        uiRelation.buttonType=save_button;
                    else if(value=="apply")
                        uiRelation.buttonType=apply_button;
                    else if(value=="reload")
                        uiRelation.buttonType=reload_button;
                    else if(value=="default")
                        uiRelation.buttonType=default_button;
                    else if(value=="help")
                        uiRelation.buttonType=help_button;
                }
                else if(prop== "buttongroup")
                {
                    foreach(QString s,value.split(","))
                    {
                        uiRelation.buttonGroup.append(s.toInt());
                    }
                }
                else if(prop=="url")
                    uiRelation.url=str.mid(str.indexOf(":")+1);
            }
            if(!uiRelation.buttonType==none)
            {
                QPushButton * button=NULL;
                switch(uiRelation.buttonType)
                {
                case save_button:
                    saveButtonWidget=qobject_cast<QPushButton *>(widget);
                    if(saveButtonWidget)
                        addApplySaveButtons(NULL,saveButtonWidget);
                    break;
                case apply_button:
                    applyButtonWidget=qobject_cast<QPushButton *>(widget);
                    if(applyButtonWidget)
                        addApplySaveButtons(applyButtonWidget,NULL);
                    break;
                case default_button:
                    button=qobject_cast<QPushButton *>(widget);
                    if(button)
                        addDefaultButton(button,uiRelation.buttonGroup.at(0));
                    break;
                case reload_button:
                    button=qobject_cast<QPushButton *>(widget);
                    if(button)
                        addReloadButton(button,uiRelation.buttonGroup.at(0));
                    break;
                case help_button:
                    button=qobject_cast<QPushButton *>(widget);
                    if(button)
                        addHelpButton(button,uiRelation.url);
                    break;

                default:
                    break;
                }
            }
            else
            {
                QWidget * wid=qobject_cast<QWidget *>(widget);
                if(wid)
                    addUAVObjectToWidgetRelation(uiRelation.objname,uiRelation.fieldname,wid,uiRelation.element,uiRelation.scale,uiRelation.haslimits,&uiRelation.buttonGroup);
            }
        }
    }
    refreshWidgetsValues();
    forceShadowUpdates();
    foreach(objectToWidget * ow,objOfInterest)
    {
        if(ow->widget)
            qDebug()<<"Master:"<<ow->widget->objectName();
        foreach(shadow * sh,ow->shadowsList)
        {
            if(sh->widget)
                qDebug()<<"Child"<<sh->widget->objectName();
        }
    }
}
/**
 * Adds a widget to a list of default/reload groups
 * default/reload groups are groups of widgets to be set with default or reloaded (values from persistent memory) when a defined button is pressed
 * @param widget pointer to the widget to be added to the groups
 * @param groups list of the groups on which to add the widget
 */
void ConfigTaskWidget::addWidgetToDefaultReloadGroups(QWidget *widget, QList<int> * groups)
{
    foreach(objectToWidget * oTw,objOfInterest)
    {
        bool addOTW=false;
        if(oTw->widget==widget)
            addOTW=true;
        else
        {
            foreach(shadow * sh, oTw->shadowsList)
            {
                if(sh->widget==widget)
                    addOTW=true;
            }
        }
        if(addOTW)
        {
            foreach(int i,*groups)
            {
                if(defaultReloadGroups.contains(i))
                {
                    defaultReloadGroups.value(i)->append(oTw);
                }
                else
                {
                    defaultReloadGroups.insert(i,new QList<objectToWidget*>());
                    defaultReloadGroups.value(i)->append(oTw);
                }
            }
        }
    }
}
/**
 * Adds a button to a default group
 * @param button pointer to the default button
 * @param buttongroup number of the group
 */
void ConfigTaskWidget::addDefaultButton(QPushButton *button, int buttonGroup)
{
    button->setProperty("group",buttonGroup);
    connect(button,SIGNAL(clicked()),this,SLOT(defaultButtonClicked()));
}
/**
 * Adds a button to a reload group
 * @param button pointer to the reload button
 * @param buttongroup number of the group
 */
void ConfigTaskWidget::addReloadButton(QPushButton *button, int buttonGroup)
{
    button->setProperty("group",buttonGroup);
    reloadButtonList.append(button);
    connect(button,SIGNAL(clicked()),this,SLOT(reloadButtonClicked()));
}
/**
 * Called when a default button is clicked
 */
void ConfigTaskWidget::defaultButtonClicked()
{
    int group=sender()->property("group").toInt();
    emit defaultRequested(group);
    QList<objectToWidget*> * list=defaultReloadGroups.value(group);
    foreach(objectToWidget * oTw,*list)
    {
        if(!oTw->object || !oTw->field)
            continue;
        UAVDataObject * temp=((UAVDataObject*)oTw->object)->dirtyClone();
        setWidgetFromField(oTw->widget,temp->getField(oTw->field->getName()),oTw->index,oTw->scale,oTw->isLimited);
    }
}
/**
 * Called when a reload button is clicked
 */
void ConfigTaskWidget::reloadButtonClicked()
{
    if(timeOut)
        return;
    int group=sender()->property("group").toInt();
    QList<objectToWidget*> * list=defaultReloadGroups.value(group,NULL);
    if(!list)
        return;
    ObjectPersistence* objper = dynamic_cast<ObjectPersistence*>( getObjectManager()->getObject(ObjectPersistence::NAME) );
    timeOut=new QTimer(this);
    QEventLoop * eventLoop=new QEventLoop(this);
    connect(timeOut, SIGNAL(timeout()),eventLoop,SLOT(quit()));
    connect(objper, SIGNAL(objectUpdated(UAVObject*)), eventLoop, SLOT(quit()));

    QList<temphelper> temp;
    foreach(objectToWidget * oTw,*list)
    {
        if (oTw->object != NULL)
        {
            temphelper value;
            value.objid=oTw->object->getObjID();
            value.objinstid=oTw->object->getInstID();
            if(temp.contains(value))
                continue;
            else
                temp.append(value);
            ObjectPersistence::DataFields data;
            data.Operation = ObjectPersistence::OPERATION_LOAD;
            data.Selection = ObjectPersistence::SELECTION_SINGLEOBJECT;
            data.ObjectID = oTw->object->getObjID();
            data.InstanceID = oTw->object->getInstID();
            objper->setData(data);
            objper->updated();
            timeOut->start(500);
            eventLoop->exec();
            if(timeOut->isActive())
            {
                oTw->object->requestUpdate();
                if(oTw->widget)
                    setWidgetFromField(oTw->widget,oTw->field,oTw->index,oTw->scale,oTw->isLimited);
            }
            timeOut->stop();
        }
    }
    if(eventLoop)
    {
        delete eventLoop;
        eventLoop=NULL;
    }
    if(timeOut)
    {
        delete timeOut;
        timeOut=NULL;
    }
}

/**
 * Connects widgets "contents changed" signals to a slot
 */
void ConfigTaskWidget::connectWidgetUpdatesToSlot(QWidget * widget,const char* function)
{
    if(!widget)
        return;
    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        connect(cb,SIGNAL(currentIndexChanged(int)),this,function);
    }
    else if(QSlider * cb=qobject_cast<QSlider *>(widget))
    {
        connect(cb,SIGNAL(valueChanged(int)),this,function);
    }
    else if(MixerCurveWidget * cb=qobject_cast<MixerCurveWidget *>(widget))
    {
        connect(cb,SIGNAL(curveUpdated()),this,function);
    }
    else if(QTableWidget * cb=qobject_cast<QTableWidget *>(widget))
    {
        connect(cb,SIGNAL(cellChanged(int,int)),this,function);
    }
    else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
    {
        connect(cb,SIGNAL(valueChanged(int)),this,function);
    }
    else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
    {
        connect(cb,SIGNAL(valueChanged(double)),this,function);
    }
    else if(QCheckBox * cb=qobject_cast<QCheckBox *>(widget))
    {
        connect(cb,SIGNAL(stateChanged(int)),this,function);
    }
    else if(QPushButton * cb=qobject_cast<QPushButton *>(widget))
    {
        connect(cb,SIGNAL(clicked()),this,function);
    }
    else
        qDebug()<<__FUNCTION__<<"widget to uavobject relation not implemented"<<widget->metaObject()->className();

}
/**
 * Disconnects widgets "contents changed" signals to a slot
 */
void ConfigTaskWidget::disconnectWidgetUpdatesToSlot(QWidget * widget,const char* function)
{
    if(!widget)
        return;
    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        disconnect(cb,SIGNAL(currentIndexChanged(int)),this,function);
    }
    else if(QSlider * cb=qobject_cast<QSlider *>(widget))
    {
        disconnect(cb,SIGNAL(valueChanged(int)),this,function);
    }
    else if(MixerCurveWidget * cb=qobject_cast<MixerCurveWidget *>(widget))
    {
        disconnect(cb,SIGNAL(curveUpdated()),this,function);
    }
    else if(QTableWidget * cb=qobject_cast<QTableWidget *>(widget))
    {
        disconnect(cb,SIGNAL(cellChanged(int,int)),this,function);
    }
    else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
    {
        disconnect(cb,SIGNAL(valueChanged(int)),this,function);
    }
    else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
    {
        disconnect(cb,SIGNAL(valueChanged(double)),this,function);
    }
    else if(QCheckBox * cb=qobject_cast<QCheckBox *>(widget))
    {
        disconnect(cb,SIGNAL(stateChanged(int)),this,function);
    }
    else if(QPushButton * cb=qobject_cast<QPushButton *>(widget))
    {
        disconnect(cb,SIGNAL(clicked()),this,function);
    }
    else
        qDebug()<<__FUNCTION__<<"widget to uavobject relation not implemented"<<widget->metaObject()->className();

}
/**
 * Sets a widget value from an UAVObject field
 * @param widget pointer for the widget to set
 * @param field pointer to the UAVObject field to use
 * @param index index of the element to use
 * @param scale scale to be used on the assignement
 * @return returns true if the assignement was successfull
 */
bool ConfigTaskWidget::setFieldFromWidget(QWidget * widget,UAVObjectField * field,int index,double scale)
{
    if(!widget || !field)
        return false;
    QVariant ret=getVariantFromWidget(widget,scale);
    if(ret.isValid())
    {
        field->setValue(ret,index);
        return true;
    }
    {
        qDebug()<<__FUNCTION__<<"widget to uavobject relation not implemented"<<widget->metaObject()->className();
        return false;
    }
}
/**
 * Gets a variant from a widget
 * @param widget pointer to the widget from where to get the value
 * @param scale scale to be used on the assignement
 * @return returns the value of the widget times the scale
 */
QVariant ConfigTaskWidget::getVariantFromWidget(QWidget * widget,double scale)
{
    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        return (QString)cb->currentText();
    }
    else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
    {
        return (double)(cb->value()* scale);
    }
    else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
    {
        return (double)(cb->value()* scale);
    }
    else if(QSlider * cb=qobject_cast<QSlider *>(widget))
    {
        return(double)(cb->value()* scale);
    }
    else if(QCheckBox * cb=qobject_cast<QCheckBox *>(widget))
    {
        return (QString)(cb->isChecked()?"TRUE":"FALSE");
    }
    else if(QLineEdit * cb=qobject_cast<QLineEdit *>(widget))
    {
        return (QString)cb->displayText();
    }
    else
        return QVariant();
}
/**
 * Sets a widget from a variant
 * @param widget pointer for the widget to set
 * @param scale scale to be used on the assignement
 * @param value value to be used on the assignement
 * @return returns true if the assignement was successfull
 */
bool ConfigTaskWidget::setWidgetFromVariant(QWidget *widget, QVariant value, double scale)
{
    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        cb->setCurrentIndex(cb->findText(value.toString()));
        return true;
    }
    else if(QLabel * cb=qobject_cast<QLabel *>(widget))
    {
        if(scale==0)
            cb->setText(value.toString());
        else
            cb->setText(QString::number((value.toDouble()/scale)));
        return true;
    }
    else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
    {
        cb->setValue((double)(value.toDouble()/scale));
        return true;
    }
    else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
    {
        cb->setValue((int)qRound(value.toDouble()/scale));
        return true;
    }
    else if(QSlider * cb=qobject_cast<QSlider *>(widget))
    {
        cb->setValue((int)qRound(value.toDouble()/scale));
        return true;
    }
    else if(QCheckBox * cb=qobject_cast<QCheckBox *>(widget))
    {
        bool bvalue=value.toString()=="TRUE";
        cb->setChecked(bvalue);
        return true;
    }
    else if(QLineEdit * cb=qobject_cast<QLineEdit *>(widget))
    {
        if(scale==0)
            cb->setText(value.toString());
        else
            cb->setText(QString::number((value.toDouble()/scale)));
        return true;
    }
    else
        return false;
}
/**
 * Sets a widget from a UAVObject field
 * @param widget pointer to the widget to set
 * @param field pointer to the field from where to get the value from
 * @param index index of the element to use
 * @param scale scale to be used on the assignement
 * @param hasLimits set to true if you want to limit the values (check wiki)
 * @return returns true if the assignement was successfull
 */
bool ConfigTaskWidget::setWidgetFromField(QWidget * widget,UAVObjectField * field,int index,double scale,bool hasLimits)
{
    if(!widget || !field)
        return false;
    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        if(cb->count()==0)
            loadWidgetLimits(cb,field,index,hasLimits,scale);
    }
    QVariant var=field->getValue(index);
    checkWidgetsLimits(widget,field,index,hasLimits,var,scale);
    bool ret=setWidgetFromVariant(widget,var,scale);
    if(ret)
        return true;
    else
    {
        qDebug()<<__FUNCTION__<<"widget to uavobject relation not implemented"<<widget->metaObject()->className();
        return false;
    }
}
void ConfigTaskWidget::checkWidgetsLimits(QWidget * widget,UAVObjectField * field,int index,bool hasLimits, QVariant value, double scale)
{
    if(!hasLimits)
        return;
    if(!field->isWithinLimits(value,index,currentBoard))
    {
        if(!widget->property("styleBackup").isValid())
            widget->setProperty("styleBackup",widget->styleSheet());
        widget->setStyleSheet(outOfLimitsStyle);
        widget->setProperty("wasOverLimits",(bool)true);
        if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
        {
            if(cb->findText(value.toString())==-1)
                cb->addItem(value.toString());
        }
        else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
        {
            if((double)(value.toDouble()/scale)>cb->maximum())
            {
                cb->setMaximum((double)(value.toDouble()/scale));
            }
            else if((double)(value.toDouble()/scale)<cb->minimum())
            {
                cb->setMinimum((double)(value.toDouble()/scale));
            }

        }
        else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
        {
            if((int)qRound(value.toDouble()/scale)>cb->maximum())
            {
                cb->setMaximum((int)qRound(value.toDouble()/scale));
            }
            else if((int)qRound(value.toDouble()/scale)<cb->minimum())
            {
                cb->setMinimum((int)qRound(value.toDouble()/scale));
            }
        }
        else if(QSlider * cb=qobject_cast<QSlider *>(widget))
        {
            if((int)qRound(value.toDouble()/scale)>cb->maximum())
            {
                cb->setMaximum((int)qRound(value.toDouble()/scale));
            }
            else if((int)qRound(value.toDouble()/scale)<cb->minimum())
            {
                cb->setMinimum((int)qRound(value.toDouble()/scale));
            }
        }

    }
    else if(widget->property("wasOverLimits").isValid())
    {
        if(widget->property("wasOverLimits").toBool())
        {
            widget->setProperty("wasOverLimits",(bool)false);
            if(widget->property("styleBackup").isValid())
            {
                QString style=widget->property("styleBackup").toString();
                widget->setStyleSheet(style);
            }
            loadWidgetLimits(widget,field,index,hasLimits,scale);
        }
    }
}

void ConfigTaskWidget::loadWidgetLimits(QWidget * widget,UAVObjectField * field,int index,bool hasLimits,double scale)
{   
    if(!widget || !field)
        return;
    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        cb->clear();
        QStringList option=field->getOptions();
        if(hasLimits)
        {
            foreach(QString str,option)
            {
                if(field->isWithinLimits(str,index,currentBoard))
                    cb->addItem(str);
            }
        }
        else
            cb->addItems(option);
    }
    if(!hasLimits)
        return;
    else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
    {
        if(field->getMaxLimit(index).isValid())
        {
            cb->setMaximum((double)(field->getMaxLimit(index,currentBoard).toDouble()/scale));
        }
        if(field->getMinLimit(index,currentBoard).isValid())
        {
            cb->setMinimum((double)(field->getMinLimit(index,currentBoard).toDouble()/scale));
        }
    }
    else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
    {
        if(field->getMaxLimit(index,currentBoard).isValid())
        {
            cb->setMaximum((int)qRound(field->getMaxLimit(index,currentBoard).toDouble()/scale));
        }
        if(field->getMinLimit(index,currentBoard).isValid())
        {
            cb->setMinimum((int)qRound(field->getMinLimit(index,currentBoard).toDouble()/scale));
        }
    }
    else if(QSlider * cb=qobject_cast<QSlider *>(widget))
    {
        if(field->getMaxLimit(index,currentBoard).isValid())
        {
            cb->setMaximum((int)qRound(field->getMaxLimit(index,currentBoard).toDouble()/scale));
        }
        if(field->getMinLimit(index,currentBoard).isValid())
        {
            cb->setMinimum((int)(field->getMinLimit(index,currentBoard).toDouble()/scale));
        }
    }
}

void ConfigTaskWidget::disableMouseWheelEvents()
{
    //Disable mouse wheel events
    foreach( QSpinBox * sp, findChildren<QSpinBox*>() ) {
        sp->installEventFilter( this );
    }
    foreach( QDoubleSpinBox * sp, findChildren<QDoubleSpinBox*>() ) {
        sp->installEventFilter( this );
    }
    foreach( QSlider * sp, findChildren<QSlider*>() ) {
        sp->installEventFilter( this );
    }
    foreach( QComboBox * sp, findChildren<QComboBox*>() ) {
        sp->installEventFilter( this );
    }
}

bool ConfigTaskWidget::eventFilter( QObject * obj, QEvent * evt ) {
    //Filter all wheel events, and ignore them
    if ( evt->type() == QEvent::Wheel &&
         (qobject_cast<QAbstractSpinBox*>( obj ) ||
          qobject_cast<QComboBox*>( obj ) ||
          qobject_cast<QAbstractSlider*>( obj ) ))
    {
        evt->ignore();
        return true;
    }
    return QWidget::eventFilter( obj, evt );
}
/**
  @}
  @}
  */
