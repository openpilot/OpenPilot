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
#include "configvehicletypewidget.h"

#include "mixersettings.h"
#include "actuatorcommand.h"
#include "actuatorsettings.h"
#include "systemsettings.h"
#include "uavsettingsimportexport/uavsettingsimportexportfactory.h"
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/generalsettings.h>

#include <QDebug>
#include <QStringList>
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

ConfigOutputWidget::ConfigOutputWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_ui = new Ui_OutputWidget();
    m_ui->setupUi(this);

    m_ui->gvFrame->setVisible(false);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings *settings = pm->getObject<Core::Internal::GeneralSettings>();
    if (!settings->useExpertMode()) {
        m_ui->saveRCOutputToRAM->setVisible(false);
    }

    UAVSettingsImportExportFactory *importexportplugin = pm->getObject<UAVSettingsImportExportFactory>();
    connect(importexportplugin, SIGNAL(importAboutToBegin()), this, SLOT(stopTests()));

    connect(m_ui->channelOutTest, SIGNAL(clicked(bool)), this, SLOT(runChannelTests(bool)));

    // Configure the task widget
    // Connect the help button
    connect(m_ui->outputHelp, SIGNAL(clicked()), this, SLOT(openHelp()));

    addApplySaveButtons(m_ui->saveRCOutputToRAM, m_ui->saveRCOutputToSD);

    // Track the ActuatorSettings object
    addUAVObject("ActuatorSettings");

    // NOTE: we have channel indices from 0 to 9, but the convention for OP is Channel 1 to Channel 10.
    // Register for ActuatorSettings changes:
    for (unsigned int i = 0; i < ActuatorCommand::CHANNEL_NUMELEM; i++) {
        OutputChannelForm *form = new OutputChannelForm(i, this);
        form->moveTo(*(m_ui->channelLayout));

        connect(m_ui->channelOutTest, SIGNAL(toggled(bool)), form, SLOT(enableChannelTest(bool)));
        connect(form, SIGNAL(channelChanged(int, int)), this, SLOT(sendChannelTest(int, int)));

        addWidget(form->ui.actuatorMin);
        addWidget(form->ui.actuatorNeutral);
        addWidget(form->ui.actuatorMax);
        addWidget(form->ui.actuatorRev);
        addWidget(form->ui.actuatorLink);
    }


    // Associate the buttons with their UAVO fields
    addWidget(m_ui->spinningArmed);
    MixerSettings *mixer = MixerSettings::GetInstance(getObjectManager());
    Q_ASSERT(mixer);
    m_banks << OutputBankControls(mixer, m_ui->chBank1, QColor("#C6ECAE"), m_ui->cb_outputRate1, m_ui->cb_outputMode1);
    m_banks << OutputBankControls(mixer, m_ui->chBank2, QColor("#91E5D3"), m_ui->cb_outputRate2, m_ui->cb_outputMode2);
    m_banks << OutputBankControls(mixer, m_ui->chBank3, QColor("#FCEC52"), m_ui->cb_outputRate3, m_ui->cb_outputMode3);
    m_banks << OutputBankControls(mixer, m_ui->chBank4, QColor("#C3A8FF"), m_ui->cb_outputRate4, m_ui->cb_outputMode4);
    m_banks << OutputBankControls(mixer, m_ui->chBank5, QColor("#F7F7F2"), m_ui->cb_outputRate5, m_ui->cb_outputMode5);
    m_banks << OutputBankControls(mixer, m_ui->chBank6, QColor("#FF9F51"), m_ui->cb_outputRate6, m_ui->cb_outputMode6);

    QList<int> rates;
    rates << 50 << 60 << 125 << 165 << 270 << 330 << 400 << 490;
    int i = 0;
    foreach(OutputBankControls controls, m_banks) {
        addWidget(controls.rateCombo());

        controls.rateCombo()->addItem(tr("-"), QVariant(0));
        controls.rateCombo()->model()->setData(controls.rateCombo()->model()->index(0, 0), QVariant(0), Qt::UserRole - 1);
        foreach(int rate, rates) {
            controls.rateCombo()->addItem(tr("%1 Hz").arg(rate), rate);
        }

        addWidgetBinding("ActuatorSettings", "BankMode", controls.modeCombo(), i++, 0, true);
        connect(controls.modeCombo(), SIGNAL(currentIndexChanged(int)), this, SLOT(onBankTypeChange()));
    }

    SystemAlarms *systemAlarmsObj = SystemAlarms::GetInstance(getObjectManager());
    connect(systemAlarmsObj, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(updateWarnings(UAVObject *)));

    disconnect(this, SLOT(refreshWidgetsValues(UAVObject *)));

    populateWidgets();
    refreshWidgetsValues();

    updateEnableControls();
}

