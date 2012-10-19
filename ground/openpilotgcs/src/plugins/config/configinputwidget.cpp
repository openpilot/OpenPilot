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

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/generalsettings.h>

#define ACCESS_MIN_MOVE -3
#define ACCESS_MAX_MOVE 3
#define STICK_MIN_MOVE -8
#define STICK_MAX_MOVE 8

ConfigInputWidget::ConfigInputWidget(QWidget *parent) : 
    ConfigTaskWidget(parent),
    wizardStep(wizardNone),
    // not currently stored in the settings UAVO
    transmitterMode(mode2), 
    transmitterType(acro),  
    //
    loop(NULL),
    skipflag(false)
{
    manualCommandObj = ManualControlCommand::GetInstance(getObjectManager());
    manualSettingsObj = ManualControlSettings::GetInstance(getObjectManager());
    flightStatusObj = FlightStatus::GetInstance(getObjectManager());
    receiverActivityObj=ReceiverActivity::GetInstance(getObjectManager());
    m_config = new Ui_InputWidget();
    m_config->setupUi(this);
    
    addApplySaveButtons(m_config->saveRCInputToRAM,m_config->saveRCInputToSD);

    ExtensionSystem::PluginManager *pm=ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings * settings=pm->getObject<Core::Internal::GeneralSettings>();
    if(!settings->useExpertMode())
        m_config->saveRCInputToRAM->setVisible(false);
    
    addApplySaveButtons(m_config->saveRCInputToRAM,m_config->saveRCInputToSD);

	//Generate the rows of buttons in the input channel form GUI
    unsigned int index=0;
    foreach (QString name, manualSettingsObj->getField("ChannelNumber")->getElementNames())
    {
        Q_ASSERT(index < ManualControlSettings::CHANNELGROUPS_NUMELEM);
        inputChannelForm * inpForm=new inputChannelForm(this,index==0);
        m_config->channelSettings->layout()->addWidget(inpForm); //Add the row to the UI
        inpForm->setName(name);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelGroups",inpForm->ui->channelGroup,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelNumber",inpForm->ui->channelNumber,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelMin",inpForm->ui->channelMin,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelNeutral",inpForm->ui->channelNeutral,index);
        addUAVObjectToWidgetRelation("ManualControlSettings","ChannelMax",inpForm->ui->channelMax,index);
        ++index;
    }

    addUAVObjectToWidgetRelation("ManualControlSettings", "Deadband", m_config->deadband, 0, 0.01f);

    connect(m_config->configurationWizard,SIGNAL(clicked()),this,SLOT(goToWizard()));
    connect(m_config->stackedWidget,SIGNAL(currentChanged(int)),this,SLOT(disableWizardButton(int)));
    connect(m_config->runCalibration,SIGNAL(toggled(bool)),this, SLOT(simpleCalibration(bool)));

    connect(m_config->wzNext,SIGNAL(clicked()),this,SLOT(wzNext()));
    connect(m_config->wzCancel,SIGNAL(clicked()),this,SLOT(wzCancel()));
    connect(m_config->wzBack,SIGNAL(clicked()),this,SLOT(wzBack()));

    m_config->stackedWidget->setCurrentIndex(0);
    addUAVObjectToWidgetRelation("ManualControlSettings","FlightModePosition",m_config->fmsModePos1,0,1,true);
    addUAVObjectToWidgetRelation("ManualControlSettings","FlightModePosition",m_config->fmsModePos2,1,1,true);
    addUAVObjectToWidgetRelation("ManualControlSettings","FlightModePosition",m_config->fmsModePos3,2,1,true);
    addUAVObjectToWidgetRelation("ManualControlSettings","FlightModePosition",m_config->fmsModePos4,3,1,true);
    addUAVObjectToWidgetRelation("ManualControlSettings","FlightModePosition",m_config->fmsModePos5,4,1,true);
    addUAVObjectToWidgetRelation("ManualControlSettings","FlightModePosition",m_config->fmsModePos6,5,1,true);
    addUAVObjectToWidgetRelation("ManualControlSettings","FlightModeNumber",m_config->fmsPosNum);

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
    connect( ManualControlSettings::GetInstance(getObjectManager()),SIGNAL(objectUpdated(UAVObject*)),this,SLOT(updatePositionSlider()));
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
    if (QFile::exists(":/configgadget/images/TX2.svg") && m_renderer->load(QString(":/configgadget/images/TX2.svg")) && m_renderer->isValid())
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

    heliChannelOrder << ManualControlSettings::CHANNELGROUPS_COLLECTIVE <<
                        ManualControlSettings::CHANNELGROUPS_THROTTLE <<
                        ManualControlSettings::CHANNELGROUPS_ROLL <<
                        ManualControlSettings::CHANNELGROUPS_PITCH <<
                        ManualControlSettings::CHANNELGROUPS_YAW <<
                        ManualControlSettings::CHANNELGROUPS_FLIGHTMODE <<
                        ManualControlSettings::CHANNELGROUPS_ACCESSORY0 <<
                        ManualControlSettings::CHANNELGROUPS_ACCESSORY1 <<
                        ManualControlSettings::CHANNELGROUPS_ACCESSORY2;

    acroChannelOrder << ManualControlSettings::CHANNELGROUPS_THROTTLE <<
                        ManualControlSettings::CHANNELGROUPS_ROLL <<
                        ManualControlSettings::CHANNELGROUPS_PITCH <<
                        ManualControlSettings::CHANNELGROUPS_YAW <<
                        ManualControlSettings::CHANNELGROUPS_FLIGHTMODE <<
                        ManualControlSettings::CHANNELGROUPS_ACCESSORY0 <<
                        ManualControlSettings::CHANNELGROUPS_ACCESSORY1 <<
                        ManualControlSettings::CHANNELGROUPS_ACCESSORY2;
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
    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/x/04Cf", QUrl::StrictMode) );
}

