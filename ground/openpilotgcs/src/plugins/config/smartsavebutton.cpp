#include "smartsavebutton.h"

smartSaveButton::smartSaveButton(QPushButton * update, QPushButton * save):bupdate(update),bsave(save)
{
    connect(bsave,SIGNAL(clicked()),this,SLOT(processClick()));
    connect(bupdate,SIGNAL(clicked()),this,SLOT(processClick()));

}
void smartSaveButton::processClick()
{
    bool save=false;
    QPushButton *button=bupdate;
    if(sender()==bsave)
    {
        save=true;
        button=bsave;
    }
    emit preProcessOperations();
    button->setEnabled(false);
    button->setIcon(QIcon(":/uploader/images/system-run.svg"));
    QTimer timer;
    timer.setSingleShot(true);
    bool error=false;
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager* utilMngr = pm->getObject<UAVObjectUtilManager>();
    foreach(UAVObject * obj,objects)
    {
        up_result=false;
        current_object=obj;
        for(int i=0;i<3;++i)
        {

            connect(obj,SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(transaction_finished(UAVObject*, bool)));
            connect(&timer,SIGNAL(timeout()),&loop,SLOT(quit()));
            obj->updated();
            timer.start(1000);
            loop.exec();
            timer.stop();
            disconnect(obj,SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(transaction_finished(UAVObject*, bool)));
            disconnect(&timer,SIGNAL(timeout()),&loop,SLOT(quit()));
            if(up_result)
                break;
        }
        if(up_result==false)
        {
            error=true;
            continue;
        }
        sv_result=false;
        current_objectID=obj->getObjID();
        if(save)
        {
        for(int i=0;i<3;++i)
        {
            connect(utilMngr,SIGNAL(saveCompleted(int,bool)),this,SLOT(saving_finished(int,bool)));
            connect(&timer,SIGNAL(timeout()),&loop,SLOT(quit()));
            utilMngr->saveObjectToSD(obj);
            timer.start(1000);
            loop.exec();
            timer.stop();
            disconnect(utilMngr,SIGNAL(saveCompleted(int,bool)),this,SLOT(saving_finished(int,bool)));
            disconnect(&timer,SIGNAL(timeout()),&loop,SLOT(quit()));
            if(sv_result)
                break;
        }
        if(sv_result==false)
        {
            error=true;
        }
        }
    }
    button->setEnabled(true);
    if(!error)
    {
        button->setIcon(QIcon(":/uploader/images/dialog-apply.svg"));
    }
    else
    {
        button->setIcon(QIcon(":/uploader/images/process-stop.svg"));
    }
}

void smartSaveButton::setObjects(QList<UAVObject *> list)
{
    objects=list;
}

void smartSaveButton::addObject(UAVObject * obj)
{
    objects.append(obj);
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
        loop.quit();
    }
}

void smartSaveButton::enableControls(bool value)
{
    bupdate->setEnabled(value);
    bsave->setEnabled(value);
}
