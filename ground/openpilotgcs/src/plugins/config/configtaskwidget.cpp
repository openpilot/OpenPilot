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


ConfigTaskWidget::ConfigTaskWidget(QWidget *parent) : QWidget(parent),smartsave(NULL),dirty(false)
{
    pm = ExtensionSystem::PluginManager::instance();
    objManager = pm->getObject<UAVObjectManager>();
    connect(parent, SIGNAL(autopilotConnected()),this, SLOT(onAutopilotConnect()));
    connect(parent, SIGNAL(autopilotDisconnected()),this, SLOT(onAutopilotDisconnect()));
}
void ConfigTaskWidget::addWidget(QWidget * widget)
{
    addUAVObjectToWidgetRelation("","",widget);
}
void ConfigTaskWidget::addUAVObject(QString objectName)
{
    addUAVObjectToWidgetRelation(objectName,"",NULL);
}
void ConfigTaskWidget::addUAVObjectToWidgetRelation(QString object, QString field, QWidget * widget)
{
    UAVObject *obj=NULL;
    UAVObjectField *_field=NULL;
    if(!object.isEmpty())
        obj = objManager->getObject(QString(object));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(refreshWidgetsValues()));
    //smartsave->addObject(obj);
    if(!field.isEmpty() && obj)
        _field = obj->getField(QString(field));
    objectToWidget * ow=new objectToWidget();
    ow->field=_field;
    ow->object=obj;
    ow->widget=widget;
    objOfInterest.append(ow);
    if(obj)
        smartsave->addObject(obj);
    if(widget==NULL)
    {
        // do nothing
    }
    else if(QComboBox * cb=qobject_cast<QComboBox *>(widget))
    {
        connect(cb,SIGNAL(currentIndexChanged(int)),this,SLOT(widgetsContentsChanged()));
    }
    else if(QSlider * cb=qobject_cast<QSlider *>(widget))
    {
        connect(cb,SIGNAL(sliderMoved(int)),this,SLOT(widgetsContentsChanged()));
    }
    else if(MixerCurveWidget * cb=qobject_cast<MixerCurveWidget *>(widget))
    {
        connect(cb,SIGNAL(curveUpdated(QList<double>,double)),this,SLOT(widgetsContentsChanged()));
    }
    else if(QTableWidget * cb=qobject_cast<QTableWidget *>(widget))
    {
        connect(cb,SIGNAL(cellChanged(int,int)),this,SLOT(widgetsContentsChanged()));
    }
    else if(QSpinBox * cb=qobject_cast<QSpinBox *>(widget))
    {
        connect(cb,SIGNAL(valueChanged(int)),this,SLOT(widgetsContentsChanged()));
    }
    else if(QDoubleSpinBox * cb=qobject_cast<QDoubleSpinBox *>(widget))
    {
        connect(cb,SIGNAL(valueChanged(double)),this,SLOT(widgetsContentsChanged()));
    }
}


ConfigTaskWidget::~ConfigTaskWidget()
{
    delete smartsave;
}

void ConfigTaskWidget::saveObjectToSD(UAVObject *obj)
{
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
        enableControls(false);
}

void ConfigTaskWidget::onAutopilotConnect()
{
        enableControls(true);
        refreshWidgetsValues();
}

void ConfigTaskWidget::populateWidgets()
{
    foreach(objectToWidget * ow,objOfInterest)
    {
        if(ow->object==NULL || ow->field==NULL)
        {
            // do nothing
        }
        else if(QComboBox * cb=qobject_cast<QComboBox *>(ow->widget))
        {
            cb->addItems(ow->field->getOptions());
            cb->setCurrentIndex(cb->findText(ow->field->getValue().toString()));
        }
        else if(QLabel * cb=qobject_cast<QLabel *>(ow->widget))
        {
            cb->setText(ow->field->getValue().toString());
        }
    }
    dirty=false;
}

void ConfigTaskWidget::refreshWidgetsValues()
{
    foreach(objectToWidget * ow,objOfInterest)
    {
        if(ow->object==NULL || ow->field==NULL)
        {
            //do nothing
        }
        else if(QComboBox * cb=qobject_cast<QComboBox *>(ow->widget))
        {
            cb->setCurrentIndex(cb->findText(ow->field->getValue().toString()));
        }
        else if(QLabel * cb=qobject_cast<QLabel *>(ow->widget))
        {
            cb->setText(ow->field->getValue().toString());
        }
    }
}

void ConfigTaskWidget::updateObjectsFromWidgets()
{
    foreach(objectToWidget * ow,objOfInterest)
    {
        if(ow->object==NULL || ow->field==NULL)
        {
            //do nothing
        }
        else if(QComboBox * cb=qobject_cast<QComboBox *>(ow->widget))
        {
                ow->field->setValue(cb->currentText());
        }
        else if(QLabel * cb=qobject_cast<QLabel *>(ow->widget))
        {
            ow->field->setValue(cb->text());
        }
    }
}

void ConfigTaskWidget::setupButtons(QPushButton *update, QPushButton *save)
{
    smartsave=new smartSaveButton(update,save);
    connect(smartsave, SIGNAL(preProcessOperations()), this, SLOT(updateObjectsFromWidgets()));
    connect(smartsave,SIGNAL(saveSuccessfull()),this,SLOT(clearDirty()));
    connect(smartsave,SIGNAL(beginOp()),this,SLOT(disableObjUpdates()));
    connect(smartsave,SIGNAL(endOp()),this,SLOT(enableObjUpdates()));
}

void ConfigTaskWidget::enableControls(bool enable)
{
    if(smartsave)
        smartsave->enableControls(enable);
}

void ConfigTaskWidget::widgetsContentsChanged()
{
    qDebug()<<"dirty!!!";
    dirty=true;
}

void ConfigTaskWidget::clearDirty()
{
    dirty=false;
}

bool ConfigTaskWidget::isDirty()
{
    return dirty;
}

void ConfigTaskWidget::refreshValues()
{
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








/**
  @}
  @}
  */