ConfigOutputWidget::~ConfigOutputWidget()
{
    SystemAlarms *systemAlarmsObj = SystemAlarms::GetInstance(getObjectManager());

    disconnect(systemAlarmsObj, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(updateWarnings(UAVObject *)));
    foreach(OutputBankControls controls, m_banks) {
        disconnect(controls.modeCombo(), SIGNAL(currentIndexChanged(int)), this, SLOT(onBankTypeChange()));
    }
}

void ConfigOutputWidget::enableControls(bool enable)
{
    ConfigTaskWidget::enableControls(enable);

    if (!enable) {
        m_ui->channelOutTest->setChecked(false);
    }
    m_ui->channelOutTest->setEnabled(enable);
}

/**
   Force update all channels with the values in the OutputChannelForms.
 */
void ConfigOutputWidget::sendAllChannelTests()
{
    for (unsigned int i = 0; i < ActuatorCommand::CHANNEL_NUMELEM; i++) {
        OutputChannelForm *form = getOutputChannelForm(i);
        sendChannelTest(i, form->neutral());
    }
}

/**
   Toggles the channel testing mode by making the GCS take over
   the ActuatorCommand objects
 */
void ConfigOutputWidget::runChannelTests(bool state)
{
    SystemAlarms *systemAlarmsObj = SystemAlarms::GetInstance(getObjectManager());
    SystemAlarms::DataFields systemAlarms = systemAlarmsObj->getData();

    if (state && systemAlarms.Alarm[SystemAlarms::ALARM_ACTUATOR] != SystemAlarms::ALARM_OK) {
        QMessageBox mbox;
        mbox.setText(QString(tr("The actuator module is in an error state. This can also occur because there are no inputs. "
                                "Please fix these before testing outputs.")));
        mbox.setStandardButtons(QMessageBox::Ok);
        mbox.exec();

        // Unfortunately must cache this since callback will reoccur
        m_accInitialData = ActuatorCommand::GetInstance(getObjectManager())->getMetadata();

        m_ui->channelOutTest->setChecked(false);
        return;
    }

    // Confirm this is definitely what they want
    if (state) {
        QMessageBox mbox;
        mbox.setText(QString(tr("This option will start your motors by the amount selected on the sliders regardless of transmitter."
                                "It is recommended to remove any blades from motors. Are you sure you want to do this?")));
        mbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int retval = mbox.exec();
        if (retval != QMessageBox::Yes) {
            state = false;
            qDebug() << "Cancelled";
            m_ui->channelOutTest->setChecked(false);
            return;
        }
    }

    ActuatorCommand *obj = ActuatorCommand::GetInstance(getObjectManager());
    UAVObject::Metadata mdata = obj->getMetadata();
    if (state) {
        m_accInitialData = mdata;
        UAVObject::SetFlightAccess(mdata, UAVObject::ACCESS_READONLY);
        UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_ONCHANGE);
        UAVObject::SetGcsTelemetryAcked(mdata, false);
        UAVObject::SetGcsTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_ONCHANGE);
        mdata.gcsTelemetryUpdatePeriod = 100;
    } else {
        mdata = m_accInitialData; // Restore metadata
    }
    obj->setMetadata(mdata);
    obj->updated();

    // Setup the correct initial channel values when the channel testing mode is turned on.
    if (state) {
        sendAllChannelTests();
    }

    // Add info at end
    if (!state && isDirty()) {
        QMessageBox mbox;
        mbox.setText(QString(tr("You may want to save your neutral settings.")));
        mbox.setStandardButtons(QMessageBox::Ok);
        mbox.setIcon(QMessageBox::Information);
        mbox.exec();
    }
}