void ConfigInputWidget::goToWizard()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Arming Settings are now set to 'Always Disarmed' for your safety."));
    msgBox.setDetailedText(tr("You will have to reconfigure the arming settings manually "
                              "when the wizard is finished. After the last step of the "
                              "wizard you will be taken to the Arming Settings screen."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();

    // Set correct tab visible before starting wizard.
    if(m_config->tabWidget->currentIndex() != 0) {
        m_config->tabWidget->setCurrentIndex(0);
    }
    
    // Stash current manual settings data in case the wizard is 
    // cancelled or the user proceeds far enough into the wizard such
    // that the UAVO is changed, but then backs out to the start and
    // chooses a different TX type (which could otherwise result in
    // unexpected TX channels being enabled)
    manualSettingsData=manualSettingsObj->getData();
    previousManualSettingsData = manualSettingsData;
    manualSettingsData.Arming=ManualControlSettings::ARMING_ALWAYSDISARMED;
    manualSettingsObj->setData(manualSettingsData);
    
    // start the wizard
    wizardSetUpStep(wizardWelcome);
    m_config->graphicsView->fitInView(m_txBackground, Qt::KeepAspectRatio);
}

void ConfigInputWidget::disableWizardButton(int value)
{
    if(value!=0)
        m_config->groupBox_3->setVisible(false);
    else
        m_config->groupBox_3->setVisible(true);
}

void ConfigInputWidget::wzCancel()
{
    dimOtherControls(false);
    manualCommandObj->setMetadata(manualCommandObj->getDefaultMetadata());
    m_config->stackedWidget->setCurrentIndex(0);

    if(wizardStep != wizardNone)
        wizardTearDownStep(wizardStep);
    wizardStep=wizardNone;
    m_config->stackedWidget->setCurrentIndex(0);

    // Load settings back from beginning of wizard
    manualSettingsObj->setData(previousManualSettingsData);
}

void ConfigInputWidget::wzNext()
{
    // In identify sticks mode the next button can indicate
    // channel advance
    if(wizardStep != wizardNone &&
            wizardStep != wizardIdentifySticks)
        wizardTearDownStep(wizardStep);

    // State transitions for next button
    switch(wizardStep) {
    case wizardWelcome:
        wizardSetUpStep(wizardChooseType);
        break;
    case wizardChooseType:
        wizardSetUpStep(wizardChooseMode);
        break;
    case wizardChooseMode:
        wizardSetUpStep(wizardIdentifySticks);
        break;
    case wizardIdentifySticks:
        nextChannel();
        if(currentChannelNum==-1) { // Gone through all channels
            wizardTearDownStep(wizardIdentifySticks);
            wizardSetUpStep(wizardIdentifyCenter);
        }
        break;
    case wizardIdentifyCenter:
        wizardSetUpStep(wizardIdentifyLimits);
        break;
    case wizardIdentifyLimits:
        wizardSetUpStep(wizardIdentifyInverted);
        break;
    case wizardIdentifyInverted:
        wizardSetUpStep(wizardFinish);
        break;
    case wizardFinish:
        wizardStep=wizardNone;
        // Leave setting the throttle neutral until the final Next press,
        // else the throttle scaling causes the graphical stick movement to not
        // match the tx stick
        manualSettingsData.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_THROTTLE]=
        manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_THROTTLE]+
        ((manualSettingsData.ChannelMax[ManualControlSettings::CHANNELMAX_THROTTLE]-
          manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_THROTTLE])*0.02);
        if((abs(manualSettingsData.ChannelMax[ManualControlSettings::CHANNELMAX_FLIGHTMODE] -
                manualSettingsData.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_FLIGHTMODE])<100) ||
           (abs(manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_FLIGHTMODE] -
                manualSettingsData.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_FLIGHTMODE])<100))
        {
            manualSettingsData.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_FLIGHTMODE]=
                manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_FLIGHTMODE]+
                (manualSettingsData.ChannelMax[ManualControlSettings::CHANNELMAX_FLIGHTMODE] -
                 manualSettingsData.ChannelMin[ManualControlSettings::CHANNELMIN_FLIGHTMODE])/2;
        }
        manualSettingsObj->setData(manualSettingsData);
        // move to Arming Settings tab
        m_config->stackedWidget->setCurrentIndex(0);
        m_config->tabWidget->setCurrentIndex(2);
        break;
    default:
        Q_ASSERT(0);
    }
}

