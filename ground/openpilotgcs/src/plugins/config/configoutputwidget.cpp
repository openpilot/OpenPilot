/**
 ******************************************************************************
 *
 * @file       configoutputwidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Servo output configuration panel for the config gadget
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

#include "configoutputwidget.h"
#include "outputchannelform.h"

#include "uavtalk/telemetrymanager.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include "actuatorcommand.h"
#include "actuatorsettings.h"
#include "systemalarms.h"
#include "uavsettingsimportexport/uavsettingsimportexportfactory.h"

ConfigOutputWidget::ConfigOutputWidget(QWidget *parent) : ConfigTaskWidget(parent),wasItMe(false)
{
    m_config = new Ui_OutputWidget();
    m_config->setupUi(this);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    setupButtons(m_config->saveRCOutputToRAM,m_config->saveRCOutputToSD);
    addUAVObject("ActuatorSettings");

    UAVSettingsImportExportFactory * importexportplugin =  pm->getObject<UAVSettingsImportExportFactory>();
    connect(importexportplugin,SIGNAL(importAboutToBegin()),this,SLOT(stopTests()));

    setupButtons(m_config->saveRCOutputToRAM,m_config->saveRCOutputToSD);
        addUAVObject("ActuatorSettings");

    // NOTE: we have channel indices from 0 to 9, but the convention for OP is Channel 1 to Channel 10.
    // Register for ActuatorSettings changes:
    for (unsigned int i = 0; i < ActuatorCommand::CHANNEL_NUMELEM; i++)
    {
        OutputChannelForm *form = new OutputChannelForm(i, this, i==0);
        connect(m_config->channelOutTest, SIGNAL(toggled(bool)),
                form, SLOT(enableChannelTest(bool)));
        connect(form, SIGNAL(channelChanged(int,int)),
                this, SLOT(sendChannelTest(int,int)));
        m_config->channelLayout->addWidget(form);
    }

    connect(m_config->channelOutTest, SIGNAL(toggled(bool)), this, SLOT(runChannelTests(bool)));

    refreshWidgetsValues();

    firstUpdate = true;

    connect(m_config->spinningArmed, SIGNAL(toggled(bool)), this, SLOT(setSpinningArmed(bool)));

    // Connect the help button
    connect(m_config->outputHelp, SIGNAL(clicked()), this, SLOT(openHelp()));
    addWidget(m_config->outputRate3);
    addWidget(m_config->outputRate2);
    addWidget(m_config->outputRate1);

    addWidget(m_config->spinningArmed);

    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVObject* obj = objManager->getObject(QString("ActuatorCommand"));
    if(obj->getMetadata().gcsTelemetryUpdateMode == UAVObject::UPDATEMODE_ONCHANGE)
        this->setEnabled(false);
    connect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(disableIfNotMe(UAVObject*)));
}
void ConfigOutputWidget::enableControls(bool enable)
{
    ConfigTaskWidget::enableControls(enable);
    if(!enable)
        m_config->channelOutTest->setChecked(false);
    m_config->channelOutTest->setEnabled(enable);
}

ConfigOutputWidget::~ConfigOutputWidget()
{
   // Do nothing
}


// ************************************

/**
  Toggles the channel testing mode by making the GCS take over
  the ActuatorCommand objects
  */
void ConfigOutputWidget::runChannelTests(bool state)
{
    qDebug()<<"configoutputwidget runChannelTests"<<state;
    SystemAlarms * systemAlarmsObj = SystemAlarms::GetInstance(getObjectManager());
    SystemAlarms::DataFields systemAlarms = systemAlarmsObj->getData();

    if(state && systemAlarms.Alarm[SystemAlarms::ALARM_ACTUATOR] != SystemAlarms::ALARM_OK) {
        QMessageBox mbox;
        mbox.setText(QString(tr("The actuator module is in an error state.  This can also occur because there are no inputs.  Please fix these before testing outputs.")));
        mbox.setStandardButtons(QMessageBox::Ok);
        mbox.exec();

        // Unfortunately must cache this since callback will reoccur
        accInitialData = ActuatorCommand::GetInstance(getObjectManager())->getMetadata();

        m_config->channelOutTest->setChecked(false);
        return;
    }

    // Confirm this is definitely what they want
    if(state) {
        QMessageBox mbox;
        mbox.setText(QString(tr("This option will start your motors by the amount selected on the sliders regardless of transmitter.  It is recommended to remove any blades from motors.  Are you sure you want to do this?")));
        mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int retval = mbox.exec();
        if(retval != QMessageBox::Yes) {
            state = false;
            qDebug() << "Cancelled";
            m_config->channelOutTest->setChecked(false);
            return;
        }
    }

    ActuatorCommand * obj = ActuatorCommand::GetInstance(getObjectManager());
    UAVObject::Metadata mdata = obj->getMetadata();
    if (state)
    {
        wasItMe=true;
        accInitialData = mdata;
        mdata.flightAccess = UAVObject::ACCESS_READONLY;
        mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
        mdata.gcsTelemetryAcked = false;
        mdata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
        mdata.gcsTelemetryUpdatePeriod = 100;
    }
    else
    {
        wasItMe=false;
        mdata = accInitialData; // Restore metadata
    }
    obj->setMetadata(mdata);
    obj->updated();

}

