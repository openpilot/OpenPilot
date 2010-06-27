/**
 ******************************************************************************
 *
 * @file       hitlil2widget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlil2plugin
 * @{
 *
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
#include "hitlil2widget.h"
#include "ui_hitlil2widget.h"
#include "qxtlogger.h"
#include <QDebug>
#include "uavobjects/uavobjectmanager.h"

HITLIL2Widget::HITLIL2Widget(QWidget *parent) : QWidget(parent)
{
    il2Process = NULL;
    widget = new Ui_HITLIL2Widget();
    widget->setupUi(this);
    connect(widget->startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    connect(widget->stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
}

HITLIL2Widget::~HITLIL2Widget()
{
   delete widget;
}

void HITLIL2Widget::startButtonClicked()
{
    // Stop running process if one is active
    if (il2Process != NULL)
    {
        stopButtonClicked();
    }

    // Setup process
    widget->textBrowser->append(QString("Connecting to IL2 ...\n"));

    // Start bridge
    qxtLog->info("HITLIL2: Starting bridge, initializing IL2 and Autopilot connections");
    il2Bridge = new IL2Bridge(il2HostName,il2Port);
    connect(il2Bridge, SIGNAL(autopilotConnected()), this, SLOT(onAutopilotConnect()));
    connect(il2Bridge, SIGNAL(autopilotDisconnected()), this, SLOT(onAutopilotDisconnect()));
    connect(il2Bridge, SIGNAL(il2Connected()), this, SLOT(onFGConnect()));
    connect(il2Bridge, SIGNAL(il2Disconnected()), this, SLOT(onFGDisconnect()));

    // Initialize connection status
    if ( il2Bridge->isAutopilotConnected() )
    {
        onAutopilotConnect();
    }
    else
    {
        onAutopilotDisconnect();
    }
    if ( il2Bridge->isFGConnected() )
    {
        onFGConnect();
    }
    else
    {
        onFGDisconnect();
    }
}

void HITLIL2Widget::stopButtonClicked()
{
    // NOTE: Does not currently work, may need to send control+c to through the terminal
    if (il2Process != NULL)
    {
        il2Process->disconnect(this);
        il2Process->kill();
        delete il2Process;
        il2Bridge->disconnect(this);
        delete il2Bridge;
        il2Process = NULL;
    }
}

void HITLIL2Widget::setIl2HostName(QString il2HostName)
{
    this->il2HostName = il2HostName;
}

void HITLIL2Widget::setIl2Port(int il2Port)
{
    this->il2Port = il2Port;
}

void HITLIL2Widget::setFGManualControl(bool val)
{
    this->il2ManualControl = val;
}

void HITLIL2Widget::processReadyRead()
{
    QByteArray bytes = il2Process->readAllStandardOutput();
    QString str(bytes);
    if ( !str.contains("Error reading data") ) // ignore error
    {
        widget->textBrowser->append(str);
    }
}

void HITLIL2Widget::onAutopilotConnect()
{
    QPalette pal(widget->apLabel->palette());
    pal.setColor(QPalette::Window, Qt::green);
    widget->apLabel->setPalette(pal);
    widget->apLabel->setAutoFillBackground(true);
    widget->apLabel->setText("AutoPilot Connected");
    qxtLog->info("HITL-IL2: Autopilot connected, initializing for HITL simulation");
}

void HITLIL2Widget::onAutopilotDisconnect()
{
    QPalette pal(widget->apLabel->palette());
    pal.setColor(QPalette::Window, Qt::red);
    widget->apLabel->setPalette(pal);
    widget->apLabel->setAutoFillBackground(true);
    widget->apLabel->setText("AutoPilot Disconnected");
    qxtLog->info("HITL-IL2: Autopilot disconnected");
}

void HITLIL2Widget::onIl2Connect()
{
    QPalette pal(widget->il2Label->palette());
    pal.setColor(QPalette::Window, Qt::green);
    widget->il2Label->setPalette(pal);
    widget->il2Label->setAutoFillBackground(true);
    widget->il2Label->setText("IL2 Connected");
    qxtLog->info("HITL-IL2: IL2 connected");
}

void HITLIL2Widget::onFGDisconnect()
{
    QPalette pal(widget->il2Label->palette());
    pal.setColor(QPalette::Window, Qt::red);
    widget->il2Label->setPalette(pal);
    widget->il2Label->setAutoFillBackground(true);
    widget->il2Label->setText("IL2 Disconnected");
    qxtLog->info("HITL-IL2: IL2 disconnected");
}