OutputChannelForm *ConfigOutputWidget::getOutputChannelForm(const int index) const
{
    QList<OutputChannelForm *> outputChannelForms = findChildren<OutputChannelForm *>();
    foreach(OutputChannelForm * outputChannelForm, outputChannelForms) {
        if (outputChannelForm->index() == index) {
            return outputChannelForm;
        }
    }

    // no OutputChannelForm found with given index
    return NULL;
}

/**
 * Set the label for a channel output assignement
 */
void ConfigOutputWidget::assignOutputChannel(UAVDataObject *obj, QString &str)
{
    // FIXME: use signal/ slot approach
    UAVObjectField *field = obj->getField(str);
    QStringList options   = field->getOptions();
    int index = options.indexOf(field->getValue().toString());

    OutputChannelForm *outputChannelForm = getOutputChannelForm(index);

    if (outputChannelForm) {
        outputChannelForm->setName(str);
    }
}

/**
   Sends the channel value to the UAV to move the servo.
   Returns immediately if we are not in testing mode
 */
void ConfigOutputWidget::sendChannelTest(int index, int value)
{
    if (!m_ui->channelOutTest->isChecked()) {
        return;
    }

    if (index < 0 || (unsigned)index >= ActuatorCommand::CHANNEL_NUMELEM) {
        return;
    }

    ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(getObjectManager());
    Q_ASSERT(actuatorCommand);
    ActuatorCommand::DataFields actuatorCommandFields = actuatorCommand->getData();
    actuatorCommandFields.Channel[index] = value;
    actuatorCommand->setData(actuatorCommandFields);
}

void ConfigOutputWidget::setColor(QWidget *widget, const QColor color)
{
    QPalette p(palette());

    p.setColor(QPalette::Background, color);
    p.setColor(QPalette::Base, color);
    p.setColor(QPalette::Active, QPalette::Button, color);
    p.setColor(QPalette::Inactive, QPalette::Button, color);
    widget->setAutoFillBackground(true);
    widget->setPalette(p);
}

/********************************
 *  Output settings
 *******************************/

/**
   Request the current config from the board (RC Output)
 */