OutputChannelForm* ConfigOutputWidget::getOutputChannelForm(const int index) const
{
    QList<OutputChannelForm*> outputChannelForms = findChildren<OutputChannelForm*>();
    foreach(OutputChannelForm *outputChannelForm, outputChannelForms)
    {
        if( outputChannelForm->index() == index)
            return outputChannelForm;
    }

    // no OutputChannelForm found with given index
    return NULL;
}

/**
  * Set the label for a channel output assignement
  */
void ConfigOutputWidget::assignOutputChannel(UAVDataObject *obj, QString str)
{
    //FIXME: use signal/ slot approach
    UAVObjectField* field = obj->getField(str);
    QStringList options = field->getOptions();
    int index = options.indexOf(field->getValue().toString());

    OutputChannelForm *outputChannelForm = getOutputChannelForm(index);
    if(outputChannelForm)
        outputChannelForm->setAssignment(str);
}

/**
  * Set the "Spin motors at neutral when armed" flag in ActuatorSettings
  */
void ConfigOutputWidget::setSpinningArmed(bool val)
{
    ActuatorSettings *actuatorSettings = ActuatorSettings::GetInstance(getObjectManager());
    Q_ASSERT(actuatorSettings);
    ActuatorSettings::DataFields actuatorSettingsData = actuatorSettings->getData();

    if(val)
        actuatorSettingsData.MotorsSpinWhileArmed = ActuatorSettings::MOTORSSPINWHILEARMED_TRUE;
    else
        actuatorSettingsData.MotorsSpinWhileArmed = ActuatorSettings::MOTORSSPINWHILEARMED_FALSE;

    // Apply settings
    actuatorSettings->setData(actuatorSettingsData);
}

/**
  Sends the channel value to the UAV to move the servo.
  Returns immediately if we are not in testing mode
  */
void ConfigOutputWidget::sendChannelTest(int index, int value)
{
    if (!m_config->channelOutTest->isChecked())
        return;

    if(index < 0 || (unsigned)index >= ActuatorCommand::CHANNEL_NUMELEM)
        return;

    ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(getObjectManager());
    Q_ASSERT(actuatorCommand);
    ActuatorCommand::DataFields actuatorCommandFields = actuatorCommand->getData();
    actuatorCommandFields.Channel[index] = value;
    actuatorCommand->setData(actuatorCommandFields);
}



/********************************
  *  Output settings
  *******************************/

/**
  Request the current config from the board (RC Output)
  */