void ConfigInputWidget::wzBack()
{
    if(wizardStep != wizardNone &&
            wizardStep != wizardIdentifySticks)
        wizardTearDownStep(wizardStep);

    // State transitions for next button
    switch(wizardStep) {
    case wizardChooseType:
        wizardSetUpStep(wizardWelcome);
        break;
    case wizardChooseMode:
        wizardSetUpStep(wizardChooseType);
        break;
    case wizardIdentifySticks:
        prevChannel();
        if(currentChannelNum == -1) {
            wizardTearDownStep(wizardIdentifySticks);
            wizardSetUpStep(wizardChooseMode);
        }
        break;
    case wizardIdentifyCenter:
        wizardSetUpStep(wizardIdentifySticks);
        break;
    case wizardIdentifyLimits:
        wizardSetUpStep(wizardIdentifyCenter);
        break;
    case wizardIdentifyInverted:
        wizardSetUpStep(wizardIdentifyLimits);
        break;
    case wizardFinish:
        wizardSetUpStep(wizardIdentifyInverted);
        break;
    default:
        Q_ASSERT(0);
    }
}

void ConfigInputWidget::wizardSetUpStep(enum wizardSteps step)
{
    m_config->wzText2->clear();
    
    switch(step) {
    case wizardWelcome:
        foreach(QPointer<QWidget> wd,extraWidgets)
        {
            if(!wd.isNull())
                delete wd;
        }
        extraWidgets.clear();
        m_config->graphicsView->setVisible(false);
        setTxMovement(nothing);
        m_config->wzText->setText(tr("Welcome to the inputs configuration wizard.\n\n"
                                     "Please follow the instructions on the screen and only move your controls when asked to.\n"
                                     "Make sure you already configured your hardware settings on the proper tab and restarted your board.\n\n"
                                     "You can press 'back' at any time to return to the previous screen or press 'Cancel' to quit the wizard.\n"));
        m_config->stackedWidget->setCurrentIndex(1);
        m_config->wzBack->setEnabled(false);
        break;
    case wizardChooseType:
    {
        m_config->graphicsView->setVisible(true);
        m_config->graphicsView->fitInView(m_txBackground, Qt::KeepAspectRatio);
        setTxMovement(nothing);
        m_config->wzText->setText(tr("Please choose your transmitter type:"));
        m_config->wzBack->setEnabled(true);
        QRadioButton * typeAcro=new QRadioButton(tr("Acro: normal transmitter for fixed-wing or quad"),this);
        QRadioButton * typeHeli=new QRadioButton(tr("Helicopter: has collective pitch and throttle input"),this);
        if (transmitterType == heli) {
            typeHeli->setChecked(true);
        }
        else {
            typeAcro->setChecked(true);
        }
        m_config->wzText2->setText(tr("If selecting the Helicopter option, please engage throttle hold now."));                                           

        extraWidgets.clear();
        extraWidgets.append(typeAcro);
        extraWidgets.append(typeHeli);
        m_config->radioButtonsLayout->layout()->addWidget(typeAcro);
        m_config->radioButtonsLayout->layout()->addWidget(typeHeli);
    }
        break;
    case wizardChooseMode:
    {
        m_config->wzBack->setEnabled(true);
        extraWidgets.clear();
        m_config->wzText->setText(tr("Please choose your transmitter mode:"));
        for (int i = 0; i <= mode4; ++i) {
            QString label;
            txMode mode = static_cast<txMode>(i);
            if (transmitterType == heli) {
                switch (mode) {
                    case mode1: label = tr("Mode 1: Fore/Aft Cyclic and Yaw on the left, Throttle/Collective and Left/Right Cyclic on the right"); break;
                    case mode2: label = tr("Mode 2: Throttle/Collective and Yaw on the left, Cyclic on the right"); break;
                    case mode3: label = tr("Mode 3: Cyclic on the left, Throttle/Collective and Yaw on the right"); break;
                    case mode4: label = tr("Mode 4: Throttle/Collective and Left/Right Cyclic on the left, Fore/Aft Cyclic and Yaw on the right"); break;
                    default:    Q_ASSERT(0); break;
                }                }
            else {
                switch (mode) {
                    case mode1: label = tr("Mode 1: Elevator and Rudder on the left, Throttle and Ailerons on the right"); break;
                    case mode2: label = tr("Mode 2: Throttle and Rudder on the left, Elevator and Ailerons on the right"); break;
                    case mode3: label = tr("Mode 3: Elevator and Ailerons on the left, Throttle and Rudder on the right"); break;
                    case mode4: label = tr("Mode 4: Throttle and Ailerons on the left, Elevator and Rudder on the right"); break;
                    default:    Q_ASSERT(0); break;
                }            
                m_config->wzText2->setText(tr("For a Quad: Elevator is Pitch, Ailerons are Roll, and Rudder is Yaw."));
            }
            QRadioButton * modeButton = new QRadioButton(label, this);
            if (transmitterMode == mode) {
                modeButton->setChecked(true);
            }
            extraWidgets.append(modeButton);
            m_config->radioButtonsLayout->layout()->addWidget(modeButton);
        }
    }
        break;
    case wizardIdentifySticks:
        usedChannels.clear();
        currentChannelNum=-1;
        nextChannel();
        manualSettingsData=manualSettingsObj->getData();
        connect(receiverActivityObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyControls()));
        m_config->wzNext->setEnabled(false);
        break;
    case wizardIdentifyCenter:
        setTxMovement(centerAll);
        m_config->wzText->setText(QString(tr("Please center all controls and trims and press Next when ready.\n\n"
                                             "If your FlightMode switch has only two positions, leave it in either position.")));
        break;
    case wizardIdentifyLimits:
    {
        accessoryDesiredObj0 = AccessoryDesired::GetInstance(getObjectManager(),0);
        accessoryDesiredObj1 = AccessoryDesired::GetInstance(getObjectManager(),1);
        accessoryDesiredObj2 = AccessoryDesired::GetInstance(getObjectManager(),2);
        setTxMovement(nothing);
        m_config->wzText->setText(QString(tr("Please move all controls to their maximum extents on both directions.\n\nPress Next when ready.")));
        fastMdata();
        manualSettingsData=manualSettingsObj->getData();
        for(uint i=0;i<ManualControlSettings::CHANNELMAX_NUMELEM;++i)
        {
            // Preserve the inverted status
            if(manualSettingsData.ChannelMin[i] <= manualSettingsData.ChannelMax[i]) {
                manualSettingsData.ChannelMin[i]=manualSettingsData.ChannelNeutral[i];
                manualSettingsData.ChannelMax[i]=manualSettingsData.ChannelNeutral[i];
            } else {
                // Make this detect as still inverted
                manualSettingsData.ChannelMin[i]=manualSettingsData.ChannelNeutral[i] + 1;
                manualSettingsData.ChannelMax[i]=manualSettingsData.ChannelNeutral[i];
            }
        }
        connect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyLimits()));
        connect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        connect(flightStatusObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        connect(accessoryDesiredObj0, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
    }
        break;
    case wizardIdentifyInverted:
        dimOtherControls(true);
        setTxMovement(nothing);
        extraWidgets.clear();
        for (int index = 0; index < manualSettingsObj->getField("ChannelMax")->getElementNames().length(); index++)
        {
            QString name = manualSettingsObj->getField("ChannelMax")->getElementNames().at(index);
            if(!name.contains("Access") &&  !name.contains("Flight") && 
               (!name.contains("Collective") || transmitterType == heli))
            {
                QCheckBox * cb=new QCheckBox(name,this);
                // Make sure checked status matches current one
                cb->setChecked(manualSettingsData.ChannelMax[index] < manualSettingsData.ChannelMin[index]);
                dynamic_cast<QGridLayout*>(m_config->checkBoxesLayout->layout())->addWidget(cb, extraWidgets.size()/4, extraWidgets.size()%4);
                extraWidgets.append(cb);
                connect(cb,SIGNAL(toggled(bool)),this,SLOT(invertControls()));
            }
        }
        connect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        m_config->wzText->setText(QString(tr("Please check the picture below and correct all the sticks which show an inverted movement. Press Next when ready.")));
        fastMdata();
        break;
    case wizardFinish:
        dimOtherControls(false);
        connect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        connect(flightStatusObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        connect(accessoryDesiredObj0, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        m_config->wzText->setText(QString(tr("You have completed this wizard, please check below if the picture mimics your sticks movement.\n\n"
                                             "IMPORTANT: These new settings have not been saved to the board yet. After pressing Next you will go to the Arming Settings "
                                             "tab where you can set your desired arming sequence and save the configuration.")));
        fastMdata();
        break;
    default:
        Q_ASSERT(0);
    }
    wizardStep = step;
}

void ConfigInputWidget::wizardTearDownStep(enum wizardSteps step)
{
    QRadioButton * mode, * type;
    Q_ASSERT(step == wizardStep);
    switch(step) {
    case wizardWelcome:
        break;
     case wizardChooseType:
        type=qobject_cast<QRadioButton *>(extraWidgets.at(0));
        if(type->isChecked())
            transmitterType=acro;
        else
            transmitterType=heli;
        delete extraWidgets.at(0);
        delete extraWidgets.at(1);
        extraWidgets.clear();
        break;
   case wizardChooseMode:
        for (int i = mode1; i <= mode4; ++i) { 
            mode=qobject_cast<QRadioButton *>(extraWidgets.first());
            if(mode->isChecked()) {
                transmitterMode=static_cast<txMode>(i);
            }
            delete mode;
            extraWidgets.removeFirst();
        }
        break;
    case wizardIdentifySticks:
        disconnect(receiverActivityObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyControls()));
        m_config->wzNext->setEnabled(true);
        setTxMovement(nothing);
        break;
    case wizardIdentifyCenter:
        manualCommandData=manualCommandObj->getData();
        manualSettingsData=manualSettingsObj->getData();
        for(unsigned int i=0;i<ManualControlCommand::CHANNEL_NUMELEM;++i)
        {
            manualSettingsData.ChannelNeutral[i]=manualCommandData.Channel[i];
        }
        manualSettingsObj->setData(manualSettingsData);
        setTxMovement(nothing);
        break;
    case wizardIdentifyLimits:
        disconnect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(identifyLimits()));
        disconnect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        disconnect(flightStatusObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        disconnect(accessoryDesiredObj0, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        manualSettingsObj->setData(manualSettingsData);
        restoreMdata();
        setTxMovement(nothing);
        break;
    case wizardIdentifyInverted:
        dimOtherControls(false);
        foreach(QWidget * wd,extraWidgets)
        {
            QCheckBox * cb=qobject_cast<QCheckBox *>(wd);
            if(cb)
            {
                disconnect(cb,SIGNAL(toggled(bool)),this,SLOT(invertControls()));
                delete cb;
            }
        }
        extraWidgets.clear();
        disconnect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        restoreMdata();
        break;
    case wizardFinish:
        dimOtherControls(false);
        setTxMovement(nothing);
        disconnect(manualCommandObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        disconnect(flightStatusObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        disconnect(accessoryDesiredObj0, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(moveSticks()));
        restoreMdata();
        break;
    default:
        Q_ASSERT(0);
    }
}

/**
  * Set manual control command to fast updates
  */
void ConfigInputWidget::fastMdata()
{
    manualControlMdata = manualCommandObj->getMetadata();
    UAVObject::Metadata mdata = manualControlMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 150;
    manualCommandObj->setMetadata(mdata);
}

/**
  * Restore previous update settings for manual control data
  */
void ConfigInputWidget::restoreMdata()
{
    manualCommandObj->setMetadata(manualControlMdata);
}

/**
  * Set the display to indicate which channel the person should move
  */
void ConfigInputWidget::setChannel(int newChan)
{
    if(newChan == ManualControlSettings::CHANNELGROUPS_COLLECTIVE)
        m_config->wzText->setText(QString(tr("Please enable throttle hold mode.\n\nMove the Collective Pitch stick.")));
    else if (newChan == ManualControlSettings::CHANNELGROUPS_FLIGHTMODE)
        m_config->wzText->setText(QString(tr("Please toggle the Flight Mode switch.\n\nFor switches you may have to repeat this rapidly.")));
    else if((transmitterType == heli) && (newChan == ManualControlSettings::CHANNELGROUPS_THROTTLE))
        m_config->wzText->setText(QString(tr("Please disable throttle hold mode.\n\nMove the Throttle stick.")));
    else
        m_config->wzText->setText(QString(tr("Please move each control one at a time according to the instructions and picture below.\n\n"
                                 "Move the %1 stick.")).arg(manualSettingsObj->getField("ChannelGroups")->getElementNames().at(newChan)));

    if(manualSettingsObj->getField("ChannelGroups")->getElementNames().at(newChan).contains("Accessory") ||
       manualSettingsObj->getField("ChannelGroups")->getElementNames().at(newChan).contains("FlightMode")) {
        m_config->wzNext->setEnabled(true);
        m_config->wzText->setText(m_config->wzText->text() + tr(" Alternatively, click Next to skip this channel."));
    } else
        m_config->wzNext->setEnabled(false);

    setMoveFromCommand(newChan);

    currentChannelNum = newChan;
    channelDetected = false;
}

/**
  * Unfortunately order of channel should be different in different conditions.  Selects
  * next channel based on heli or acro mode
  */
void ConfigInputWidget::nextChannel()
{
    QList <int> order = (transmitterType == heli) ? heliChannelOrder : acroChannelOrder;

    if(currentChannelNum == -1) {
        setChannel(order[0]);
        return;
    }
    for (int i = 0; i < order.length() - 1; i++) {
        if(order[i] == currentChannelNum) {
            setChannel(order[i+1]);
            return;
        }
    }
    currentChannelNum = -1; // hit end of list
}

/**
  * Unfortunately order of channel should be different in different conditions.  Selects
  * previous channel based on heli or acro mode
  */
void ConfigInputWidget::prevChannel()
{
    QList <int> order = transmitterType == heli ? heliChannelOrder : acroChannelOrder;

    // No previous from unset channel or next state
    if(currentChannelNum == -1)
        return;

    for (int i = 1; i < order.length(); i++) {
        if(order[i] == currentChannelNum) {
            if (!usedChannels.isEmpty() && 
                usedChannels.back().channelIndex == currentChannelNum) {
                usedChannels.removeLast();
            }
            setChannel(order[i-1]);
            return;
        }
    }
    currentChannelNum = -1; // hit end of list
}

void ConfigInputWidget::identifyControls()
{
    static int debounce=0;

    receiverActivityData=receiverActivityObj->getData();
    if(receiverActivityData.ActiveChannel==255)
        return;
    if(channelDetected)
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
        lastChannel.channelIndex = currentChannelNum; 
        if(!usedChannels.contains(lastChannel) && debounce>1)
        {
            channelDetected = true;
            debounce=0;
            usedChannels.append(lastChannel);
            manualSettingsData=manualSettingsObj->getData();
            manualSettingsData.ChannelGroups[currentChannelNum]=currentChannel.group;
            manualSettingsData.ChannelNumber[currentChannelNum]=currentChannel.number;
            manualSettingsObj->setData(manualSettingsData);
        }
        else
            return;
    }

    //m_config->wzText->clear();
    setTxMovement(nothing);

    QTimer::singleShot(2500, this, SLOT(wzNext()));
}

void ConfigInputWidget::identifyLimits()
{
    manualCommandData=manualCommandObj->getData();
    for(uint i=0;i<ManualControlSettings::CHANNELMAX_NUMELEM;++i)
    {
        if(manualSettingsData.ChannelMin[i] <= manualSettingsData.ChannelMax[i]) {
            // Non inverted channel
            if(manualSettingsData.ChannelMin[i]>manualCommandData.Channel[i])
                manualSettingsData.ChannelMin[i]=manualCommandData.Channel[i];
            if(manualSettingsData.ChannelMax[i]<manualCommandData.Channel[i])
                manualSettingsData.ChannelMax[i]=manualCommandData.Channel[i];
        } else {
            // Inverted channel
            if(manualSettingsData.ChannelMax[i]>manualCommandData.Channel[i])
                manualSettingsData.ChannelMax[i]=manualCommandData.Channel[i];
            if(manualSettingsData.ChannelMin[i]<manualCommandData.Channel[i])
                manualSettingsData.ChannelMin[i]=manualCommandData.Channel[i];
        }
    }
    manualSettingsObj->setData(manualSettingsData);
}
void ConfigInputWidget::setMoveFromCommand(int command)
{
    // ManualControlSettings::ChannelNumberElem:
    //    CHANNELNUMBER_ROLL=0, 
    //    CHANNELNUMBER_PITCH=1, 
    //    CHANNELNUMBER_YAW=2, 
    //    CHANNELNUMBER_THROTTLE=3, 
    //    CHANNELNUMBER_FLIGHTMODE=4, 
    //    CHANNELNUMBER_ACCESSORY0=5, 
    //    CHANNELNUMBER_ACCESSORY1=6, 
    //    CHANNELNUMBER_ACCESSORY2=7 
 
    txMovements movement;
 
    switch (command) {
        case ManualControlSettings::CHANNELNUMBER_ROLL:
            movement = ((transmitterMode == mode3 || transmitterMode == mode4) ? 
                        moveLeftHorizontalStick: moveRightHorizontalStick);
            break;
        case ManualControlSettings::CHANNELNUMBER_PITCH:
            movement = (transmitterMode == mode1 || transmitterMode == mode3) ? 
                        moveLeftVerticalStick: moveRightVerticalStick;
            break;
        case ManualControlSettings::CHANNELNUMBER_YAW:
            movement = ((transmitterMode == mode1 || transmitterMode == mode2) ? 
                        moveLeftHorizontalStick: moveRightHorizontalStick);
            break;
        case ManualControlSettings::CHANNELNUMBER_THROTTLE:
            movement = (transmitterMode == mode2 || transmitterMode == mode4) ? 
                        moveLeftVerticalStick: moveRightVerticalStick;
            break;
        case ManualControlSettings::CHANNELNUMBER_COLLECTIVE:
            movement = (transmitterMode == mode2 || transmitterMode == mode4) ? 
                        moveLeftVerticalStick: moveRightVerticalStick;
            break;
        case ManualControlSettings::CHANNELNUMBER_FLIGHTMODE:
            movement = moveFlightMode;
            break;
        case ManualControlSettings::CHANNELNUMBER_ACCESSORY0:
            movement = moveAccess0;
            break;   
        case ManualControlSettings::CHANNELNUMBER_ACCESSORY1:
            movement = moveAccess1;
            break;   
        case ManualControlSettings::CHANNELNUMBER_ACCESSORY2:
            movement = moveAccess2;
            break;
        default:
            Q_ASSERT(0);
            break;
    }     
    setTxMovement(movement);
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
        Q_ASSERT(0);
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
    flightStatusData=flightStatusObj->getData();
    accessoryDesiredData0=accessoryDesiredObj0->getData();
    accessoryDesiredData1=accessoryDesiredObj1->getData();
    accessoryDesiredData2=accessoryDesiredObj2->getData();

    switch (transmitterMode) {
        case mode1: 
            trans=m_txLeftStickOrig;
            m_txLeftStick->setTransform(trans.translate(manualCommandData.Yaw*STICK_MAX_MOVE*10,manualCommandData.Pitch*STICK_MAX_MOVE*10),false);
            trans=m_txRightStickOrig;
            m_txRightStick->setTransform(trans.translate(manualCommandData.Roll*STICK_MAX_MOVE*10,-manualCommandData.Throttle*STICK_MAX_MOVE*10),false);
            break;
        case mode2: 
            trans=m_txLeftStickOrig;
            m_txLeftStick->setTransform(trans.translate(manualCommandData.Yaw*STICK_MAX_MOVE*10,-manualCommandData.Throttle*STICK_MAX_MOVE*10),false);
            trans=m_txRightStickOrig;
            m_txRightStick->setTransform(trans.translate(manualCommandData.Roll*STICK_MAX_MOVE*10,manualCommandData.Pitch*STICK_MAX_MOVE*10),false);
            break;
        case mode3:
            trans=m_txLeftStickOrig;
            m_txLeftStick->setTransform(trans.translate(manualCommandData.Roll*STICK_MAX_MOVE*10,manualCommandData.Pitch*STICK_MAX_MOVE*10),false);
            trans=m_txRightStickOrig;
            m_txRightStick->setTransform(trans.translate(manualCommandData.Yaw*STICK_MAX_MOVE*10,-manualCommandData.Throttle*STICK_MAX_MOVE*10),false);
            break;
        case mode4:
            trans=m_txLeftStickOrig;
            m_txLeftStick->setTransform(trans.translate(manualCommandData.Roll*STICK_MAX_MOVE*10,-manualCommandData.Throttle*STICK_MAX_MOVE*10),false);
            trans=m_txRightStickOrig;
            m_txRightStick->setTransform(trans.translate(manualCommandData.Yaw*STICK_MAX_MOVE*10,manualCommandData.Pitch*STICK_MAX_MOVE*10),false);
            break;
        default:
            Q_ASSERT(0);
            break;
    }
    if(flightStatusData.FlightMode==manualSettingsData.FlightModePosition[0])
    {
        m_txFlightMode->setElementId("flightModeLeft");
        m_txFlightMode->setTransform(m_txFlightModeLOrig,false);
    }
    else if (flightStatusData.FlightMode==manualSettingsData.FlightModePosition[1])
    {
        m_txFlightMode->setElementId("flightModeCenter");
        m_txFlightMode->setTransform(m_txFlightModeCOrig,false);
    }
    else if (flightStatusData.FlightMode==manualSettingsData.FlightModePosition[2])
    {
        m_txFlightMode->setElementId("flightModeRight");
        m_txFlightMode->setTransform(m_txFlightModeROrig,false);
    }
    m_txAccess0->setTransform(QTransform(m_txAccess0Orig).translate(accessoryDesiredData0.AccessoryVal*ACCESS_MAX_MOVE*10,0),false);
    m_txAccess1->setTransform(QTransform(m_txAccess1Orig).translate(accessoryDesiredData1.AccessoryVal*ACCESS_MAX_MOVE*10,0),false);
    m_txAccess2->setTransform(QTransform(m_txAccess2Orig).translate(accessoryDesiredData2.AccessoryVal*ACCESS_MAX_MOVE*10,0),false);
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
            int index = manualSettingsObj->getField("ChannelNumber")->getElementNames().indexOf(cb->text());
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
    ManualControlCommand::DataFields manualCommandDataPriv = manualCommandObj->getData();

    float valueScaled;
    int chMin = manualSettingsDataPriv.ChannelMin[ManualControlSettings::CHANNELMIN_FLIGHTMODE];
    int chMax = manualSettingsDataPriv.ChannelMax[ManualControlSettings::CHANNELMAX_FLIGHTMODE];
    int chNeutral = manualSettingsDataPriv.ChannelNeutral[ManualControlSettings::CHANNELNEUTRAL_FLIGHTMODE];

    int value = manualCommandDataPriv.Channel[ManualControlSettings::CHANNELMIN_FLIGHTMODE];
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

    // Bound and scale FlightMode from [-1..+1] to [0..1] range
    if (valueScaled < -1.0)
        valueScaled = -1.0;
    else
    if (valueScaled >  1.0)
        valueScaled =  1.0;

    // Convert flightMode value into the switch position in the range [0..N-1]
    // This uses the same optimized computation as flight code to be consistent
    uint8_t pos = ((int16_t)(valueScaled * 256) + 256) * manualSettingsDataPriv.FlightModeNumber >> 9;
    if (pos >= manualSettingsDataPriv.FlightModeNumber)
        pos = manualSettingsDataPriv.FlightModeNumber - 1;
    m_config->fmsSlider->setValue(pos);
}

void ConfigInputWidget::updatePositionSlider()
{
    ManualControlSettings::DataFields manualSettingsDataPriv = manualSettingsObj->getData();

    switch(manualSettingsDataPriv.FlightModeNumber) {
    default:
    case 6:
        m_config->fmsModePos6->setEnabled(true);
	// pass through
    case 5:
        m_config->fmsModePos5->setEnabled(true);
	// pass through
    case 4:
        m_config->fmsModePos4->setEnabled(true);
	// pass through
    case 3:
        m_config->fmsModePos3->setEnabled(true);
	// pass through
    case 2:
        m_config->fmsModePos2->setEnabled(true);
	// pass through
    case 1:
        m_config->fmsModePos1->setEnabled(true);
	// pass through
    case 0:
	break;
    }

    switch(manualSettingsDataPriv.FlightModeNumber) {
    case 0:
        m_config->fmsModePos1->setEnabled(false);
	// pass through
    case 1:
        m_config->fmsModePos2->setEnabled(false);
	// pass through
    case 2:
        m_config->fmsModePos3->setEnabled(false);
	// pass through
    case 3:
        m_config->fmsModePos4->setEnabled(false);
	// pass through
    case 4:
        m_config->fmsModePos5->setEnabled(false);
	// pass through
    case 5:
        m_config->fmsModePos6->setEnabled(false);
	// pass through
    case 6:
    default:
	break;
    }
}

void ConfigInputWidget::updateCalibration()
{
    manualCommandData=manualCommandObj->getData();
    for(uint i=0;i<ManualControlSettings::CHANNELMAX_NUMELEM;++i)
    {
        if((!reverse[i] && manualSettingsData.ChannelMin[i]>manualCommandData.Channel[i]) ||
                (reverse[i] && manualSettingsData.ChannelMin[i]<manualCommandData.Channel[i]))
            manualSettingsData.ChannelMin[i]=manualCommandData.Channel[i];
        if((!reverse[i] && manualSettingsData.ChannelMax[i]<manualCommandData.Channel[i]) ||
                (reverse[i] && manualSettingsData.ChannelMax[i]>manualCommandData.Channel[i]))
            manualSettingsData.ChannelMax[i]=manualCommandData.Channel[i];
        manualSettingsData.ChannelNeutral[i] = manualCommandData.Channel[i];
    }

    manualSettingsObj->setData(manualSettingsData);
    manualSettingsObj->updated();
}

void ConfigInputWidget::simpleCalibration(bool enable)
{
    if (enable) {
        m_config->configurationWizard->setEnabled(false);

        QMessageBox msgBox;
        msgBox.setText(tr("Arming Settings are now set to 'Always Disarmed' for your safety."));
        msgBox.setDetailedText(tr("You will have to reconfigure the arming settings manually when the wizard is finished."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();

        manualCommandData = manualCommandObj->getData();

        manualSettingsData=manualSettingsObj->getData();
        manualSettingsData.Arming=ManualControlSettings::ARMING_ALWAYSDISARMED;
        manualSettingsObj->setData(manualSettingsData);

        for (unsigned int i = 0; i < ManualControlCommand::CHANNEL_NUMELEM; i++) {
            reverse[i] = manualSettingsData.ChannelMax[i] < manualSettingsData.ChannelMin[i];
            manualSettingsData.ChannelMin[i] = manualCommandData.Channel[i];
            manualSettingsData.ChannelNeutral[i] = manualCommandData.Channel[i];
            manualSettingsData.ChannelMax[i] = manualCommandData.Channel[i];
        }

        fastMdata();

        connect(manualCommandObj, SIGNAL(objectUnpacked(UAVObject*)), this, SLOT(updateCalibration()));
    } else {
        m_config->configurationWizard->setEnabled(true);

        manualCommandData = manualCommandObj->getData();
        manualSettingsData = manualSettingsObj->getData();

        restoreMdata();

        for (unsigned int i = 0; i < ManualControlCommand::CHANNEL_NUMELEM; i++)
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
