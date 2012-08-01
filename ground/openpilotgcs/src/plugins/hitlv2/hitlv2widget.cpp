/**
 ******************************************************************************
 *
 * @file       hitlv2widget.cpp
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

#include "hitlv2widget.h"
#include "ui_hitlv2widget.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QThread>

#include "hitlv2plugin.h"
#include "simulatorv2.h"
#include "uavobjectmanager.h"
#include <coreplugin/icore.h>
#include <coreplugin/threadmanager.h>

QStringList Simulator::instances;

HITLWidget::HITLWidget(QWidget *parent)
    : QWidget(parent),
      simulator(0)
{
    widget = new Ui_HITLWidget();
    widget->setupUi(this);
    widget->startButton->setEnabled(true);
    widget->stopButton->setEnabled(false);

    strAutopilotDisconnected = " AP OFF ";
    strSimulatorDisconnected = " Sim OFF ";
    strAutopilotConnected = " AP ON ";
//    strSimulatorConnected = " Sim ON ";
    strStyleEnable = "QFrame{background-color: green; color: white}";
    strStyleDisable = "QFrame{background-color: red; color: grey}";

    widget->apLabel->setText(strAutopilotDisconnected);
    widget->simLabel->setText(strSimulatorDisconnected);

    connect(widget->startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    connect(widget->stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
    connect(widget->buttonClearLog, SIGNAL(clicked()), this, SLOT(buttonClearLogClicked()));
}

HITLWidget::~HITLWidget()
{
    delete widget;
}

void HITLWidget::startButtonClicked()
{
    // allow only one instance per simulator
    if (Simulator::Instances().indexOf(settings.simulatorId) != -1) {
        widget->textBrowser->append(settings.simulatorId + " alreary started!");
        return;
    }

    if (!HITLPlugin::typeSimulators.size()) {
        widget->textBrowser->append("There is no registered simulators, add through HITLPlugin::addSimulator");
        return;
    }

    // Stop running process if one is active
    if (simulator) {
        QMetaObject::invokeMethod(simulator, "onDeleteSimulator", Qt::QueuedConnection);
        simulator = NULL;
    }

    if (settings.hostAddress == "" || settings.inPort == 0) {
        widget->textBrowser->append("Before start, set UDP parameters in options page!");
        return;
    }

    SimulatorCreator* creator = HITLPlugin::getSimulatorCreator(settings.simulatorId);
    simulator = creator->createSimulator(settings);
    simulator->setName(creator->Description());
    simulator->setSimulatorId(creator->ClassId());

    connect(simulator, SIGNAL(processOutput(QString)), this, SLOT(onProcessOutput(QString)));

    // Setup process
    onProcessOutput(QString("[%1] Starting %2... ")
                    .arg(QTime::currentTime().toString("hh:mm:ss"))
                    .arg(creator->Description()));

    // Start bridge
    bool ret = QMetaObject::invokeMethod(simulator, "setupProcess", Qt::QueuedConnection);
    if (ret) {
        Simulator::setInstance(settings.simulatorId);

        connect(this, SIGNAL(deleteSimulator()), simulator, SLOT(onDeleteSimulator()), Qt::QueuedConnection);

        widget->startButton->setEnabled(false);
        widget->stopButton->setEnabled(true);

        connect(simulator, SIGNAL(autopilotConnected()), this, SLOT(onAutopilotConnect()), Qt::QueuedConnection);
        connect(simulator, SIGNAL(autopilotDisconnected()), this, SLOT(onAutopilotDisconnect()), Qt::QueuedConnection);
        connect(simulator, SIGNAL(simulatorConnected()), this, SLOT(onSimulatorConnect()), Qt::QueuedConnection);
        connect(simulator, SIGNAL(simulatorDisconnected()), this, SLOT(onSimulatorDisconnect()), Qt::QueuedConnection);

        // Initialize connection status
        if (simulator->isAutopilotConnected())
            onAutopilotConnect();
        else
            onAutopilotDisconnect();

        if (simulator->isSimulatorConnected())
            onSimulatorConnect();
        else
            onSimulatorDisconnect();
    }
}

void HITLWidget::stopButtonClicked()
{
    if (simulator)
        widget->textBrowser->append(QString("[%1] Terminate %2 ")
                                    .arg(QTime::currentTime().toString("hh:mm:ss"))
                                    .arg(simulator->Name()));

    widget->startButton->setEnabled(true);
    widget->stopButton->setEnabled(false);
    widget->apLabel->setStyleSheet(QString::fromUtf8("QFrame{background-color: transparent; color: white}"));
    widget->simLabel->setStyleSheet(QString::fromUtf8("QFrame{background-color: transparent; color: white}"));
    widget->apLabel->setText(strAutopilotDisconnected);
    widget->simLabel->setText(strSimulatorDisconnected);
    if (simulator) {
        QMetaObject::invokeMethod(simulator, "onDeleteSimulator", Qt::QueuedConnection);
        simulator = NULL;
    }
}

void HITLWidget::buttonClearLogClicked()
{
    widget->textBrowser->clear();
}

void HITLWidget::onProcessOutput(QString text)
{
    widget->textBrowser->append(text);
}

void HITLWidget::onAutopilotConnect()
{
    widget->apLabel->setStyleSheet(strStyleEnable);
    widget->apLabel->setText(strAutopilotConnected);
}

void HITLWidget::onAutopilotDisconnect()
{
    widget->apLabel->setStyleSheet(strStyleDisable);
    widget->apLabel->setText(strAutopilotDisconnected);
}

void HITLWidget::onSimulatorConnect()
{
    widget->simLabel->setStyleSheet(strStyleEnable);
    widget->simLabel->setText(" " + simulator->Name() + " ON ");
}

void HITLWidget::onSimulatorDisconnect()
{
    widget->simLabel->setStyleSheet(strStyleDisable);
    widget->simLabel->setText(" " + simulator->Name() + " OFF ");
}