void ConfigOutputWidget::refreshWidgetsValues(UAVObject *obj)
{
    bool dirty = isDirty();

    ConfigTaskWidget::refreshWidgetsValues(obj);

    // Get Actuator Settings
    ActuatorSettings *actuatorSettings = ActuatorSettings::GetInstance(getObjectManager());

    Q_ASSERT(actuatorSettings);
    ActuatorSettings::DataFields actuatorSettingsData = actuatorSettings->getData();

    // Get channel descriptions
    QStringList channelDesc = ConfigVehicleTypeWidget::getChannelDescriptions();

    // Initialize output forms
    QList<OutputChannelForm *> outputChannelForms = findChildren<OutputChannelForm *>();
    foreach(OutputChannelForm * outputChannelForm, outputChannelForms) {
        outputChannelForm->setName(channelDesc[outputChannelForm->index()]);

        // init min,max,neutral
        int minValue = actuatorSettingsData.ChannelMin[outputChannelForm->index()];
        int maxValue = actuatorSettingsData.ChannelMax[outputChannelForm->index()];
        outputChannelForm->setRange(minValue, maxValue);

        int neutral  = actuatorSettingsData.ChannelNeutral[outputChannelForm->index()];
        outputChannelForm->setNeutral(neutral);
    }

    // Get the SpinWhileArmed setting
    m_ui->spinningArmed->setChecked(actuatorSettingsData.MotorsSpinWhileArmed == ActuatorSettings::MOTORSSPINWHILEARMED_TRUE);

    for (int i = 0; i < m_banks.count(); i++) {
        OutputBankControls controls = m_banks.at(i);
        // Reset to all disabled
        controls.label()->setText("-");

        controls.rateCombo()->setEnabled(false);
        setColor(controls.rateCombo(), palette().color(QPalette::Background));
        controls.rateCombo()->setCurrentIndex(0);

        controls.modeCombo()->setEnabled(false);
        setColor(controls.modeCombo(), palette().color(QPalette::Background));
    }

    // Get connected board model
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    UAVObjectUtilManager *utilMngr     = pm->getObject<UAVObjectUtilManager>();
    Q_ASSERT(utilMngr);
    QStringList bankLabels;
    QList<int> channelBanks;

    if (utilMngr) {
        int board = utilMngr->getBoardModel();
        // Setup labels and combos for banks according to board type
        if ((board & 0xff00) == 0x0900) {
            // Revolution family of boards 6 timer banks
            bankLabels << "1 (1-2)" << "2 (3)" << "3 (4)" << "4 (5-6)" << "5 (7-8)" << "6 (9-10)";
            channelBanks << 1 << 1 << 2 << 3 << 4 << 4 << 5 << 5 << 6 << 6;
        }
    }

    int i = 0;
    foreach(QString banklabel, bankLabels) {
        OutputBankControls controls = m_banks.at(i);

        controls.label()->setText(banklabel);
        int index = controls.rateCombo()->findData(actuatorSettingsData.BankUpdateFreq[i]);
        if (index == -1) {
            controls.rateCombo()->addItem(tr("%1 Hz").arg(actuatorSettingsData.BankUpdateFreq[i]), actuatorSettingsData.BankUpdateFreq[i]);
        }
        controls.rateCombo()->setCurrentIndex(index);
        controls.rateCombo()->setEnabled(controls.modeCombo()->currentIndex() == ActuatorSettings::BANKMODE_PWM);
        setColor(controls.rateCombo(), controls.color());
        controls.modeCombo()->setEnabled(true);
        setColor(controls.modeCombo(), controls.color());
        i++;
    }

    // Get Channel ranges:
    i = 0;
    foreach(OutputChannelForm * outputChannelForm, outputChannelForms) {
        int minValue = actuatorSettingsData.ChannelMin[outputChannelForm->index()];
        int maxValue = actuatorSettingsData.ChannelMax[outputChannelForm->index()];

        outputChannelForm->setRange(minValue, maxValue);
        if (channelBanks.count() > i) {
            outputChannelForm->setBank(QString("%1").arg(channelBanks.at(i)));
            outputChannelForm->setColor(m_banks.at(channelBanks.at(i++) - 1).color());
        }
        int neutral = actuatorSettingsData.ChannelNeutral[outputChannelForm->index()];
        outputChannelForm->setNeutral(neutral);
    }

    setDirty(dirty);
}

/**
 * Sends the config to the board, without saving to the SD card (RC Output)
 */
