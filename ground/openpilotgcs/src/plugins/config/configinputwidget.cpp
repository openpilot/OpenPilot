/**
 ******************************************************************************
 *
 * @file       configinputwidget.cpp
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
#include <utils/stylehelper.h>
#include <QMessageBox>

#define ACCESS_MIN_MOVE -6
#define ACCESS_MAX_MOVE 6
#define STICK_MIN_MOVE -8
#define STICK_MAX_MOVE 8

QList<inputChannelForm*> inputList;

ConfigInputWidget::ConfigInputWidget(QWidget *parent) : ConfigTaskWidget(parent),wizardStep(wizardWelcome),loop(NULL),skipflag(false)
{
    manualCommandObj = ManualControlCommand::GetInstance(getObjectManager());
    manualSettingsObj = ManualControlSettings::GetInstance(getObjectManager());
    receiverActivityObj=ReceiverActivity::GetInstance(getObjectManager());
    m_config = new Ui_InputWidget();
    m_config->setupUi(this);

    setupButtons(m_config->saveRCInputToRAM,m_config->saveRCInputToSD);

    inputList.clear();

    int index=0;
    foreach(QString name,manualSettingsObj->getFields().at(0)->getElementNames())
    {
        Q_ASSERT(index < ManualControlSettings::CHANNELGROUPS_NUMELEM);
        inputChannelForm * inp=new inputChannelForm(this,index==0);
        m_config->channelSettings->layout()->addWidget(inp);
        inp->ui->channelName->setText(name);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelGroups",inp->ui->channelGroup,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelNumber",inp->ui->channelNumber,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelMin",inp->ui->channelMin,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelNeutral",inp->ui->channelNeutral,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelMax",inp->ui->channelMax,index);
        inputList.append(inp);
        ++index;
    }

    connect(m_config->configurationWizard,SIGNAL(clicked()),this,SLOT(goToNormalWizard()));
    connect(m_config->runCalibration,SIGNAL(toggled(bool)),this, SLOT(simpleCalibration(bool)));
    connect(manualSettingsObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(settingsUpdated()));

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
    addUAVObjectToWidgetRelation("ManualControlSettings","ArmedTimeout",m_config->armTimeout,0,1000);
    connect( ManualControlCommand::GetInstance(getObjectManager()),SIGNAL(objectUpdated(UAVObject*)),this,SLOT(moveFMSlider()));
    enableControls(false);

    populateWidgets();
    refreshWidgetsValues();
    // Connect the help button
    connect(m_config->inputHelp, SIGNAL(clicked()), this, SLOT(openHelp()));

    m_config->graphicsView->setScene(new QGraphicsScene(this));
    m_config->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_renderer = new QSvgRenderer();
    QGraphicsScene *l_scene = m_config->graphicsView->scene();
    m_config->graphicsView->setBackgroundBrush(QBrush(Utils::StyleHelper::baseColor()));
    if (QFile::exists(":/configgadget/images/TX.svg") && m_renderer->load(QString(":/configgadget/images/TX.svg")) && m_renderer->isValid())
    {
        l_scene->clear(); // Deletes all items contained in the scene as well.

        m_txBackground = new QGraphicsSvgItem();
        // All other items will be clipped to the shape of the background
        m_txBackground->setFlags(QGraphicsItem::ItemClipsChildrenToShape|
                                 QGraphicsItem::ItemClipsToShape);
        m_txBackground->setSharedRenderer(m_renderer);
        m_txBackground->setElementId("background");
        l_scene->addItem(m_txBackground);

        m_txMainBody = new QGraphicsSvgItem();
        m_txMainBody->setParentItem(m_txBackground);
        m_txMainBody->setSharedRenderer(m_renderer);
        m_txMainBody->setElementId("body");
        l_scene->addItem(m_txMainBody);

        m_txLeftStick = new QGraphicsSvgItem();
        m_txLeftStick->setParentItem(m_txBackground);
        m_txLeftStick->setSharedRenderer(m_renderer);
        m_txLeftStick->setElementId("ljoy");

        m_txRightStick = new QGraphicsSvgItem();
        m_txRightStick->setParentItem(m_txBackground);
        m_txRightStick->setSharedRenderer(m_renderer);
        m_txRightStick->setElementId("rjoy");

        m_txAccess0 = new QGraphicsSvgItem();
        m_txAccess0->setParentItem(m_txBackground);
        m_txAccess0->setSharedRenderer(m_renderer);
        m_txAccess0->setElementId("access0");

        m_txAccess1 = new QGraphicsSvgItem();
        m_txAccess1->setParentItem(m_txBackground);
        m_txAccess1->setSharedRenderer(m_renderer);
        m_txAccess1->setElementId("access1");

        m_txAccess2 = new QGraphicsSvgItem();
        m_txAccess2->setParentItem(m_txBackground);
        m_txAccess2->setSharedRenderer(m_renderer);
        m_txAccess2->setElementId("access2");

        m_txFlightMode = new QGraphicsSvgItem();
        m_txFlightMode->setParentItem(m_txBackground);
        m_txFlightMode->setSharedRenderer(m_renderer);
        m_txFlightMode->setElementId("flightModeCenter");
        m_txFlightMode->setZValue(-10);

        m_txArrows = new QGraphicsSvgItem();
        m_txArrows->setParentItem(m_txBackground);
        m_txArrows->setSharedRenderer(m_renderer);
        m_txArrows->setElementId("arrows");
        m_txArrows->setVisible(false);

        QRectF orig=m_renderer->boundsOnElement("ljoy");
        QMatrix Matrix = m_renderer->matrixForElement("ljoy");
        orig=Matrix.mapRect(orig);
        m_txLeftStickOrig.translate(orig.x(),orig.y());
        m_txLeftStick->setTransform(m_txLeftStickOrig,false);

        orig=m_renderer->boundsOnElement("arrows");
        Matrix = m_renderer->matrixForElement("arrows");
        orig=Matrix.mapRect(orig);
        m_txArrowsOrig.translate(orig.x(),orig.y());
        m_txArrows->setTransform(m_txArrowsOrig,false);

        orig=m_renderer->boundsOnElement("body");
        Matrix = m_renderer->matrixForElement("body");
        orig=Matrix.mapRect(orig);
        m_txMainBodyOrig.translate(orig.x(),orig.y());
        m_txMainBody->setTransform(m_txMainBodyOrig,false);

        orig=m_renderer->boundsOnElement("flightModeCenter");
        Matrix = m_renderer->matrixForElement("flightModeCenter");
        orig=Matrix.mapRect(orig);
        m_txFlightModeCOrig.translate(orig.x(),orig.y());
        m_txFlightMode->setTransform(m_txFlightModeCOrig,false);

        orig=m_renderer->boundsOnElement("flightModeLeft");
        Matrix = m_renderer->matrixForElement("flightModeLeft");
        orig=Matrix.mapRect(orig);
        m_txFlightModeLOrig.translate(orig.x(),orig.y());
        orig=m_renderer->boundsOnElement("flightModeRight");
        Matrix = m_renderer->matrixForElement("flightModeRight");
        orig=Matrix.mapRect(orig);
        m_txFlightModeROrig.translate(orig.x(),orig.y());

        orig=m_renderer->boundsOnElement("rjoy");
        Matrix = m_renderer->matrixForElement("rjoy");
        orig=Matrix.mapRect(orig);
        m_txRightStickOrig.translate(orig.x(),orig.y());
        m_txRightStick->setTransform(m_txRightStickOrig,false);

        orig=m_renderer->boundsOnElement("access0");
        Matrix = m_renderer->matrixForElement("access0");
        orig=Matrix.mapRect(orig);
        m_txAccess0Orig.translate(orig.x(),orig.y());
        m_txAccess0->setTransform(m_txAccess0Orig,false);

        orig=m_renderer->boundsOnElement("access1");
        Matrix = m_renderer->matrixForElement("access1");
        orig=Matrix.mapRect(orig);
        m_txAccess1Orig.translate(orig.x(),orig.y());
        m_txAccess1->setTransform(m_txAccess1Orig,false);

        orig=m_renderer->boundsOnElement("access2");
        Matrix = m_renderer->matrixForElement("access2");
        orig=Matrix.mapRect(orig);
        m_txAccess2Orig.translate(orig.x(),orig.y());
        m_txAccess2->setTransform(m_txAccess2Orig,true);
    }
    m_config->graphicsView->fitInView(m_txMainBody, Qt::KeepAspectRatio );
    animate=new QTimer(this);
    connect(animate,SIGNAL(timeout()),this,SLOT(moveTxControls()));
}
void ConfigInputWidget::resetTxControls()
{

    m_txLeftStick->setTransform(m_txLeftStickOrig,false);
    m_txRightStick->setTransform(m_txRightStickOrig,false);
    m_txAccess0->setTransform(m_txAccess0Orig,false);
    m_txAccess1->setTransform(m_txAccess1Orig,false);
    m_txAccess2->setTransform(m_txAccess2Orig,false);
    m_txFlightMode->setElementId("flightModeCenter");
    m_txFlightMode->setTransform(m_txFlightModeCOrig,false);
    m_txArrows->setVisible(false);
}

ConfigInputWidget::~ConfigInputWidget()
{

}

void ConfigInputWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_config->graphicsView->fitInView(m_txBackground, Qt::KeepAspectRatio );
}

void ConfigInputWidget::openHelp()
{

    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/display/Doc/Input+Configuration", QUrl::StrictMode) );
}
void ConfigInputWidget::goToSimpleWizard()
{
    isSimple=true;
    goToWizard();
}
void ConfigInputWidget::goToNormalWizard()
{
    isSimple=false;
    goToWizard();
}
void ConfigInputWidget::goToWizard()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Arming Settings are now set to Always Disarmed for your safety."));
    msgBox.setDetailedText(tr("You will have to reconfigure arming settings yourself afterwards."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
    setupWizardWidget(wizardWelcome);
    m_config->graphicsView->fitInView(m_txBackground, Qt::KeepAspectRatio );
}

void ConfigInputWidget::wzCancel()
{
    dimOtherControls(false);
    manualCommandObj->setMetadata(manualCommandObj->getDefaultMetadata());
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
        m_config->graphicsView->setVisible(false);
        setTxMovement(nothing);
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
                                     "Please follow the instructions on the screen and only move your controls when asked to.\n"
                                     "Make sure you already configured your hardware settings on the proper tab and restarted your board.\n"
                                     "At any time you can press 'back' to return to the previous screeen or 'Cancel' to cancel the wizard.\n"));
        m_config->stackedWidget->setCurrentIndex(1);
        m_config->wzBack->setEnabled(false);
        wizardStep=wizardWelcome;
    }
    else if(step==wizardChooseMode)
    {
        m_config->graphicsView->setVisible(true);
        setTxMovement(nothing);
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
    else if(step==wizardIdentifySticks && !isSimple)
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
        setMoveFromCommand(currentCommand);
        m_config->wzText->setText(QString(tr("Please move each control once at a time according to the instructions and picture below.\n\n"
                                             "Move the %1 stick")).arg(manualSettingsObj->getField("ChannelGroups")->getElementNames().at(currentCommand)));
        manualSettingsData=manualSettingsObj->getData();
        connect(receiverActivityObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyControls()));
        m_config->wzNext->setEnabled(false);
    }
    else if(step==wizardIdentifyCenter || (isSimple && step==wizardIdentifySticks))
    {
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
        setTxMovement(centerAll);
        if(wizardStep==wizardIdentifySticks)
            disconnect(receiverActivityObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyControls()));
        else
        {
            disconnect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyLimits()));
            manualSettingsObj->setData(manualSettingsData);
            manualCommandObj->setMetadata(manualCommandObj->getDefaultMetadata());
        }
        wizardStep=wizardIdentifyCenter;
        m_config->wzText->setText(QString(tr("Please center all control controls and press next when ready (if your FlightMode switch has only two positions, leave it on either position)")));
    }
    else if(step==wizardIdentifyLimits)
    {
        dimOtherControls(false);
        setTxMovement(moveAll);
        if(wizardStep==wizardIdentifyCenter)
        {
            wizardStep=wizardIdentifyLimits;
            manualCommandData=manualCommandObj->getData();
            manualSettingsData=manualSettingsObj->getData();
            for(unsigned int i=0;i<ManualControlCommand::CHANNEL_NUMELEM;++i)
            {
                manualSettingsData.ChannelNeutral[i]=manualCommandData.Channel[i];
            }
            manualSettingsObj->setData(manualSettingsData);
        }
        if(wizardStep==wizardIdentifyInverted)
        {
            foreach(QWidget * wd,extraWidgets)
            {
                QCheckBox * cb=qobject_cast<QCheckBox *>(wd);
                if(cb)
                {
                    disconnect(cb,SIGNAL(toggled(bool)),this,SLOT(invertControls()));
                    delete cb;
                }
            }
        }
        extraWidgets.clear();
        disconnect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        wizardStep=wizardIdentifyLimits;
        m_config->wzText->setText(QString(tr("Please move all controls to their maximum extents on both directions and press next when ready")));
        UAVObject::Metadata mdata= manualCommandObj->getMetadata();
        mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
        mdata.flightTelemetryUpdatePeriod = 150;
        manualCommandObj->setMetadata(mdata);
        manualSettingsData=manualSettingsObj->getData();
        for(uint i=0;i<ManualControlSettings::CHANNELMAX_NUMELEM;++i)
        {
            manualSettingsData.ChannelMin[i]=manualSettingsData.ChannelNeutral[i];
            manualSettingsData.ChannelMax[i]=manualSettingsData.ChannelNeutral[i];
        }
        connect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyLimits()));
    }
    else if(step==wizardIdentifyInverted)
    {
        dimOtherControls(true);
        setTxMovement(nothing);
        if(wizardStep==wizardIdentifyLimits)
        {
            disconnect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyLimits()));
            manualSettingsObj->setData(manualSettingsData);
        }
        extraWidgets.clear();
        foreach(QString name,manualSettingsObj->getFields().at(0)->getElementNames())
        {
            if(!name.contains("Access") &&  !name.contains("Flight"))
            {
                QCheckBox * cb=new QCheckBox(name,this);
                extraWidgets.append(cb);
                m_config->checkBoxesLayout->layout()->addWidget(cb);
                connect(cb,SIGNAL(toggled(bool)),this,SLOT(invertControls()));
            }
        }
        connect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        wizardStep=wizardIdentifyInverted;
        m_config->wzText->setText(QString(tr("Please check the picture below and check all the sticks which show an inverted movement and press next when ready")));
    }
    else if(step==wizardFinish)
    {
        foreach(QWidget * wd,extraWidgets)
        {
            QCheckBox * cb=qobject_cast<QCheckBox *>(wd);
            if(cb)
            {
                disconnect(cb,SIGNAL(toggled(bool)),this,SLOT(invertControls()));
                delete cb;
            }
        }
        wizardStep=wizardFinish;
        extraWidgets.clear();
        m_config->wzText->setText(QString(tr("You have completed this wizard, please check below if the picture below mimics your sticks movement.\n"
                                             "This new settings aren't saved to the board yet, after pressing next you will go to the initial screen where you can do that.")));

    }

    else if(step==wizardFinish+1)
    {
        setTxMovement(nothing);
        manualCommandObj->setMetadata(manualCommandObj->getDefaultMetadata());
        disconnect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        manualSettingsData=manualSettingsObj->getData();
        manualSettingsData.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_THROTTLE]=
                manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_THROTTLE]+
                ((manualSettingsData.ChannelMax[ManualControlSettings::CHANNELMAX_THROTTLE]-
                  manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_THROTTLE])*0.02);
        if((abs(manualSettingsData.ChannelMax[ManualControlSettings::CHANNELMAX_FLIGHTMODE]-manualSettingsData.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_FLIGHTMODE])<100) ||
                (abs(manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_FLIGHTMODE]-manualSettingsData.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_FLIGHTMODE])<100))
        {
            manualSettingsData.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_FLIGHTMODE]=manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_FLIGHTMODE]+
                    (manualSettingsData.ChannelMax[ManualControlSettings::CHANNELMAX_FLIGHTMODE]-manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_FLIGHTMODE])/2;
        }
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
        if(!usedChannels.contains(lastChannel) && debounce>1)
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
    setMoveFromCommand(currentCommand);
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
    for(uint i=0;i<ManualControlSettings::CHANNELMAX_NUMELEM;++i)
    {
        if(manualSettingsData.ChannelMin[i]>manualCommandData.Channel[i])
            manualSettingsData.ChannelMin[i]=manualCommandData.Channel[i];
        if(manualSettingsData.ChannelMax[i]<manualCommandData.Channel[i])
            manualSettingsData.ChannelMax[i]=manualCommandData.Channel[i];
    }
}
void ConfigInputWidget::setMoveFromCommand(int command)
{
    //CHANNELNUMBER_ROLL=0, CHANNELNUMBER_PITCH=1, CHANNELNUMBER_YAW=2, CHANNELNUMBER_THROTTLE=3, CHANNELNUMBER_FLIGHTMODE=4, CHANNELNUMBER_ACCESSORY0=5, CHANNELNUMBER_ACCESSORY1=6, CHANNELNUMBER_ACCESSORY2=7 } ChannelNumberElem;
    if(command==ManualControlSettings::CHANNELNUMBER_ROLL)
    {
            setTxMovement(moveRightHorizontalStick);
    }
    else if(command==ManualControlSettings::CHANNELNUMBER_PITCH)
    {
        if(transmitterMode==mode2)
            setTxMovement(moveRightVerticalStick);
        else
            setTxMovement(moveLeftVerticalStick);
    }
    else if(command==ManualControlSettings::CHANNELNUMBER_YAW)
    {
            setTxMovement(moveLeftHorizontalStick);
    }
    else if(command==ManualControlSettings::CHANNELNUMBER_THROTTLE)
    {
        if(transmitterMode==mode2)
            setTxMovement(moveLeftVerticalStick);
        else
            setTxMovement(moveRightVerticalStick);
    }
    else if(command==ManualControlSettings::CHANNELNUMBER_FLIGHTMODE)
    {
        setTxMovement(moveFlightMode);
    }
    else if(command==ManualControlSettings::CHANNELNUMBER_ACCESSORY0)
    {
        setTxMovement(moveAccess0);
    }
    else if(command==ManualControlSettings::CHANNELNUMBER_ACCESSORY1)
    {
        setTxMovement(moveAccess1);
    }
    else if(command==ManualControlSettings::CHANNELNUMBER_ACCESSORY2)
    {
        setTxMovement(moveAccess2);
    }

}

void ConfigInputWidget::setTxMovement(txMovements movement)
{
    resetTxControls();
    switch(movement)
    {
    case moveLeftVerticalStick:
        movePos=0;
        growing=true;
        currentMovement=moveLeftVerticalStick;
        animate->start(100);
        break;
    case moveRightVerticalStick:
        movePos=0;
        growing=true;
        currentMovement=moveRightVerticalStick;
        animate->start(100);
        break;
    case moveLeftHorizontalStick:
        movePos=0;
        growing=true;
        currentMovement=moveLeftHorizontalStick;
        animate->start(100);
        break;
    case moveRightHorizontalStick:
        movePos=0;
        growing=true;
        currentMovement=moveRightHorizontalStick;
        animate->start(100);
        break;
    case moveAccess0:
        movePos=0;
        growing=true;
        currentMovement=moveAccess0;
        animate->start(100);
        break;
    case moveAccess1:
        movePos=0;
        growing=true;
        currentMovement=moveAccess1;
        animate->start(100);
        break;
    case moveAccess2:
        movePos=0;
        growing=true;
        currentMovement=moveAccess2;
        animate->start(100);
        break;
    case moveFlightMode:
        movePos=0;
        growing=true;
        currentMovement=moveFlightMode;
        animate->start(1000);
        break;
    case centerAll:
        movePos=0;
        currentMovement=centerAll;
        animate->start(1000);
        break;
    case moveAll:
        movePos=0;
        growing=true;
        currentMovement=moveAll;
        animate->start(50);
        break;
    case nothing:
        movePos=0;
        animate->stop();
        break;
    default:
        break;
    }
}

void ConfigInputWidget::moveTxControls()
{
    QTransform trans;
    QGraphicsItem * item;
    txMovementType move;
    int limitMax;
    int limitMin;
    static bool auxFlag=false;
    switch(currentMovement)
    {
    case moveLeftVerticalStick:
        item=m_txLeftStick;
        trans=m_txLeftStickOrig;
        limitMax=STICK_MAX_MOVE;
        limitMin=STICK_MIN_MOVE;
        move=vertical;
        break;
    case moveRightVerticalStick:
        item=m_txRightStick;
        trans=m_txRightStickOrig;
        limitMax=STICK_MAX_MOVE;
        limitMin=STICK_MIN_MOVE;
        move=vertical;
        break;
    case moveLeftHorizontalStick:
        item=m_txLeftStick;
        trans=m_txLeftStickOrig;
        limitMax=STICK_MAX_MOVE;
        limitMin=STICK_MIN_MOVE;
        move=horizontal;
        break;
    case moveRightHorizontalStick:
        item=m_txRightStick;
        trans=m_txRightStickOrig;
        limitMax=STICK_MAX_MOVE;
        limitMin=STICK_MIN_MOVE;
        move=horizontal;
        break;
    case moveAccess0:
        item=m_txAccess0;
        trans=m_txAccess0Orig;
        limitMax=ACCESS_MAX_MOVE;
        limitMin=ACCESS_MIN_MOVE;
        move=horizontal;
        break;
    case moveAccess1:
        item=m_txAccess1;
        trans=m_txAccess1Orig;
        limitMax=ACCESS_MAX_MOVE;
        limitMin=ACCESS_MIN_MOVE;
        move=horizontal;
        break;
    case moveAccess2:
        item=m_txAccess2;
        trans=m_txAccess2Orig;
        limitMax=ACCESS_MAX_MOVE;
        limitMin=ACCESS_MIN_MOVE;
        move=horizontal;
        break;
    case moveFlightMode:
        item=m_txFlightMode;
        move=jump;
        break;
    case centerAll:
        item=m_txArrows;
        move=jump;
        break;
    case moveAll:
        limitMax=STICK_MAX_MOVE;
        limitMin=STICK_MIN_MOVE;
        move=mix;
        break;
    default:
        break;
    }
    if(move==vertical)
        item->setTransform(trans.translate(0,movePos*10),false);
    else if(move==horizontal)
        item->setTransform(trans.translate(movePos*10,0),false);
    else if(move==jump)
    {
        if(item==m_txArrows)
        {
            m_txArrows->setVisible(!m_txArrows->isVisible());
        }
        else if(item==m_txFlightMode)
        {
            QGraphicsSvgItem * svg;
            svg=(QGraphicsSvgItem *)item;
            if (svg)
            {
                if(svg->elementId()=="flightModeCenter")
                {
                    if(growing)
                    {
                        svg->setElementId("flightModeRight");
                        m_txFlightMode->setTransform(m_txFlightModeROrig,false);
                    }
                    else
                    {
                        svg->setElementId("flightModeLeft");
                        m_txFlightMode->setTransform(m_txFlightModeLOrig,false);
                    }
                }
                else if(svg->elementId()=="flightModeRight")
                {
                    growing=false;
                    svg->setElementId("flightModeCenter");
                    m_txFlightMode->setTransform(m_txFlightModeCOrig,false);
                }
                else if(svg->elementId()=="flightModeLeft")
                {
                    growing=true;
                    svg->setElementId("flightModeCenter");
                    m_txFlightMode->setTransform(m_txFlightModeCOrig,false);
                }
            }
        }
    }
    else if(move==mix)
    {
        trans=m_txAccess0Orig;
        m_txAccess0->setTransform(trans.translate(movePos*10*ACCESS_MAX_MOVE/STICK_MAX_MOVE,0),false);
        trans=m_txAccess1Orig;
        m_txAccess1->setTransform(trans.translate(movePos*10*ACCESS_MAX_MOVE/STICK_MAX_MOVE,0),false);
        trans=m_txAccess2Orig;
        m_txAccess2->setTransform(trans.translate(movePos*10*ACCESS_MAX_MOVE/STICK_MAX_MOVE,0),false);

        if(auxFlag)
        {
            trans=m_txLeftStickOrig;
            m_txLeftStick->setTransform(trans.translate(0,movePos*10),false);
            trans=m_txRightStickOrig;
            m_txRightStick->setTransform(trans.translate(0,movePos*10),false);
        }
        else
        {
            trans=m_txLeftStickOrig;
            m_txLeftStick->setTransform(trans.translate(movePos*10,0),false);
            trans=m_txRightStickOrig;
            m_txRightStick->setTransform(trans.translate(movePos*10,0),false);
        }

        if(movePos==0)
        {
            m_txFlightMode->setElementId("flightModeCenter");
            m_txFlightMode->setTransform(m_txFlightModeCOrig,false);
        }
        else if(movePos==ACCESS_MAX_MOVE/2)
        {
            m_txFlightMode->setElementId("flightModeRight");
            m_txFlightMode->setTransform(m_txFlightModeROrig,false);
        }
        else if(movePos==ACCESS_MIN_MOVE/2)
        {
            m_txFlightMode->setElementId("flightModeLeft");
            m_txFlightMode->setTransform(m_txFlightModeLOrig,false);
        }
    }
    if(move==horizontal || move==vertical ||move==mix)
    {
        if(movePos==0 && growing)
            auxFlag=!auxFlag;
        if(growing)
            ++movePos;
        else
            --movePos;
        if(movePos>limitMax)
        {
            movePos=movePos-2;
            growing=false;
        }
        if(movePos<limitMin)
        {
            movePos=movePos+2;
            growing=true;
        }
    }
}

void ConfigInputWidget::moveSticks()
{
    QTransform trans;
    manualCommandData=manualCommandObj->getData();
    if(transmitterMode==mode2)
    {
        trans=m_txLeftStickOrig;
        m_txLeftStick->setTransform(trans.translate(manualCommandData.Yaw*STICK_MAX_MOVE*10,-manualCommandData.Throttle*STICK_MAX_MOVE*10),false);
        trans=m_txRightStickOrig;
        m_txRightStick->setTransform(trans.translate(manualCommandData.Roll*STICK_MAX_MOVE*10,manualCommandData.Pitch*STICK_MAX_MOVE*10),false);
    }
    else
    {
        trans=m_txRightStickOrig;
        m_txRightStick->setTransform(trans.translate(manualCommandData.Roll*STICK_MAX_MOVE*10,-manualCommandData.Throttle*STICK_MAX_MOVE*10),false);
        trans=m_txLeftStickOrig;
        m_txLeftStick->setTransform(trans.translate(manualCommandData.Yaw*STICK_MAX_MOVE*10,manualCommandData.Pitch*STICK_MAX_MOVE*10),false);
    }
}

void ConfigInputWidget::dimOtherControls(bool value)
{
    qreal opac;
    if(value)
        opac=0.1;
    else
        opac=1;
    m_txAccess0->setOpacity(opac);
    m_txAccess1->setOpacity(opac);
    m_txAccess2->setOpacity(opac);
    m_txFlightMode->setOpacity(opac);
}

void ConfigInputWidget::enableControls(bool enable)
{
    m_config->configurationWizard->setEnabled(enable);
    m_config->runCalibration->setEnabled(enable);

    ConfigTaskWidget::enableControls(enable);

}

void ConfigInputWidget::invertControls()
{
    manualSettingsData=manualSettingsObj->getData();
    foreach(QWidget * wd,extraWidgets)
    {
        QCheckBox * cb=qobject_cast<QCheckBox *>(wd);
        if(cb)
        {
            int index=manualSettingsObj->getFields().at(0)->getElementNames().indexOf(cb->text());
            if((cb->isChecked() && (manualSettingsData.ChannelMax[index]>manualSettingsData.ChannelMin[index])) ||
                    (!cb->isChecked() && (manualSettingsData.ChannelMax[index]<manualSettingsData.ChannelMin[index])))
            {
                qint16 aux;
                aux=manualSettingsData.ChannelMax[index];
                manualSettingsData.ChannelMax[index]=manualSettingsData.ChannelMin[index];
                manualSettingsData.ChannelMin[index]=aux;
            }
        }
    }
    manualSettingsObj->setData(manualSettingsData);
}
void ConfigInputWidget::moveFMSlider()
{
    ManualControlSettings::DataFields manualSettingsDataPriv = manualSettingsObj->getData();
    ManualControlCommand::DataFields manualCommandDataPriv=manualCommandObj->getData();
    uint chIndex = manualSettingsDataPriv.ChannelNumber[ManualControlSettings::CHANNELNUMBER_FLIGHTMODE];
    if (chIndex < 8) {
        float valueScaled;

        int chMin = manualSettingsDataPriv.ChannelMin[ManualControlSettings::CHANNELMIN_FLIGHTMODE];
        int chMax = manualSettingsDataPriv.ChannelMax[ManualControlSettings::CHANNELMAX_FLIGHTMODE];
        int chNeutral = manualSettingsDataPriv.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_FLIGHTMODE];

        int value = manualCommandDataPriv.Channel[chIndex];
        if ((chMax > chMin && value >= chNeutral) || (chMin > chMax && value <= chNeutral))
        {
            if (chMax != chNeutral)
                valueScaled = (float)(value - chNeutral) / (float)(chMax - chNeutral);
            else
                valueScaled = 0;
        }
        else
        {
            if (chMin != chNeutral)
                valueScaled = (float)(value - chNeutral) / (float)(chNeutral - chMin);
            else
                valueScaled = 0;
        }

        if(valueScaled < -(1.0 / 3.0))
            m_config->fmsSlider->setValue(-100);
        else if (valueScaled > (1.0/3.0))
            m_config->fmsSlider->setValue(100);
        else
            m_config->fmsSlider->setValue(0);
    }
}

void ConfigInputWidget::updateCalibration()
{
    bool changed = false;

    manualCommandData=manualCommandObj->getData();
    for(uint i=0;i<ManualControlSettings::CHANNELMAX_NUMELEM;++i)
    {
        if(manualSettingsData.ChannelMin[i]>manualCommandData.Channel[i])
            manualSettingsData.ChannelMin[i]=manualCommandData.Channel[i];
        if(manualSettingsData.ChannelMax[i]<manualCommandData.Channel[i])
            manualSettingsData.ChannelMax[i]=manualCommandData.Channel[i];
        manualSettingsData.ChannelNeutral[i] = manualCommandData.Channel[i];
    }

    manualSettingsObj->setData(manualSettingsData);
    manualSettingsObj->updated();
    settingsUpdated();
}

void ConfigInputWidget::settingsUpdated()
{
    manualSettingsData=manualSettingsObj->getData();
    Q_ASSERT(inputList.length() <= ManualControlSettings::CHANNELGROUPS_NUMELEM);

    for(int i = 0; i < inputList.length(); i++) {
        inputList[i]->ui->channelNeutral->setMaximum(manualSettingsData.ChannelMax[i]);
        inputList[i]->ui->channelNeutral->setMinimum(manualSettingsData.ChannelMin[i]);
        inputList[i]->ui->channelNeutral->setValue(manualSettingsData.ChannelNeutral[i]);
    }
}

void ConfigInputWidget::simpleCalibration(bool enable)
{
    if (enable) {
        QMessageBox msgBox;
        msgBox.setText(tr("Arming Settings are now set to Always Disarmed for your safety."));
        msgBox.setDetailedText(tr("You will have to reconfigure arming settings yourself afterwards."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();

        manualCommandData = manualCommandObj->getData();

        manualSettingsData=manualSettingsObj->getData();
        manualSettingsData.Arming=ManualControlSettings::ARMING_ALWAYSDISARMED;
        manualSettingsObj->setData(manualSettingsData);

        for (int i = 0; i < ManualControlCommand::CHANNEL_NUMELEM; i++) {
            manualSettingsData.ChannelMin[i] = manualCommandData.Channel[i];
            manualSettingsData.ChannelNeutral[i] = manualCommandData.Channel[i];
            manualSettingsData.ChannelMax[i] = manualCommandData.Channel[i];
        }

        UAVObject::Metadata mdata= manualCommandObj->getMetadata();
        mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
        mdata.flightTelemetryUpdatePeriod = 150;
        manualCommandObj->setMetadata(mdata);

        connect(manualCommandObj, SIGNAL(objectUnpacked(UAVObject*)), this, SLOT(updateCalibration()));
    } else {
        manualCommandData = manualCommandObj->getData();
        manualSettingsData = manualSettingsObj->getData();

        manualCommandObj->setMetadata(manualCommandObj->getDefaultMetadata());

        for (int i = 0; i < ManualControlCommand::CHANNEL_NUMELEM; i++)
            manualSettingsData.ChannelNeutral[i] = manualCommandData.Channel[i];

        // Force flight mode neutral to middle
        manualSettingsData.ChannelNeutral[ManualControlSettings::CHANNELNUMBER_FLIGHTMODE] =
                (manualSettingsData.ChannelMax[ManualControlSettings::CHANNELNUMBER_FLIGHTMODE] +
                manualSettingsData.ChannelMin[ManualControlSettings::CHANNELNUMBER_FLIGHTMODE]) / 2;

        // Force throttle to be near min
        manualSettingsData.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_THROTTLE]=
                manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_THROTTLE]+
                ((manualSettingsData.ChannelMax[ManualControlSettings::CHANNELMAX_THROTTLE]-
                  manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_THROTTLE])*0.02);

        manualSettingsObj->setData(manualSettingsData);

        disconnect(manualCommandObj, SIGNAL(objectUnpacked(UAVObject*)), this, SLOT(updateCalibration()));
    }
}
