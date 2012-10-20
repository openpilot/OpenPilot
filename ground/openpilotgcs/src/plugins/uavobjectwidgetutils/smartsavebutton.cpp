/**
 ******************************************************************************
 *
 * @file       smartsavebutton.cpp
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
#include "smartsavebutton.h"

smartSaveButton::smartSaveButton()
{

}

void smartSaveButton::addButtons(QPushButton *save, QPushButton *apply)
{
    buttonList.insert(save,save_button);
    buttonList.insert(apply,apply_button);
    connect(save,SIGNAL(clicked()),this,SLOT(processClick()));
    connect(apply,SIGNAL(clicked()),this,SLOT(processClick()));
}
void smartSaveButton::addApplyButton(QPushButton *apply)
{
    buttonList.insert(apply,apply_button);
    connect(apply,SIGNAL(clicked()),this,SLOT(processClick()));
}
void smartSaveButton::addSaveButton(QPushButton *save)
{
    buttonList.insert(save,save_button);
    connect(save,SIGNAL(clicked()),this,SLOT(processClick()));
}
void smartSaveButton::processClick()
{
    emit beginOp();
    bool save=false;
    QPushButton *button=qobject_cast<QPushButton *>(sender());
    if(!button)
        return;
    if(buttonList.value(button)==save_button)
        save=true;
    processOperation(button,save);

}

void smartSaveButton::processOperation(QPushButton * button,bool save)
{
    emit preProcessOperations();
    if(button)
    {
        button->setEnabled(false);
        button->setIcon(QIcon(":/uploader/images/system-run.svg"));
    }
    QTimer timer;
    timer.setSingleShot(true);
    bool error=false;
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager* utilMngr = pm->getObject<UAVObjectUtilManager>();
    foreach(UAVDataObject * obj,objects)
    {
        UAVObject::Metadata mdata= obj->getMetadata();
        if(UAVObject::GetGcsAccess(mdata)==UAVObject::ACCESS_READONLY)
            continue;
        up_result=false;
        current_object=obj;
        for(int i=0;i<3;++i)
        {
            qDebug()<<"SMARTSAVEBUTTON"<<"Upload try number"<<i<<"Object"<<obj->getName();
            connect(obj,SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(transaction_finished(UAVObject*, bool)));
            connect(&timer,SIGNAL(timeout()),&loop,SLOT(quit()));
            obj->updated();
            timer.start(3000);
            //qDebug()<<"begin loop";
            loop.exec();
            if(timer.isActive())
            {
                qDebug()<<"SMARTSAVEBUTTON"<<"Upload did not timeout"<<i<<"Object"<<obj->getName();
            }
            else
                qDebug()<<"SMARTSAVEBUTTON"<<"Upload TIMEOUT"<<i<<"Object"<<obj->getName();
            timer.stop();
            disconnect(obj,SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(transaction_finished(UAVObject*, bool)));
            disconnect(&timer,SIGNAL(timeout()),&loop,SLOT(quit()));
            if(up_result)
                break;
        }
        if(up_result==false)
        {
            qDebug()<<"SMARTSAVEBUTTON"<<"Object upload error:"<<obj->getName();
            error=true;
            continue;
        }
        sv_result=false;
        current_objectID=obj->getObjID();
        if(save && (obj->isSettings()))
        {
            for(int i=0;i<3;++i)
            {
                qDebug()<<"SMARTSAVEBUTTON"<<"Save try number"<<i<<"Object"<<obj->getName();
                connect(utilMngr,SIGNAL(saveCompleted(int,bool)),this,SLOT(saving_finished(int,bool)));
                connect(&timer,SIGNAL(timeout()),&loop,SLOT(quit()));
                utilMngr->saveObjectToSD(obj);
                timer.start(3000);
                loop.exec();
                if(timer.isActive())
                {
                    qDebug()<<"SMARTSAVEBUTTON"<<"Saving did not timeout"<<i<<"Object"<<obj->getName();
                }
                else
                    qDebug()<<"SMARTSAVEBUTTON"<<"Saving TIMEOUT"<<i<<"Object"<<obj->getName();
                timer.stop();
                disconnect(utilMngr,SIGNAL(saveCompleted(int,bool)),this,SLOT(saving_finished(int,bool)));
                disconnect(&timer,SIGNAL(timeout()),&loop,SLOT(quit()));
                if(sv_result)
                    break;
            }
            if(sv_result==false)
            {
                qDebug()<<"SMARTSAVEBUTTON"<<"failed to save:"<<obj->getName();
                error=true;
            }
        }
    }
    if(button)
        button->setEnabled(true);
    if(!error)
    {
        if(button)
            button->setIcon(QIcon(":/uploader/images/dialog-apply.svg"));
        emit saveSuccessfull();
    }
    else
    {
        if(button)
            button->setIcon(QIcon(":/uploader/images/process-stop.svg"));
    }
    emit endOp();
}

void smartSaveButton::setObjects(QList<UAVDataObject *> list)
{
    objects=list;
}

void smartSaveButton::addObject(UAVDataObject * obj)
{
    Q_ASSERT(obj);
    if(!objects.contains(obj))
        objects.append(obj);
}
void smartSaveButton::removeObject(UAVDataObject * obj)
{
    if(objects.contains(obj))
        objects.removeAll(obj);
}
void smartSaveButton::removeAllObjects()
{
    objects.clear();
}
void smartSaveButton::clearObjects()
{
    objects.clear();
}
void smartSaveButton::transaction_finished(UAVObject* obj, bool result)
{
    if(current_object==obj)
    {
        up_result=result;
        loop.quit();
    }
}

void smartSaveButton::saving_finished(int id, bool result)
{
    if(id==current_objectID)
    {
        sv_result=result;
        //qDebug()<<"saving_finished result="<<result;
        loop.quit();
    }
}

void smartSaveButton::enableControls(bool value)
{
    foreach(QPushButton * button,buttonList.keys())
        button->setEnabled(value);
}

void smartSaveButton::resetIcons()
{
    foreach(QPushButton * button,buttonList.keys())
        button->setIcon(QIcon());
}

void smartSaveButton::apply()
{
    processOperation(NULL,false);
}

void smartSaveButton::save()
{
    processOperation(NULL,true);
}


