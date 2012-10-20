/**
 ******************************************************************************
 *
 * @file       hitloptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup HITLPlugin HITL Plugin
 * @{
 * @brief The Hardware In The Loop plugin 
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

#include "hitloptionspage.h"
#include "hitlconfiguration.h"
#include "ui_hitloptionspage.h"
#include <hitlplugin.h>

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>
#include <simulator.h>



HITLOptionsPage::HITLOptionsPage(HITLConfiguration *conf, QObject *parent) :
    IOptionsPage(parent),
	config(conf)
{
}

QWidget *HITLOptionsPage::createPage(QWidget *parent)
{
    Q_UNUSED(parent);

    // Create page
    m_optionsPage = new Ui::HITLOptionsPage();
    QWidget* optionsPageWidget = new QWidget;
    m_optionsPage->setupUi(optionsPageWidget);
	int index = 0;
	foreach(SimulatorCreator* creator, HITLPlugin::typeSimulators)
	{
		m_optionsPage->chooseFlightSimulator->insertItem(index++, creator->Description(),creator->ClassId());
	}

	//QString classId = widget->listSimulators->itemData(0).toString();
	//SimulatorCreator* creator = HITLPlugin::getSimulatorCreator(classId);

	//QWidget* embedPage = creator->createOptionsPage();
	//m_optionsPage->verticalLayout->addWidget(embedPage);

	m_optionsPage->executablePath->setExpectedKind(Utils::PathChooser::File);
	m_optionsPage->executablePath->setPromptDialogTitle(tr("Choose flight simulator executable"));
	m_optionsPage->dataPath->setExpectedKind(Utils::PathChooser::Directory);
	m_optionsPage->dataPath->setPromptDialogTitle(tr("Choose flight simulator data directory"));

    // Restore the contents from the settings:
    foreach(SimulatorCreator* creator, HITLPlugin::typeSimulators)
    {
        QString id = config->Settings().simulatorId;
        if(creator->ClassId() == id)
            m_optionsPage->chooseFlightSimulator->setCurrentIndex(HITLPlugin::typeSimulators.indexOf(creator));
    }

    m_optionsPage->executablePath->setPath(config->Settings().binPath);
    m_optionsPage->dataPath->setPath(config->Settings().dataPath);

    m_optionsPage->manualControlRadioButton->setChecked(config->Settings().manualControlEnabled);
    m_optionsPage->gcsReceiverRadioButton->setChecked(config->Settings().gcsReceiverEnabled);

    m_optionsPage->startSim->setChecked(config->Settings().startSim);
    m_optionsPage->noiseCheckBox->setChecked(config->Settings().addNoise);

    m_optionsPage->hostAddress->setText(config->Settings().hostAddress);
    m_optionsPage->remoteAddress->setText(config->Settings().remoteAddress);
    m_optionsPage->outputPort->setText(QString::number(config->Settings().outPort));
    m_optionsPage->inputPort->setText(QString::number(config->Settings().inPort));
    m_optionsPage->latitude->setText(config->Settings().latitude);
    m_optionsPage->longitude->setText(config->Settings().longitude);

    m_optionsPage->groundTruthCheckbox->setChecked(config->Settings().groundTruthEnabled);
    m_optionsPage->gpsPositionCheckbox->setChecked(config->Settings().gpsPositionEnabled);
    m_optionsPage->attActualCheckbox->setChecked(config->Settings().attActualEnabled);
    m_optionsPage->attRawCheckbox->setChecked(config->Settings().attRawEnabled);



    m_optionsPage->attRawRateSpinbox->setValue(config->Settings().attRawRate);
    m_optionsPage->gpsPosRateSpinbox->setValue(config->Settings().gpsPosRate);
    m_optionsPage->groundTruthRateSpinbox->setValue(config->Settings().groundTruthRate);
//    m_optionsPage->attActualRate->setValue(config->Settings().attActualRate);

    m_optionsPage->baroAltitudeCheckbox->setChecked(config->Settings().baroAltitudeEnabled);
    m_optionsPage->baroAltRateSpinbox->setValue(config->Settings().baroAltRate);

    m_optionsPage->minOutputPeriodSpinbox->setValue(config->Settings().minOutputPeriod);

    m_optionsPage->attActHW->setChecked(config->Settings().attActHW);
    m_optionsPage->attActCalc->setChecked(config->Settings().attActCalc);
    m_optionsPage->attActSim->setChecked(config->Settings().attActSim);

    m_optionsPage->airspeedActualCheckbox->setChecked(config->Settings().airspeedActualEnabled);
    m_optionsPage->airspeedRateSpinbox->setValue(config->Settings().airspeedActualRate);

    return optionsPageWidget;
}

void HITLOptionsPage::apply()
{
    SimulatorSettings settings;
    int i = m_optionsPage->chooseFlightSimulator->currentIndex();

    settings.simulatorId = m_optionsPage->chooseFlightSimulator->itemData(i).toString();
    settings.binPath = m_optionsPage->executablePath->path();
    settings.dataPath = m_optionsPage->dataPath->path();
    settings.startSim = m_optionsPage->startSim->isChecked();
    settings.addNoise = m_optionsPage->noiseCheckBox->isChecked();
    settings.hostAddress = m_optionsPage->hostAddress->text();
    settings.remoteAddress = m_optionsPage->remoteAddress->text();

    settings.inPort = m_optionsPage->inputPort->text().toInt();
    settings.outPort = m_optionsPage->outputPort->text().toInt();
    settings.longitude = m_optionsPage->longitude->text();
    settings.latitude = m_optionsPage->latitude->text();

    settings.addNoise = m_optionsPage->noiseCheckBox->isChecked();

    settings.attRawEnabled = m_optionsPage->attRawCheckbox->isChecked();
    settings.attRawRate = m_optionsPage->attRawRateSpinbox->value();

    settings.attActualEnabled = m_optionsPage->attActualCheckbox->isChecked();

    settings.gpsPositionEnabled = m_optionsPage->gpsPositionCheckbox->isChecked();
    settings.gpsPosRate = m_optionsPage->gpsPosRateSpinbox->value();

    settings.groundTruthEnabled = m_optionsPage->groundTruthCheckbox->isChecked();
    settings.groundTruthRate = m_optionsPage->groundTruthRateSpinbox->value();

    settings.baroAltitudeEnabled = m_optionsPage->baroAltitudeCheckbox->isChecked();
    settings.baroAltRate = m_optionsPage->baroAltRateSpinbox->value();

    settings.minOutputPeriod = m_optionsPage->minOutputPeriodSpinbox->value();

    settings.manualControlEnabled = m_optionsPage->manualControlRadioButton->isChecked();
    settings.gcsReceiverEnabled = m_optionsPage->gcsReceiverRadioButton->isChecked();

    settings.attActHW = m_optionsPage->attActHW->isChecked();
    settings.attActSim = m_optionsPage->attActSim->isChecked();
    settings.attActCalc = m_optionsPage->attActCalc->isChecked();

    settings.airspeedActualEnabled=m_optionsPage->airspeedActualCheckbox->isChecked();
    settings.airspeedActualRate=m_optionsPage->airspeedRateSpinbox->value();

    //Write settings to file
    config->setSimulatorSettings(settings);
}

void HITLOptionsPage::finish()
{
    delete m_optionsPage;
}