void ConfigOutputWidget::refreshWidgetsValues()
{
    bool dirty=isDirty();

    // Reset all channel assignements:
    QList<OutputChannelForm*> outputChannelForms = findChildren<OutputChannelForm*>();
    foreach(OutputChannelForm *outputChannelForm, outputChannelForms)
    {
        outputChannelForm->setAssignment("-");
    }

    // FIXME: Use static accessor method for retrieving channel assignments
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager);

   // Get the channel assignements:
    UAVDataObject * obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        if (field->getUnits().contains("channel")) {
            assignOutputChannel(obj,field->getName());
        }
    }

    ActuatorSettings *actuatorSettings = ActuatorSettings::GetInstance(getObjectManager());
    Q_ASSERT(actuatorSettings);
    ActuatorSettings::DataFields actuatorSettingsData = actuatorSettings->getData();

    // Get the SpinWhileArmed setting
    m_config->spinningArmed->setChecked(actuatorSettingsData.MotorsSpinWhileArmed == ActuatorSettings::MOTORSSPINWHILEARMED_TRUE);

    // Get Output rates for both banks
    m_config->outputRate1->setValue(actuatorSettingsData.ChannelUpdateFreq[0]);
    m_config->outputRate2->setValue(actuatorSettingsData.ChannelUpdateFreq[1]);
    UAVObjectUtilManager* utilMngr = pm->getObject<UAVObjectUtilManager>();
    if (utilMngr) {
        int board = utilMngr->getBoardModel();
        if ((board & 0xff00) == 1024) {
            // CopterControl family
            m_config->chBank1->setText("1-3");
            m_config->chBank2->setText("4");
            m_config->chBank3->setText("5,7-8");
            m_config->chBank4->setText("6,9-10");
            m_config->outputRate1->setEnabled(true);
            m_config->outputRate2->setEnabled(true);
            m_config->outputRate3->setEnabled(true);
            m_config->outputRate4->setEnabled(true);
            m_config->outputRate3->setValue(actuatorSettingsData.ChannelUpdateFreq[2]);
            m_config->outputRate4->setValue(actuatorSettingsData.ChannelUpdateFreq[3]);
        } else if ((board & 0xff00) == 256 ) {
            // Mainboard family
            m_config->outputRate1->setEnabled(true);
            m_config->outputRate2->setEnabled(true);
            m_config->outputRate3->setEnabled(false);
            m_config->outputRate4->setEnabled(false);
            m_config->chBank1->setText("1-4");
            m_config->chBank2->setText("5-8");
            m_config->chBank3->setText("-");
            m_config->chBank4->setText("-");
            m_config->outputRate3->setValue(0);
            m_config->outputRate4->setValue(0);
        }
    }

    // Get Channel ranges:
    foreach(OutputChannelForm *outputChannelForm, outputChannelForms)
    {
        int minValue = actuatorSettingsData.ChannelMin[outputChannelForm->index()];
        int maxValue = actuatorSettingsData.ChannelMax[outputChannelForm->index()];
        outputChannelForm->minmax(minValue, maxValue);

        int neutral = actuatorSettingsData.ChannelNeutral[outputChannelForm->index()];
        outputChannelForm->neutral(neutral);
    }

    setDirty(dirty);
}

/**
  * Sends the config to the board, without saving to the SD card (RC Output)
  */
void ConfigOutputWidget::updateObjectsFromWidgets()
{
    ActuatorSettings *actuatorSettings = ActuatorSettings::GetInstance(getObjectManager());
    Q_ASSERT(actuatorSettings);
    ActuatorSettings::DataFields actuatorSettingsData = actuatorSettings->getData();

    // Set channel ranges
    QList<OutputChannelForm*> outputChannelForms = findChildren<OutputChannelForm*>();
    foreach(OutputChannelForm *outputChannelForm, outputChannelForms)
    {
        actuatorSettingsData.ChannelMax[outputChannelForm->index()] = outputChannelForm->max();
        actuatorSettingsData.ChannelMin[outputChannelForm->index()] = outputChannelForm->min();
        actuatorSettingsData.ChannelNeutral[outputChannelForm->index()] = outputChannelForm->neutral();
    }

    // Set update rates
    actuatorSettingsData.ChannelUpdateFreq[0] = m_config->outputRate1->value();
    actuatorSettingsData.ChannelUpdateFreq[1] = m_config->outputRate2->value();
    actuatorSettingsData.ChannelUpdateFreq[2] = m_config->outputRate3->value();
    actuatorSettingsData.ChannelUpdateFreq[3] = m_config->outputRate4->value();

    // Apply settings
    actuatorSettings->setData(actuatorSettingsData);
}

void ConfigOutputWidget::openHelp()
{

    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/display/Doc/Output+Configuration", QUrl::StrictMode) );
}

void ConfigOutputWidget::stopTests()
{
    m_config->channelOutTest->setChecked(false);
}

void ConfigOutputWidget::disableIfNotMe(UAVObject* obj)
{
    if(obj->getMetadata().gcsTelemetryUpdateMode == UAVObject::UPDATEMODE_ONCHANGE)
    {
        if(!wasItMe)
            this->setEnabled(false);
    }
    else
        this->setEnabled(true);
}
