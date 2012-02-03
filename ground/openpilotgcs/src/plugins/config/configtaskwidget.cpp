/**
 ******************************************************************************
 *
 * @file       configtaskwidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#include "uavsettingsimportexport/uavsettingsimportexportfactory.h"
#include "configgadgetwidget.h"

ConfigTaskWidget::ConfigTaskWidget(QWidget *parent) : QWidget(parent),isConnected(false),smartsave(NULL),dirty(false)
{
    pm = ExtensionSystem::PluginManager::instance();
    objManager = pm->getObject<UAVObjectManager>();
    connect((ConfigGadgetWidget*)parent, SIGNAL(autopilotConnected()),this, SLOT(onAutopilotConnect()));
    connect((ConfigGadgetWidget*)parent, SIGNAL(autopilotDisconnected()),this, SLOT(onAutopilotDisconnect()));
    UAVSettingsImportExportFactory * importexportplugin =  pm->getObject<UAVSettingsImportExportFactory>();
    connect(importexportplugin,SIGNAL(importAboutToBegin()),this,SLOT(invalidateObjects()));
}
void ConfigTaskWidget::addWidget(QWidget * widget)
{
    addUAVObjectToWidgetRelation("","",widget);
}
void ConfigTaskWidget::addUAVObject(QString objectName)
{
    addUAVObjectToWidgetRelation(objectName,"",NULL);
}
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
void ConfigTaskWidget::addUAVObjectToWidgetRelation(QString object, QString field, QWidget *widget, QString element, float scale, bool isLimited, QList<int> *defaultReloadGroups)
{
    UAVObject *obj=objManager->getObject(QString(object));
    Q_ASSERT(obj);
    UAVObjectField *_field;
    if(!field.isEmpty() && obj)
        _field = obj->getField(QString(field));
    int index=_field->getElementNames().indexOf(QString(element));
    addUAVObjectToWidgetRelation(object, field, widget,index,scale,isLimited,defaultReloadGroups);
}
void ConfigTaskWidget::addUAVObjectToWidgetRelation(QString object, QString field, QWidget * widget, int index,float scale,bool isLimited,QList<int>* defaultReloadGroups)
{
    if(addShadowWidget(object,field,widget,index,scale,isLimited,defaultReloadGroups))
        return;

    UAVObject *obj=NULL;
    UAVObjectField *_field=NULL;
    if(!object.isEmpty())
    {
        obj = objManager->getObject(QString(object));
        Q_ASSERT(obj);
        objectUpdates.insert(obj,true);
        connect(obj, SIGNAL(objectUpdated(UAVObject*)),this, SLOT(objectUpdated(UAVObject*)));
        connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshWidgetsValues()));
    }
    if(!field.isEmpty() && obj)
        _field = obj->getField(QString(field));
    objectToWidget * ow=new objectToWidget();
    ow->field=_field;
    ow->object=obj;
    ow->widget=widget;
    ow->index=index;
    ow->scale=scale;
    objOfInterest.append(ow);
    if(obj)
    {
        if(smartsave)
        {
            smartsave->addObject((UAVDataObject*)obj);
            emit objectAdded(obj);
        }
    }
    if(widget==NULL)
    {
        // do nothing
    }
    else
    {
        connectWidgetUpdatesToSlot(widget,SLOT(widgetsContentsChanged()));
    if(defaultReloadGroups)
        addWidgetToDefaultReloadGroups(widget,defaultReloadGroups);
    shadowsList.insert(widget,ow);
    }
}

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
    qDebug()<<"ConfigTaskWidget::saveObjectToSD";
    // saveObjectToSD is now handled by the UAVUtils plugin in one
    // central place (and one central queue)
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager* utilMngr = pm->getObject<UAVObjectUtilManager>();
    utilMngr->saveObjectToSD(obj);
}


UAVObjectManager* ConfigTaskWidget::getObjectManager() {
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr);
    return objMngr;
}

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

void ConfigTaskWidget::onAutopilotConnect()
{
    invalidateObjects();
    dirty=false;
    isConnected=true;
    enableControls(true);
    refreshWidgetsValues();
}

void ConfigTaskWidget::populateWidgets()
{
    bool dirtyBack=dirty;
    foreach(objectToWidget * ow,objOfInterest)
    {
        if(ow->object==NULL || ow->field==NULL || ow->widget==NULL)
        {
            // do nothing
        }
       else
            setWidgetFromField(ow->widget,ow->field,ow->index,ow->scale);
    }
    setDirty(dirtyBack);
}

void ConfigTaskWidget::refreshWidgetsValues()
{
    bool dirtyBack=dirty;
    foreach(objectToWidget * ow,objOfInterest)
    {
        if(ow->object==NULL || ow->field==NULL || ow->widget==NULL)
        {
            //do nothing
        }
        else
        {
            setWidgetFromField(ow->widget,ow->field,ow->index,ow->scale);
        }

    }
    setDirty(dirtyBack);
}

void ConfigTaskWidget::updateObjectsFromWidgets()
{
    foreach(objectToWidget * ow,objOfInterest)
    {
        if(ow->object==NULL || ow->field==NULL)
        {

        }
        else
            setFieldFromWidget(ow->widget,ow->field,ow->index,ow->scale);

    }
}

void ConfigTaskWidget::addApplySaveButtons(QPushButton *update, QPushButton *save)
{
    smartsave=new smartSaveButton(update,save);
    connect(smartsave,SIGNAL(preProcessOperations()), this, SLOT(updateObjectsFromWidgets()));
    connect(smartsave,SIGNAL(saveSuccessfull()),this,SLOT(clearDirty()));
    connect(smartsave,SIGNAL(beginOp()),this,SLOT(disableObjUpdates()));
    connect(smartsave,SIGNAL(endOp()),this,SLOT(enableObjUpdates()));
    if(objOfInterest.count()>0)
    {
        foreach(objectToWidget * oTw,objOfInterest)
        {
            smartsave->addObject((UAVDataObject*)oTw->object);
            emit objectAdded(oTw->object);
        }
    }
    enableControls(false);
}

void ConfigTaskWidget::enableControls(bool enable)
{
    if(smartsave)
        smartsave->enableControls(enable);
}

void ConfigTaskWidget::widgetsContentsChanged()
{
    float scale;
    qDebug()<<"sender:"<<(quint32)sender();
    foreach(QWidget * w,shadowsList.keys())
        qDebug()<<"in list:"<<(quint32)w;
    objectToWidget * oTw= shadowsList.value((QWidget*)sender(),NULL);
    if(oTw)
        qDebug()<<"in oTw OK"<<(quint32)oTw->widget;
    if(oTw)
    {
        if(oTw->widget==(QWidget*)sender())
        {
            scale=oTw->scale;
            qDebug()<<"sender was master";
        }
        else
        {
            foreach (shadow * sh, oTw->shadowsList)
            {
                if(sh->widget==(QWidget*)sender())
                {
                    scale=sh->scale;
                    qDebug()<<"sender was shadow";
                }
            }
        }
        if(oTw->widget!=(QWidget *)sender())
            setWidgetFromVariant(oTw->widget,getVariantFromWidget((QWidget*)sender(),scale),oTw->scale);
        foreach (shadow * sh, oTw->shadowsList)
        {
            if(sh->widget!=(QWidget*)sender())
                setWidgetFromVariant(sh->widget,getVariantFromWidget((QWidget*)sender(),scale),sh->scale);
        }
    }
    setDirty(true);
}

void ConfigTaskWidget::clearDirty()
{
    setDirty(false);
}
void ConfigTaskWidget::setDirty(bool value)
{
    dirty=value;
}
bool ConfigTaskWidget::isDirty()
{
    if(isConnected)
        return dirty;
    else
        return false;
}

void ConfigTaskWidget::disableObjUpdates()
{
    foreach(objectToWidget * obj,objOfInterest)
    {
        if(obj->object)
            disconnect(obj->object, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshWidgetsValues()));
    }
}

void ConfigTaskWidget::enableObjUpdates()
{
    foreach(objectToWidget * obj,objOfInterest)
    {
        if(obj->object)
            connect(obj->object, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshWidgetsValues()));
    }
}

void ConfigTaskWidget::objectUpdated(UAVObject *obj)
{
    objectUpdates[obj]=true;
}

void ConfigTaskWidget::removeObject(UAVObject * obj)
{
    emit objectRemoved(obj);
    QList<objectToWidget*> toRemove;
    foreach(objectToWidget * oTw,objOfInterest)
    {
        if(oTw->object==obj)
        {
            toRemove.append(oTw);
        }
    }
    foreach(objectToWidget * oTw,toRemove)
    {
        objOfInterest.removeAll(oTw);
        smartsave->removeObject((UAVDataObject*)oTw->object);
    }
}

void ConfigTaskWidget::removeAllObjects()
{
    foreach(objectToWidget * oTw,objOfInterest)
    {
        emit objectRemoved(oTw->object);
    }
    objOfInterest.clear();
    smartsave->removeAllObjects();
}

bool ConfigTaskWidget::allObjectsUpdated()
{
    bool ret=true;
    foreach(UAVObject *obj, objectUpdates.keys())
    {
        ret=ret & objectUpdates[obj];
    }
    qDebug()<<"ALL OBJECTS UPDATE:"<<ret;
    return ret;
}

void ConfigTaskWidget::invalidateObjects()
{
    foreach(UAVObject *obj, objectUpdates.keys())
    {
        objectUpdates[obj]=false;
    }
}

bool ConfigTaskWidget::addShadowWidget(QString object, QString field, QWidget *widget, int index, float scale, bool isLimited,QList<int>* defaultReloadGroups)
{
    foreach(objectToWidget * oTw,objOfInterest)
    {
        if(!oTw->object || !oTw->widget)
            continue;
        if(oTw->object->getName()==object && oTw->field->getName()==field && oTw->index==index)
        {
            shadow * sh=new shadow;
            sh->isLimited=isLimited;
            sh->scale=scale;
            sh->widget=widget;
            oTw->shadowsList.append(sh);
            shadowsList.insert(widget,oTw);
            connectWidgetUpdatesToSlot(widget,SLOT(widgetsContentsChanged()));
            if(defaultReloadGroups)
                addWidgetToDefaultReloadGroups(widget,defaultReloadGroups);
            return true;
        }
    }
    return false;
}

void ConfigTaskWidget::autoLoadWidgets()
{
    QPushButton * saveButtonWidget=NULL;
    QPushButton * applyButtonWidget=NULL;
    foreach(QObject * widget,this->children())
    {
        QVariant info=widget->property("objrelation");
        if(info.isValid())
        {
            uiRelationAutomation uiRelation;
            uiRelation.buttonType=none;
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
                        uiRelation.scale=value.toFloat();
                }
                else if(prop== "ismaster")
                {
                    if(value=="yes")
                        uiRelation.ismaster=true;
                    else
                        uiRelation.ismaster=false;
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
                }
                else if(prop== "buttongroup")
                {
                    foreach(QString s,value.split(","))
                    {
                        uiRelation.buttonGroup.append(s.toInt());
                    }

                }
            }
            if(!uiRelation.buttonType==none)
            {
                QPushButton * button=NULL;
                switch(uiRelation.buttonType)
                {
                case save_button:
                    saveButtonWidget=qobject_cast<QPushButton *>(widget);
                    break;
                case apply_button:
                    applyButtonWidget=qobject_cast<QPushButton *>(widget);
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
                default:
                    break;
                }
            }
            else if(uiRelation.ismaster)
            {
                QWidget * wid=qobject_cast<QWidget *>(widget);
                if(wid)
                    addUAVObjectToWidgetRelation(uiRelation.objname,uiRelation.fieldname,wid,uiRelation.element,uiRelation.haslimits,&uiRelation.buttonGroup);
            }
        }
    }
    if(saveButtonWidget && applyButtonWidget)
        addApplySaveButtons(applyButtonWidget,saveButtonWidget);
}

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
void ConfigTaskWidget::addDefaultButton(QPushButton *button, int buttonGroup)
{
    button->setProperty("group",buttonGroup);
    connect(button,SIGNAL(clicked()),this,SLOT(defaultButtonClicked()));
}
void ConfigTaskWidget::addReloadButton(QPushButton *button, int buttonGroup)
{
    button->setProperty("group",buttonGroup);
    connect(button,SIGNAL(clicked()),this,SLOT(reloadButtonClicked()));
}
void ConfigTaskWidget::defaultButtonClicked()
{
    int group=sender()->property("group").toInt();
    QList<objectToWidget*> * list=defaultReloadGroups.value(group);
    foreach(objectToWidget * oTw,*list)
    {
        if(!oTw->object)
            continue;
        UAVDataObject * temp=((UAVDataObject*)oTw->object)->dirtyClone();
        setWidgetFromField(oTw->widget,temp->getField(oTw->field->getName()),oTw->index,oTw->scale);
    }
}

void ConfigTaskWidget::reloadButtonClicked()
{
    int group=sender()->property("group").toInt();
    QList<objectToWidget*> * list=defaultReloadGroups.value(group);
    ObjectPersistence* objper = dynamic_cast<ObjectPersistence*>( getObjectManager()->getObject(ObjectPersistence::NAME) );
    QTimer * timeOut=new QTimer(this);
    QEventLoop * eventLoop=new QEventLoop(this);
    connect(timeOut, SIGNAL(timeout()),eventLoop,SLOT(quit()));
    connect(objper, SIGNAL(objectUpdated(UAVObject*)), eventLoop, SLOT(quit()));
    foreach(objectToWidget * oTw,*list)
    {
        if (oTw->object != NULL)
        {
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
                setWidgetFromField(oTw->widget,oTw->field,oTw->index,oTw->scale);
            }
            timeOut->stop();
        }
    }
    delete eventLoop;
    delete timeOut;
}

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
        connect(cb,SIGNAL(sliderMoved(int)),this,function);
    }
    else if(MixerCurveWidget * cb=qobject_cast<MixerCurveWidget *>(widget))
    {
        connect(cb,SIGNAL(curveUpdated(QList<double>,double)),this,function);
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
        connect(cb,SIGNAL(clicked()),this,function);
    }
    else if(QPushButton * cb=qobject_cast<QPushButton *>(widget))
    {
        connect(cb,SIGNAL(clicked()),this,function);
    }
    else
       qDebug()<<__FUNCTION__<<"widget to uavobject relation not implemented"<<widget->metaObject()->className();

}

bool ConfigTaskWidget::setFieldFromWidget(QWidget * widget,UAVObjectField * field,int index,float scale)
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
QVariant ConfigTaskWidget::getVariantFromWidget(QWidget * widget,float scale)
{
    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        return (QString)cb->currentText();
    }
    else if(QLabel * cb=qobject_cast<QLabel *>(widget))
    {
        return (QString)cb->text();
    }
    else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
    {
        return (double)(cb->value()* scale);
    }
    else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
    {
        return (int)(cb->value()* (int)scale);
    }
    else if(QSlider * cb=qobject_cast<QSlider *>(widget))
    {
        return(int)(cb->value()* (int)scale);
    }
    else if(QCheckBox * cb=qobject_cast<QCheckBox *>(widget))
    {
        return (bool)(cb->isChecked()?"TRUE":"FALSE");
    }
    else
        return QVariant();
}

bool ConfigTaskWidget::setWidgetFromVariant(QWidget *widget, QVariant value, float scale)
{
    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
          cb->setCurrentIndex(cb->findText(value.toString()));
          return true;
    }
    else if(QLabel * cb=qobject_cast<QLabel *>(widget))
    {
        cb->setText(value.toString());
        return true;
    }
    else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
    {
        cb->setValue(value.toDouble()/scale);
        return true;
    }
    else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
    {
        cb->setValue(value.toInt()/(int)scale);
        return true;
    }
    else if(QSlider * cb=qobject_cast<QSlider *>(widget))
    {
        cb->setValue(value.toInt()/(int)scale);
        return true;
    }
    else if(QCheckBox * cb=qobject_cast<QCheckBox *>(widget))
    {
        cb->setChecked(value.toBool());
        return true;
    }
    else
        return false;
}

bool ConfigTaskWidget::setWidgetFromField(QWidget * widget,UAVObjectField * field,int index,float scale)
{
    if(!widget || !field)
        return false;
    if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        if(cb->count()==0)
            cb->addItems(field->getOptions());
    }
    QVariant var=field->getValue(index);
    bool ret=setWidgetFromVariant(widget,var,scale);
    if(ret)
        return true;
    else
    {
        qDebug()<<__FUNCTION__<<"widget to uavobject relation not implemented"<<widget->metaObject()->className();
        return false;
    }
}

/**
  @}
  @}
  */
