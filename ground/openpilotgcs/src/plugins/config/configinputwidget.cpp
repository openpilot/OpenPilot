/**
 ******************************************************************************
 *
 * @file       configservowidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Servo input/output configuration panel for the config gadget
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

#include "configinputwidget.h"

#include "uavtalk/telemetrymanager.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>


ConfigInputWidget::ConfigInputWidget(QWidget *parent) : ConfigTaskWidget(parent),wizardStep(wizardWelcome),skipflag(false),loop(NULL)
{
    manualCommandObj = ManualControlCommand::GetInstance(getObjectManager());
    manualSettingsObj = ManualControlSettings::GetInstance(getObjectManager());
    receiverActivityObj=ReceiverActivity::GetInstance(getObjectManager());
    m_config = new Ui_InputWidget();
    m_config->setupUi(this);

    setupButtons(m_config->saveRCInputToRAM,m_config->saveRCInputToSD);

    int index=0;
    foreach(QString name,manualSettingsObj->getFields().at(0)->getElementNames())
    {
        inputChannelForm * inp=new inputChannelForm(this,index==0);
        m_config->advancedPage->layout()->addWidget(inp);
        inp->ui->channelName->setText(name);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelGroups",inp->ui->channelGroup,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelNumber",inp->ui->channelNumber,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelMin",inp->ui->channelMin,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelNeutral",inp->ui->channelNeutral,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelMax",inp->ui->channelMax,index);
        ++index;
    }
    QPushButton * goWizard=new QPushButton(tr("Start Wizard"),this);
    m_config->advancedPage->layout()->addWidget(goWizard);
    connect(goWizard,SIGNAL(clicked()),this,SLOT(goToWizard()));
    connect(m_config->wzNext,SIGNAL(clicked()),this,SLOT(wzNext()));
    connect(m_config->wzCancel,SIGNAL(clicked()),this,SLOT(wzCancel()));
    connect(m_config->wzBack,SIGNAL(clicked()),this,SLOT(wzBack()));

    m_config->stackedWidget->setCurrentIndex(0);
    addUAVObjectToWidgetRelation("ManualControlSettings","FlightModePosition",m_config->fmsModePos1,0);
    addUAVObjectToWidgetRelation("ManualControlSettings","FlightModePosition",m_config->fmsModePos2,1);
    addUAVObjectToWidgetRelation("ManualControlSettings","FlightModePosition",m_config->fmsModePos3,2);

    addUAVObjectToWidgetRelation("ManualControlSettings","Stabilization1Settings",m_config->fmsSsPos1Roll,"Roll");
    addUAVObjectToWidgetRelation("ManualControlSettings","Stabilization2Settings",m_config->fmsSsPos2Roll,"Roll");
    addUAVObjectToWidgetRelation("ManualControlSettings","Stabilization3Settings",m_config->fmsSsPos3Roll,"Roll");
    addUAVObjectToWidgetRelation("ManualControlSettings","Stabilization1Settings",m_config->fmsSsPos1Pitch,"Pitch");
    addUAVObjectToWidgetRelation("ManualControlSettings","Stabilization2Settings",m_config->fmsSsPos2Pitch,"Pitch");
    addUAVObjectToWidgetRelation("ManualControlSettings","Stabilization3Settings",m_config->fmsSsPos3Pitch,"Pitch");
    addUAVObjectToWidgetRelation("ManualControlSettings","Stabilization1Settings",m_config->fmsSsPos1Yaw,"Yaw");
    addUAVObjectToWidgetRelation("ManualControlSettings","Stabilization2Settings",m_config->fmsSsPos2Yaw,"Yaw");
    addUAVObjectToWidgetRelation("ManualControlSettings","Stabilization3Settings",m_config->fmsSsPos3Yaw,"Yaw");

    addUAVObjectToWidgetRelation("ManualControlSettings","Arming",m_config->armControl);
    addUAVObjectToWidgetRelation("ManualControlSettings","armTimeout",m_config->armTimeout,0,1000);
    enableControls(false);
    populateWidgets();
    refreshWidgetsValues();
    // Connect the help button
    connect(m_config->inputHelp, SIGNAL(clicked()), this, SLOT(openHelp()));
}

ConfigInputWidget::~ConfigInputWidget()
{

}

void ConfigInputWidget::openHelp()
{

    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/display/Doc/Input+Configuration", QUrl::StrictMode) );
}

void ConfigInputWidget::goToWizard()
{
    setupWizardWidget(wizardWelcome);
}

void ConfigInputWidget::wzCancel()
{
    m_config->stackedWidget->setCurrentIndex(0);
    foreach (QWidget * wd, extraWidgets)
    {
        if(wd)
            delete wd;
    }
    extraWidgets.clear();
    switch(wizardStep)
    {
    case wizardWelcome:
        break;
    case wizardChooseMode:
        break;
    case wizardIdentifySticks:
        disconnect(receiverActivityObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyControls()));
        break;
    case wizardIdentifyCenter:
        break;
    case wizardIdentifyLimits:
        disconnect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyLimits()));
        manualSettingsObj->setData(manualSettingsData);
        manualCommandObj->setMetadata(manualCommandObj->getDefaultMetadata());
        break;
    case wizardIdentifyInverted:
        break;
    case wizardFinish:
        break;
    default:
        break;
    }
    wizardStep=wizardWelcome;
}

void ConfigInputWidget::wzNext()
{
    setupWizardWidget(wizardStep+1);
}

void ConfigInputWidget::wzBack()
{
    setupWizardWidget(wizardStep-1);
}

void ConfigInputWidget::setupWizardWidget(int step)
{
    if(step==wizardWelcome)
    {
        if(wizardStep==wizardChooseMode)
        {
            delete extraWidgets.at(0);
            delete extraWidgets.at(1);
            extraWidgets.clear();
        }
        manualSettingsData=manualSettingsObj->getData();
        manualSettingsData.Arming=ManualControlSettings::ARMING_ALWAYSDISARMED;
        manualSettingsObj->setData(manualSettingsData);
        m_config->wzText->setText(tr("Welcome to the inputs configuration wizard.\n"
                                     "Please follow the instruction on the screen and only move your controls when asked to.\n"
                                     "At any time you can press 'back' to return to the previous screeen or 'Cancel' to cancel the wizard"
                                     "For your safety your arming setting is now 'Always Disarmed' please reenable it after this wizard."));
        m_config->stackedWidget->setCurrentIndex(1);
        m_config->wzBack->setEnabled(false);
        wizardStep=wizardWelcome;
    }
    else if(step==wizardChooseMode)
    {
        if(wizardStep==wizardIdentifySticks)
        {
            disconnect(receiverActivityObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyControls()));
            m_config->wzNext->setEnabled(true);
        }
        m_config->wzText->setText(tr("Please choose your transmiter type.\n"
                                     "Mode 1 means your throttle stick is on the right\n"
                                     "Mode 2 means your throttle stick is on the left\n"));
        m_config->wzBack->setEnabled(true);
        QRadioButton * mode1=new QRadioButton(tr("Mode 1"),this);
        QRadioButton * mode2=new QRadioButton(tr("Mode 2"),this);
        mode2->setChecked(true);
        extraWidgets.clear();
        extraWidgets.append(mode1);
        extraWidgets.append(mode2);
        m_config->checkBoxesLayout->layout()->addWidget(mode1);
        m_config->checkBoxesLayout->layout()->addWidget(mode2);
        wizardStep=wizardChooseMode;
    }
    else if(step==wizardIdentifySticks)
    {
        usedChannels.clear();
        if(wizardStep==wizardChooseMode)
        {
            QRadioButton * mode=qobject_cast<QRadioButton *>(extraWidgets.at(0));
            if(mode->isChecked())
                transmitterMode=mode1;
            else
                transmitterMode=mode2;
            delete extraWidgets.at(0);
            delete extraWidgets.at(1);
            extraWidgets.clear();
        }
        wizardStep=wizardIdentifySticks;
        currentCommand=0;
        m_config->wzText->setText(QString(tr("Please move each control once at a time according to the instructions and picture below.\n\n"
                                             "Move the %1 stick")).arg(manualSettingsObj->getField("ChannelGroups")->getElementNames().at(currentCommand)));
        manualSettingsData=manualSettingsObj->getData();
        connect(receiverActivityObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyControls()));
        m_config->wzNext->setEnabled(false);
    }
    else if(step==wizardIdentifyCenter)
    {
        if(wizardStep==wizardIdentifySticks)
            disconnect(receiverActivityObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyControls()));
        else
        {
            disconnect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyLimits()));
            manualSettingsObj->setData(manualSettingsData);
            manualCommandObj->setMetadata(manualCommandObj->getDefaultMetadata());
        }
        wizardStep=wizardIdentifyCenter;
        m_config->wzText->setText(QString(tr("Please center all control controls and press next when ready")));
    }
    else if(step==wizardIdentifyLimits)
    {
        if(wizardStep==wizardIdentifyCenter)
        {
            wizardStep=wizardIdentifyLimits;
            manualCommandData=manualCommandObj->getData();
            manualSettingsData=manualSettingsObj->getData();
            for(int i=0;i<ManualControlCommand::CHANNEL_NUMELEM;++i)
            {
                manualSettingsData.ChannelNeutral[i]=manualCommandData.Channel[i];
            }
            manualSettingsObj->setData(manualSettingsData);
        }
        wizardStep=wizardIdentifyLimits;
        m_config->wzText->setText(QString(tr("Please move all controls to their maximum extends on both directions and press next when ready")));
        UAVObject::Metadata mdata= manualCommandObj->getMetadata();
        mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
        mdata.flightTelemetryUpdatePeriod = 150;
        manualCommandObj->setMetadata(mdata);
        manualSettingsData=manualSettingsObj->getData();
        for(int i=0;i<ManualControlSettings::CHANNELMAX_NUMELEM;++i)
        {
            manualSettingsData.ChannelMin[i]=manualSettingsData.ChannelNeutral[i];
            manualSettingsData.ChannelMax[i]=manualSettingsData.ChannelNeutral[i];
        }
        connect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyLimits()));
    }
    else if(step==wizardIdentifyInverted)
    {
        if(wizardStep==wizardIdentifyLimits)
        {
            disconnect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyLimits()));
            manualSettingsObj->setData(manualSettingsData);
            manualCommandObj->setMetadata(manualCommandObj->getDefaultMetadata());
        }
        extraWidgets.clear();
        foreach(QString name,manualSettingsObj->getFields().at(0)->getElementNames())
        {
            QCheckBox * cb=new QCheckBox(name,this);
            extraWidgets.append(cb);
            m_config->checkBoxesLayout->layout()->addWidget(cb);
        }
        wizardStep=wizardIdentifyInverted;
        m_config->wzText->setText(QString(tr("Please check the picture below and check all controls which show an inverted movement  and press next when ready")));
    }
    else if(step==wizardFinish)
    {
        manualSettingsData=manualSettingsObj->getData();
        foreach(QWidget * wd,extraWidgets)
        {
            QCheckBox * cb=qobject_cast<QCheckBox *>(wd);
            if(cb)
            {
                if(cb->isChecked())
                {
                    int index=manualSettingsObj->getFields().at(0)->getElementNames().indexOf(cb->text());
                    qint16 aux;
                    aux=manualSettingsData.ChannelMax[index];
                    manualSettingsData.ChannelMax[index]=manualSettingsData.ChannelMin[index];
                    manualSettingsData.ChannelMin[index]=aux;
                }
            }
            delete cb;
        }
        wizardStep=wizardFinish;
        manualSettingsObj->setData(manualSettingsData);
        extraWidgets.clear();
        m_config->wzText->setText(QString(tr("You have completed this wizard, please check below if the picture below mimics your controls movement.\n"
                                             "This new settings aren't saved to the board yet, after pressing next you will go to the initial screen where you can do that.")));

    }

    else if(step==wizardFinish+1)
    {
        manualSettingsData=manualSettingsObj->getData();
        manualSettingsData.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_THROTTLE]=
                manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_THROTTLE]+
                ((manualSettingsData.ChannelMax[ManualControlSettings::CHANNELMAX_THROTTLE]-
                manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_THROTTLE])*0.02);
        manualSettingsObj->setData(manualSettingsData);
        m_config->stackedWidget->setCurrentIndex(0);
        wizardStep=wizardWelcome;
    }

}

void ConfigInputWidget::identifyControls()
{
    static int debounce=0;
    receiverActivityData=receiverActivityObj->getData();
    if(receiverActivityData.ActiveChannel==255)
        return;
    else
    {
        receiverActivityData=receiverActivityObj->getData();
        currentChannel.group=receiverActivityData.ActiveGroup;
        currentChannel.number=receiverActivityData.ActiveChannel;
        if(currentChannel==lastChannel)
            ++debounce;
        lastChannel.group= currentChannel.group;
        lastChannel.number=currentChannel.number;
        if(!usedChannels.contains(lastChannel) && debounce>5)
        {
            debounce=0;
            usedChannels.append(lastChannel);
            manualSettingsData=manualSettingsObj->getData();
            manualSettingsData.ChannelGroups[currentCommand]=currentChannel.group;
            manualSettingsData.ChannelNumber[currentCommand]=currentChannel.number;
            manualSettingsObj->setData(manualSettingsData);
        }
        else
            return;
    }
    ++currentCommand;
    if(currentCommand>ManualControlSettings::CHANNELGROUPS_NUMELEM-1)
    {
        disconnect(receiverActivityObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyControls()));
        m_config->wzNext->setEnabled(true);
    }
    m_config->wzText->setText(QString(tr("Please move each control once at a time according to the instructions and picture below.\n\n"
                                         "Move the %1 stick")).arg(manualSettingsObj->getFields().at(0)->getElementNames().at(currentCommand)));
    if(manualSettingsObj->getField("ChannelGroups")->getElementNames().at(currentCommand).contains("Accessory"))
    {
        m_config->wzNext->setEnabled(true);
    }
}

void ConfigInputWidget::identifyLimits()
{
    manualCommandData=manualCommandObj->getData();
    for(int i=0;i<ManualControlSettings::CHANNELMAX_NUMELEM;++i)
    {
        if(manualSettingsData.ChannelMin[i]>manualCommandData.Channel[i])
            manualSettingsData.ChannelMin[i]=manualCommandData.Channel[i];
        if(manualSettingsData.ChannelMax[i]<manualCommandData.Channel[i])
            manualSettingsData.ChannelMax[i]=manualCommandData.Channel[i];
    }
}