void ConfigOutputWidget::updateObjectsFromWidgets()
{
    ConfigTaskWidget::updateObjectsFromWidgets();

    ActuatorSettings *actuatorSettings = ActuatorSettings::GetInstance(getObjectManager());

    Q_ASSERT(actuatorSettings);
    if (actuatorSettings) {
        ActuatorSettings::DataFields actuatorSettingsData = actuatorSettings->getData();

        // Set channel ranges
        QList<OutputChannelForm *> outputChannelForms     = findChildren<OutputChannelForm *>();
        foreach(OutputChannelForm * outputChannelForm, outputChannelForms) {
            actuatorSettingsData.ChannelMax[outputChannelForm->index()]     = outputChannelForm->max();
            actuatorSettingsData.ChannelMin[outputChannelForm->index()]     = outputChannelForm->min();
            actuatorSettingsData.ChannelNeutral[outputChannelForm->index()] = outputChannelForm->neutral();
        }

        // Set update rates
        actuatorSettingsData.BankUpdateFreq[0]    = m_ui->cb_outputRate1->currentData().toUInt();
        actuatorSettingsData.BankUpdateFreq[1]    = m_ui->cb_outputRate2->currentData().toUInt();
        actuatorSettingsData.BankUpdateFreq[2]    = m_ui->cb_outputRate3->currentData().toUInt();
        actuatorSettingsData.BankUpdateFreq[3]    = m_ui->cb_outputRate4->currentData().toUInt();
        actuatorSettingsData.BankUpdateFreq[4]    = m_ui->cb_outputRate5->currentData().toUInt();
        actuatorSettingsData.BankUpdateFreq[5]    = m_ui->cb_outputRate6->currentData().toUInt();

        actuatorSettingsData.MotorsSpinWhileArmed = m_ui->spinningArmed->isChecked() ?
                                                    ActuatorSettings::MOTORSSPINWHILEARMED_TRUE :
                                                    ActuatorSettings::MOTORSSPINWHILEARMED_FALSE;

        // Apply settings
        actuatorSettings->setData(actuatorSettingsData);
    }
}

void ConfigOutputWidget::openHelp()
{
    QDesktopServices::openUrl(QUrl(tr("http://wiki.openpilot.org/x/WIGf"), QUrl::StrictMode));
}

void ConfigOutputWidget::onBankTypeChange()
{
    QComboBox *bankModeCombo = qobject_cast<QComboBox *>(sender());

    if (bankModeCombo != NULL) {
        foreach(OutputBankControls controls, m_banks) {
            if (controls.modeCombo() == bankModeCombo) {
                bool enabled = bankModeCombo->currentIndex() == ActuatorSettings::BANKMODE_PWM;
                controls.rateCombo()->setEnabled(enabled);
                controls.rateCombo()->setCurrentIndex(enabled ? 1 : 0);
                break;
            }
        }
    }
}

void ConfigOutputWidget::stopTests()
{
    m_ui->channelOutTest->setChecked(false);
}

void ConfigOutputWidget::updateWarnings(UAVObject *)
{
    SystemAlarms *systemAlarmsObj = SystemAlarms::GetInstance(getObjectManager());
    SystemAlarms::DataFields systemAlarms = systemAlarmsObj->getData();

    if (systemAlarms.Alarm[SystemAlarms::ALARM_SYSTEMCONFIGURATION] > SystemAlarms::ALARM_WARNING) {
        switch (systemAlarms.ExtendedAlarmStatus[SystemAlarms::EXTENDEDALARMSTATUS_SYSTEMCONFIGURATION]) {
        case SystemAlarms::EXTENDEDALARMSTATUS_UNSUPPORTEDCONFIG_ONESHOT:
            setWarning(tr("OneShot and PWMSync output only works with Receiver Port settings marked with '+OneShot'<br>"
                          "When using Receiver Port setting 'PPM_PIN8+OneShot' "
                          "<b><font color='%1'>Bank %2</font></b> must be set to PWM")
                       .arg(m_banks.at(3).color().name()).arg(m_banks.at(3).label()->text()));
            return;
        }
    }
    setWarning(NULL);
}

void ConfigOutputWidget::setWarning(QString message)
{
    m_ui->gvFrame->setVisible(!message.isNull());
    m_ui->picWarning->setPixmap(message.isNull() ? QPixmap() : QPixmap(":/configgadget/images/error.svg"));
    m_ui->txtWarning->setText(message);
}


OutputBankControls::OutputBankControls(MixerSettings *mixer, QLabel *label, QColor color, QComboBox *rateCombo, QComboBox *modeCombo) :
    m_mixer(mixer), m_label(label), m_color(color), m_rateCombo(rateCombo), m_modeCombo(modeCombo)
{}

OutputBankControls::~OutputBankControls()
{}
