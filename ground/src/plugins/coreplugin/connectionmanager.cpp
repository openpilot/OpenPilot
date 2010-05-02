/**
 ******************************************************************************
 *
 * @file       connectionmanager.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   coreplugin
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

#include "connectionmanager.h"

#include <aggregation/aggregate.h>
#include <coreplugin/iconnection.h>
#include <extensionsystem/pluginmanager.h>

#include <utils/styledbar.h>

#include "fancytabwidget.h"
#include "fancyactionbar.h"
#include "mainwindow.h"
#include "qextserialport/src/qextserialenumerator.h"
#include "qextserialport/src/qextserialport.h"

#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QComboBox>

namespace Core {


ConnectionManager::ConnectionManager(Internal::MainWindow *mainWindow, Internal::FancyTabWidget *modeStack)
    : m_availableDevList(0),
    m_connectBtn(0),
    m_ioDev(NULL)
{
    QVBoxLayout *top = new QVBoxLayout;
    top->setSpacing(0);
    top->setMargin(0);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(5,0,5,0);
    layout->addWidget(new QLabel("Connections: "));

    m_availableDevList = new QComboBox;
    //m_availableDevList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_availableDevList->setMinimumWidth(100);
    m_availableDevList->setMaximumWidth(150);
    m_availableDevList->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_availableDevList);

    m_connectBtn = new QPushButton("Connect");
    m_connectBtn->setEnabled(false);
    layout->addWidget(m_connectBtn);

    Utils::StyledBar *bar = new Utils::StyledBar;
    bar->setLayout(layout);

    top->addWidget(bar);
    setLayout(top);

    modeStack->insertCornerWidget(modeStack->cornerWidgetCount()-1, this);

    QObject::connect(m_connectBtn, SIGNAL(pressed()),
                     this, SLOT(onConnectPressed()));
}

ConnectionManager::~ConnectionManager()
{
}

void ConnectionManager::init()
{
    //register to the plugin manager so we can receive
    //new connection object from plugins
    QObject::connect(ExtensionSystem::PluginManager::instance(), SIGNAL(objectAdded(QObject*)),
                     this, SLOT(objectAdded(QObject*)));
    QObject::connect(ExtensionSystem::PluginManager::instance(), SIGNAL(aboutToRemoveObject(QObject*)),
                     this, SLOT(aboutToRemoveObject(QObject*)));
}

/**
*   Slot called when a plugin added an object to the core pool
*/
void ConnectionManager::objectAdded(QObject *obj)
{
    //Check if a plugin added a connection object to the pool
    IConnection *connection = Aggregation::query<IConnection>(obj);
    if (!connection)
        return;

    //qDebug() << "Connection object registered:" << connection->connectionName();
    //qDebug() << connection->availableDevices();

    //register devices and populate CB
    devChanged(connection);

    QObject::connect(connection, SIGNAL(availableDevChanged(IConnection *)),
                     this, SLOT(devChanged(IConnection*)));
}

void ConnectionManager::aboutToRemoveObject(QObject *obj)
{
}

/**
*   Slot called when the user pressed the connect/disconnect button
*/
void ConnectionManager::onConnectPressed()
{
    //check if we are trying to connect a new device
    //or if we are disconnecting it
    if(!m_ioDev)
    {
        m_connectBtn->setText("Disconnect");
        m_availableDevList->setEnabled(false);

        m_connectionDevice = findDevice(m_availableDevList->currentText());

        if(m_connectionDevice.connection)
        {
            m_ioDev = m_connectionDevice.connection->openDevice(m_connectionDevice.devName);

            if(m_ioDev)
            {
                m_ioDev->open(QIODevice::ReadWrite);
                //signal interested plugins that the user wants to connect to the device
                emit deviceConnected(m_ioDev);
                return;
            }
            else
            {
                m_connectionDevice.connection = NULL;
            }
        }
    }

    //both in case of error and disconnection, we fall back here
    m_connectBtn->setText("Connect");
    m_availableDevList->setEnabled(true);

    //signal interested plugins that user is disconnecting his device
    emit deviceDisconnected();

    //close the device
    if(m_ioDev)
    {
        m_ioDev->close();

        //delete the object created by the IConnection class
        delete m_ioDev;
        m_ioDev = NULL;
    }

    if(m_connectionDevice.connection)
        m_connectionDevice.connection->closeDevice(m_connectionDevice.devName);

    m_connectionDevice.connection = NULL;
}

/**
*   Find a device by its displayed (visible on screen) name
*/
devListItem ConnectionManager::findDevice(const QString &displayedName)
{
    foreach(devListItem d, m_devList)
    {
        if(d.displayedName == displayedName)
        {
            return d;
        }
    }
    qDebug() << "findDevice: cannot find " << displayedName << " in device list";
    devListItem d;
    d.connection = NULL;
    return d;
}

/**
*   Unregister all devices from one connection plugin
*   \param[in] connection Connection type that you want to forget about :)
*/
void ConnectionManager::unregisterAll(IConnection *connection)
{
    for(QLinkedList<devListItem>::iterator iter = m_devList.begin();
    iter != m_devList.end(); )
    {
        if(iter->connection == connection)
        {
            iter = m_devList.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

/**
*   Register a device from a specific connection plugin
*/
void ConnectionManager::registerDevice(IConnection *conn, const QString &devN, const QString &disp)
{
    devListItem d;
    d.connection = conn;
    d.devName = devN;
    d.displayedName = disp;

    m_devList.append(d);
}

/**
*   A device plugin notified us that its device list has changed
*   (eg: user plug/unplug a usb device)
*   \param[in] connection Connection type which signaled the update
*/
void ConnectionManager::devChanged(IConnection *connection)
{
    //clear device list combobox
    m_availableDevList->clear();

    //remove registered devices of this IConnection from the list
    unregisterAll(connection);

    //and add them back in the list
    QStringList availableDev = connection->availableDevices();
    foreach(QString dev, availableDev)
    {
        QString cbName = connection->shortName() + ": " + dev;
        registerDevice(connection, dev, cbName);
    }

    //add all the list again to the combobox
    foreach(devListItem d, m_devList)
    {
        m_availableDevList->addItem(d.displayedName);
    }

    //disable connection button if the list is empty
    if(m_availableDevList->count() > 0)
        m_connectBtn->setEnabled(true);
    else
        m_connectBtn->setEnabled(false);
}


} //namespace Core
