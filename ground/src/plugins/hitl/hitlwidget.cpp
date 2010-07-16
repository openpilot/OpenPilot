/**
 ******************************************************************************
 *
 * @file       hitlwidget.cpp
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
#include "hitlwidget.h"
#include "ui_hitlwidget.h"
#include "qxtlogger.h"
#include <QDebug>
#include "uavobjects/uavobjectmanager.h"
#include <QFile>
#include <QDir>

HITLWidget::HITLWidget(QWidget *parent) : QWidget(parent)
{
    // Note: Only tested on windows 7
#if defined(Q_WS_WIN)
    cmdShell = QString("c:/windows/system32/cmd.exe");
#else
    cmdShell = QString("bash");
#endif
    fgProcess = NULL;
    widget = new Ui_HITLWidget();
    widget->setupUi(this);
    connect(widget->startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    connect(widget->stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
}

HITLWidget::~HITLWidget()
{
   delete widget;
}

void HITLWidget::startButtonClicked()
{
    // Stop running process if one is active
    if (fgProcess != NULL)
    {
        stopButtonClicked();
    }

    // Copy FlightGear generic protocol configuration file to the FG protocol directory
    // NOTE: Not working on Windows 7, if FG is installed in the "Program Files",
    // likelly due to permissions. The file should be manually copied to data/Protocol/opfgprotocol.xml
    QFile xmlFile(":/flightgear/genericprotocol/opfgprotocol.xml");
    xmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QString xml = xmlFile.readAll();
    xmlFile.close();
    QFile xmlFileOut(fgPathData + "/Protocol/opfgprotocol.xml");
    xmlFileOut.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlFileOut.write(xml.toAscii());
    xmlFileOut.close();

    // Setup process
    widget->textBrowser->append(QString("Starting FlightGear ...\n"));
    qxtLog->info("HITL: Starting FlightGear");
    fgProcess = new QProcess();
    fgProcess->setReadChannelMode(QProcess::MergedChannels);
    connect(fgProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(processReadyRead()));    

    // Start shell (Note: Could not start FG directly on Windows, only through terminal!)
    fgProcess->start(cmdShell);
    if (fgProcess->waitForStarted() == false)
    {
        widget->textBrowser->append("Error:" + fgProcess->errorString());
    }

    // Setup arguments
    // Note: The input generic protocol is set to update at a much higher rate than the actual updates are sent by the GCS.
    // If this is not done then a lag will be introduced by FlightGear, likelly because the receive socket buffer builds up during startup.
    QString args("--fg-root=\"" + fgPathData + "\" --timeofday=dusk --httpd=5400 --enable-hud --in-air --altitude=2000 --vc=100 --generic=socket,out,50,localhost,5500,udp,opfgprotocol");
    if ( !fgManualControl )
    {
        args.append(" --generic=socket,in,400,localhost,5501,udp,opfgprotocol");
    }

    // Start FlightGear
    QString cmd;
    cmd = "\"" + fgPathBin + "\" " + args + "\n";
    fgProcess->write(cmd.toAscii());

    // Start bridge
    qxtLog->info("HITL: Starting bridge, initializing FlighGear and Autopilot connections");
    fgBridge = new FlightGearBridge();
    connect(fgBridge, SIGNAL(autopilotConnected()), this, SLOT(onAutopilotConnect()));
    connect(fgBridge, SIGNAL(autopilotDisconnected()), this, SLOT(onAutopilotDisconnect()));
    connect(fgBridge, SIGNAL(fgConnected()), this, SLOT(onFGConnect()));
    connect(fgBridge, SIGNAL(fgDisconnected()), this, SLOT(onFGDisconnect()));

    // Initialize connection status
    if ( fgBridge->isAutopilotConnected() )
    {
        onAutopilotConnect();
    }
    else
    {
        onAutopilotDisconnect();
    }
    if ( fgBridge->isFGConnected() )
    {
        onFGConnect();
    }
    else
    {
        onFGDisconnect();
    }
}

void HITLWidget::stopButtonClicked()
{
    // NOTE: Does not currently work, may need to send control+c to through the terminal
    if (fgProcess != NULL)
    {
        fgProcess->disconnect(this);
        fgProcess->kill();
        delete fgProcess;
        fgBridge->disconnect(this);
        delete fgBridge;
        fgProcess = NULL;
    }
}

void HITLWidget::setFGPathBin(QString fgPath)
{
    this->fgPathBin = fgPath;
}

void HITLWidget::setFGPathData(QString fgPath)
{
    this->fgPathData = fgPath;
}

void HITLWidget::setFGManualControl(bool val)
{
    this->fgManualControl = val;
}

void HITLWidget::processReadyRead()
{
    QByteArray bytes = fgProcess->readAllStandardOutput();
    QString str(bytes);
    if ( !str.contains("Error reading data") ) // ignore error
    {
        widget->textBrowser->append(str);
    }
}

void HITLWidget::onAutopilotConnect()
{
    QPalette pal(widget->apLabel->palette());
    pal.setColor(QPalette::Window, Qt::green);
    widget->apLabel->setPalette(pal);
    widget->apLabel->setAutoFillBackground(true);
    widget->apLabel->setText("AutoPilot Connected");
    qxtLog->info("HITL: Autopilot connected, initializing for HITL simulation");
}

void HITLWidget::onAutopilotDisconnect()
{
    QPalette pal(widget->apLabel->palette());
    pal.setColor(QPalette::Window, Qt::red);
    widget->apLabel->setPalette(pal);
    widget->apLabel->setAutoFillBackground(true);
    widget->apLabel->setText("AutoPilot Disconnected");
    qxtLog->info("HITL: Autopilot disconnected");
}

void HITLWidget::onFGConnect()
{
    QPalette pal(widget->fgLabel->palette());
    pal.setColor(QPalette::Window, Qt::green);
    widget->fgLabel->setPalette(pal);
    widget->fgLabel->setAutoFillBackground(true);
    widget->fgLabel->setText("FlightGear Connected");
    qxtLog->info("HITL: FlighGear connected");
}

void HITLWidget::onFGDisconnect()
{
    QPalette pal(widget->fgLabel->palette());
    pal.setColor(QPalette::Window, Qt::red);
    widget->fgLabel->setPalette(pal);
    widget->fgLabel->setAutoFillBackground(true);
    widget->fgLabel->setText("FlightGear Disconnected");
    qxtLog->info("HITL: FlighGear disconnected");
}


