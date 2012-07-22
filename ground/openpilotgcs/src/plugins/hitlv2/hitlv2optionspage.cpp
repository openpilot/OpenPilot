/**
 ******************************************************************************
 *
 * @file       hitlv2optionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup HITLPlugin HITLv2 Plugin
 * @{
 * @brief The Hardware In The Loop plugin version 2
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

#include "hitlv2optionspage.h"
#include "hitlv2configuration.h"
#include "ui_hitlv2optionspage.h"
#include "hitlv2plugin.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>
#include "simulatorv2.h"

HITLOptionsPage::HITLOptionsPage(HITLConfiguration *conf, QObject *parent) :
    IOptionsPage(parent),
    config(conf)
{
}

QWidget *HITLOptionsPage::createPage(QWidget *parent)
{
//    qDebug() << "HITLOptionsPage::createPage";
    // Create page
    m_optionsPage = new Ui::HITLOptionsPage();
    QWidget* optionsPageWidget = new QWidget;
    m_optionsPage->setupUi(optionsPageWidget);
    int index = 0;
    foreach (SimulatorCreator* creator, HITLPlugin::typeSimulators)
        m_optionsPage->chooseFlightSimulator->insertItem(index++, creator->Description(), creator->ClassId());

    m_optionsPage->executablePath->setExpectedKind(Utils::PathChooser::File);
    m_optionsPage->executablePath->setPromptDialogTitle(tr("Choose flight simulator executable"));
    m_optionsPage->dataPath->setExpectedKind(Utils::PathChooser::Directory);
    m_optionsPage->dataPath->setPromptDialogTitle(tr("Choose flight simulator data directory"));

    // Restore the contents from the settings:
    foreach (SimulatorCreator* creator, HITLPlugin::typeSimulators) {
        QString id = config->Settings().simulatorId;
        if (creator->ClassId() == id)
            m_optionsPage->chooseFlightSimulator->setCurrentIndex(HITLPlugin::typeSimulators.indexOf(creator));
    }

    m_optionsPage->hostAddress->setText(config->Settings().hostAddress);
    m_optionsPage->inputPort->setText(QString::number(config->Settings().inPort));
    m_optionsPage->remoteAddress->setText(config->Settings().remoteAddress);
    m_optionsPage->outputPort->setText(QString::number(config->Settings().outPort));
    m_optionsPage->executablePath->setPath(config->Settings().binPath);
    m_optionsPage->dataPath->setPath(config->Settings().dataPath);

    m_optionsPage->homeLocation->setChecked(config->Settings().homeLocation);
    m_optionsPage->homeLocRate->setValue(config->Settings().homeLocRate);

    m_optionsPage->attRaw->setChecked(config->Settings().attRaw);
    m_optionsPage->attRawRate->setValue(config->Settings().attRawRate);

    m_optionsPage->attActual->setChecked(config->Settings().attActual);
    m_optionsPage->attActHW->setChecked(config->Settings().attActHW);
    m_optionsPage->attActSim->setChecked(config->Settings().attActSim);
    m_optionsPage->attActCalc->setChecked(config->Settings().attActCalc);

    m_optionsPage->sonarAltitude->setChecked(config->Settings().sonarAltitude);
    m_optionsPage->sonarMaxAlt->setValue(config->Settings().sonarMaxAlt);
    m_optionsPage->sonarAltRate->setValue(config->Settings().sonarAltRate);

    m_optionsPage->gpsPosition->setChecked(config->Settings().gpsPosition);
    m_optionsPage->gpsPosRate->setValue(config->Settings().gpsPosRate);

    m_optionsPage->inputCommand->setChecked(config->Settings().inputCommand);
    m_optionsPage->gcsReciever->setChecked(config->Settings().gcsReciever);
    m_optionsPage->manualControl->setChecked(config->Settings().manualControl);

    m_optionsPage->manualOutput->setChecked(config->Settings().manualOutput);
    m_optionsPage->outputRate->setValue(config->Settings().outputRate);

    return optionsPageWidget;
}

void HITLOptionsPage::apply()
{
    SimulatorSettings settings;
    int i = m_optionsPage->chooseFlightSimulator->currentIndex();

    settings.simulatorId    = m_optionsPage->chooseFlightSimulator->itemData(i).toString();
    settings.hostAddress    = m_optionsPage->hostAddress->text();
    settings.inPort         = m_optionsPage->inputPort->text().toInt();
    settings.remoteAddress  = m_optionsPage->remoteAddress->text();
    settings.outPort        = m_optionsPage->outputPort->text().toInt();
    settings.binPath        = m_optionsPage->executablePath->path();
    settings.dataPath       = m_optionsPage->dataPath->path();

    settings.homeLocation   = m_optionsPage->homeLocation->isChecked();
    settings.homeLocRate    = m_optionsPage->homeLocRate->value();

    settings.attRaw         = m_optionsPage->attRaw->isChecked();
    settings.attRawRate     = m_optionsPage->attRawRate->value();

    settings.attActual      = m_optionsPage->attActual->isChecked();
    settings.attActHW       = m_optionsPage->attActHW->isChecked();
    settings.attActSim      = m_optionsPage->attActSim->isChecked();
    settings.attActCalc     = m_optionsPage->attActCalc->isChecked();

    settings.sonarAltitude  = m_optionsPage->sonarAltitude->isChecked();
    settings.sonarMaxAlt    = m_optionsPage->sonarMaxAlt->value();
    settings.sonarAltRate   = m_optionsPage->sonarAltRate->value();

    settings.gpsPosition    = m_optionsPage->gpsPosition->isChecked();
    settings.gpsPosRate     = m_optionsPage->gpsPosRate->value();

    settings.inputCommand   = m_optionsPage->inputCommand->isChecked();
    settings.gcsReciever    = m_optionsPage->gcsReciever->isChecked();
    settings.manualControl  = m_optionsPage->manualControl->isChecked();

    settings.manualOutput   = m_optionsPage->manualOutput->isChecked();
    settings.outputRate     = m_optionsPage->outputRate->value();

    config->setSimulatorSettings(settings);
}

void HITLOptionsPage::finish()
{
    delete m_optionsPage;
}
