/**
 ******************************************************************************
 *
 * @file       connectionmanager.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief The Core GCS plugin
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
#include "qextserialport/src/qextserialenumerator.h"
#include "qextserialport/src/qextserialport.h"
#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QComboBox>
#include <QEventLoop>

namespace Core {


ConnectionManager::ConnectionManager(Internal::MainWindow *mainWindow, QTabWidget *modeStack) :
    QWidget(mainWindow),
    m_availableDevList(0),
    m_connectBtn(0),
    m_ioDev(NULL),
    polling(true),
    m_mainWindow(mainWindow)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(5);
    layout->setContentsMargins(5,2,5,2);

    m_monitorWidget = new TelemetryMonitorWidget(this);
    layout->addWidget(m_monitorWidget, Qt::AlignHCenter);

    layout->addWidget(new QLabel(tr("Connections:")));

    m_availableDevList = new QComboBox;
    m_availableDevList->setMinimumWidth(100);
    m_availableDevList->setMaximumWidth(150);
    m_availableDevList->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_availableDevList);

    m_connectBtn = new QPushButton(tr("Connect"));
    m_connectBtn->setEnabled(false);
    layout->addWidget(m_connectBtn);

    setLayout(layout);

    modeStack->setCornerWidget(this, Qt::TopRightCorner);

    QObject::connect(m_connectBtn, SIGNAL(clicked()), this, SLOT(onConnectClicked()));

    // setup our reconnect timers
    reconnect = new QTimer(this);
    reconnectCheck = new QTimer(this);
    connect(reconnect,SIGNAL(timeout()),this,SLOT(reconnectSlot()));
    connect(reconnectCheck,SIGNAL(timeout()),this,SLOT(reconnectCheckSlot()));
}

ConnectionManager::~ConnectionManager()
{
    disconnectDevice();
    suspendPolling();
    if (m_monitorWidget)
        delete m_monitorWidget;
}

void ConnectionManager::init()
{
    //register to the plugin manager so we can receive
    //new connection object from plugins
    QObject::connect(ExtensionSystem::PluginManager::instance(), SIGNAL(objectAdded(QObject*)), this, SLOT(objectAdded(QObject*)));
    QObject::connect(ExtensionSystem::PluginManager::instance(), SIGNAL(aboutToRemoveObject(QObject*)), this, SLOT(aboutToRemoveObject(QObject*)));
}

/**
*   Method called when the user clicks the "Connect" button
*/
bool ConnectionManager::connectDevice(DevListItem device)
{
    DevListItem connection_device = findDevice(m_availableDevList->itemData(m_availableDevList->currentIndex(),Qt::ToolTipRole).toString());
    if (!connection_device.connection)
        return false;

    QIODevice *io_dev = connection_device.connection->openDevice(connection_device.device.name);
    if (!io_dev)
        return false;

    io_dev->open(QIODevice::ReadWrite);

    // check if opening the device worked
    if (!io_dev->isOpen()) {
        return false;
    }

    // we appear to have connected to the device OK
    // remember the connection/device details
    m_connectionDevice = connection_device;
    m_ioDev = io_dev;

    connect(m_connectionDevice.connection, SIGNAL(destroyed(QObject *)), this, SLOT(onConnectionDestroyed(QObject *)), Qt::QueuedConnection);

    // signal interested plugins that we connected to the device
    emit deviceConnected(io_dev);
    m_connectBtn->setText("Disconnect");
    m_availableDevList->setEnabled(false);

    // tell the monitorwidget we're conneced
    m_monitorWidget->connect();

    return true;
}

/**
*   Method called by plugins who want to force a disconnection.
*   Used by Uploader gadget for instance.
*/
bool ConnectionManager::disconnectDevice()
{
    // tell the monitor widget we're disconnected
    m_monitorWidget->disconnect();

    if (!m_ioDev) {
        // apparently we are already disconnected: this can
        // happen if a plugin tries to force a disconnect whereas
        // we are not connected. Just return.
        return false;
    }

    // We are connected - disconnect from the device

    // stop our timers
    if(reconnect->isActive())
        reconnect->stop();
    if(reconnectCheck->isActive())
        reconnectCheck->stop();

    // signal interested plugins that user is disconnecting his device
    emit deviceAboutToDisconnect();

    try {
        if (m_connectionDevice.connection) {
            m_connectionDevice.connection->closeDevice(m_connectionDevice.getConName());
        }
    } catch (...) {	// handle exception
        qDebug() << "Exception: m_connectionDevice.connection->closeDevice(" << m_connectionDevice.getConName() << ")";
    }

    m_connectionDevice.connection = NULL;
    m_ioDev = NULL;

    emit deviceDisconnected();
    m_connectBtn->setText("Connect");
    m_availableDevList->setEnabled(true);

    return true;
}

/**
*   Slot called when a plugin added an object to the core pool
*/
void ConnectionManager::objectAdded(QObject *obj)
{
    //Check if a plugin added a connection object to the pool
    IConnection *connection = Aggregation::query<IConnection>(obj);
    if (!connection) return;

    //register devices and populate CB
    devChanged(connection);

    // Keep track of the registration to be able to tell plugins
    // to do things
    m_connectionsList.append(connection);

    QObject::connect(connection, SIGNAL(availableDevChanged(IConnection *)), this, SLOT(devChanged(IConnection *)));
}

void ConnectionManager::aboutToRemoveObject(QObject *obj)
{
    //Check if a plugin added a connection object to the pool
    IConnection *connection = Aggregation::query<IConnection>(obj);
    if (!connection) return;

    if (m_connectionDevice.connection && m_connectionDevice.connection == connection)
    {	// we are currently using the one that is about to be removed
        disconnectDevice();
        m_connectionDevice.connection = NULL;
        m_ioDev = NULL;
    }

    if (m_connectionsList.contains(connection))
        m_connectionsList.removeAt(m_connectionsList.indexOf(connection));
}


void ConnectionManager::onConnectionDestroyed(QObject *obj)
{
    Q_UNUSED(obj)
    disconnectDevice();
}

/**
*   Slot called when the user clicks the connect/disconnect button
*/
void ConnectionManager::onConnectClicked()
{
    // Check if we have a ioDev already created:
    if (!m_ioDev)
    {
        // connecting to currently selected device
        DevListItem device = findDevice(m_availableDevList->itemData(m_availableDevList->currentIndex(), Qt::ToolTipRole).toString());
        if (device.connection)
            connectDevice(device);
    }
    else
    {	// disconnecting
        disconnectDevice();
    }
}

/**
*   Slot called when the telemetry is connected
*/
void ConnectionManager::telemetryConnected()
{
    qDebug() << "TelemetryMonitor: connected";

    if(reconnectCheck->isActive())
        reconnectCheck->stop();

    //tell the monitor we're connected
    m_monitorWidget->connect();
}

/**
*   Slot called when the telemetry is disconnected
*/
void ConnectionManager::telemetryDisconnected()
{
    qDebug() << "TelemetryMonitor: disconnected";

    if (m_ioDev){
        if(m_connectionDevice.connection->shortName()=="Serial") {
            if(!reconnect->isActive())
                reconnect->start(1000);
        }
    }

    //tell the monitor we're disconnected
    m_monitorWidget->disconnect();
}

/**
*   Slot called when the telemetry rates are updated
*/
void ConnectionManager::telemetryUpdated(double txRate, double rxRate)
{
    m_monitorWidget->updateTelemetry(txRate, rxRate);
}

void ConnectionManager::reconnectSlot()
{
    qDebug()<<"reconnect";
    if(m_ioDev->isOpen())
        m_ioDev->close();

    if(m_ioDev->open(QIODevice::ReadWrite)) {
        qDebug()<<"reconnect successfull";
        reconnect->stop();
        reconnectCheck->start(20000);
    }
    else
        qDebug()<<"reconnect NOT successfull";
}

void ConnectionManager::reconnectCheckSlot()
{
    reconnectCheck->stop();
    reconnect->start(1000);
}

/**
*   Find a device by its displayed (visible on screen) name
*/
DevListItem ConnectionManager::findDevice(const QString &devName)
{
    foreach (DevListItem d, m_devList)
    {
        if (d.getConName() == devName)
            return d;
    }

    qDebug() << "findDevice: cannot find " << devName << " in device list";

    DevListItem d;
    d.connection = NULL;
    return d;
}

/**
*   Tells every connection plugin to stop polling for devices if they
*   are doing that.
*
*/
void ConnectionManager::suspendPolling()
{
    foreach (IConnection *cnx, m_connectionsList)
    {
        cnx->suspendPolling();
    }

    m_connectBtn->setEnabled(false);
    m_availableDevList->setEnabled(false);
    polling = false;
}

/**
*   Tells every connection plugin to resume polling for devices if they
*   are doing that.
*/
void ConnectionManager::resumePolling()
{
    foreach (IConnection *cnx, m_connectionsList)
    {
        cnx->resumePolling();
    }

    m_connectBtn->setEnabled(true);
    m_availableDevList->setEnabled(true);
    polling = true;
}

/**
 * Synchronize the list of connections displayed with those physically
 * present
 * @param[in] connection Connection type that you want to forget about :)
 */
void ConnectionManager::updateConnectionList(IConnection *connection)
{
    // Get the updated list of devices
    QList <IConnection::device> availableDev = connection->availableDevices();

    // Go through the list of connections of that type.  If they are not in the
    // available device list then remove them.  If they are connected, then
    // disconnect them.
    for (QLinkedList<DevListItem>::iterator iter = m_devList.begin(); iter != m_devList.end(); )
    {
        if (iter->connection != connection) {
            ++iter;
            continue;
        }

        // See if device exists in the updated availability list
        bool found = availableDev.contains(iter->device);
        if (!found) {
            // we are currently using the one we are about to erase
            if (m_connectionDevice.connection && m_connectionDevice.connection == connection && m_connectionDevice.device == iter->device) {
                disconnectDevice();
            }

            iter = m_devList.erase(iter);
        } else
            ++iter;
    }

    // Add any back to list that don't exist
    foreach (IConnection::device dev, availableDev)
    {
        bool found = m_devList.contains(DevListItem(connection, dev));
        if (!found) {
            registerDevice(connection,dev);
        }
    }
}

/**
*   Register a device from a specific connection plugin
*   @param devN contains the connection shortname + device name which is diplayed in the tooltip
*  @param disp is the name that is displayed in the dropdown menu
*  @param name is the actual device name
*/
void ConnectionManager::registerDevice(IConnection *conn, IConnection::device device)
{
    DevListItem d(conn,device);
    m_devList.append(d);
}

/**
*   A device plugin notified us that its device list has changed
*   (eg: user plug/unplug a usb device)
*   \param[in] connection Connection type which signaled the update
*/
void ConnectionManager::devChanged(IConnection *connection)
{
    if(!ExtensionSystem::PluginManager::instance()->allPluginsLoaded())
    {
        connectionBackup.append(connection);
        connect(ExtensionSystem::PluginManager::instance(),SIGNAL(pluginsLoadEnded()),this,SLOT(connectionsCallBack()),Qt::UniqueConnection);
        return;
    }
    //clear device list combobox
    m_availableDevList->clear();

    //remove registered devices of this IConnection from the list
    updateConnectionList(connection);

    updateConnectionDropdown();

    qDebug() << "# devices " << m_devList.count();
    emit availableDevicesChanged(m_devList);


    //disable connection button if the liNameif (m_availableDevList->count() > 0)
    if (m_availableDevList->count() > 0)
        m_connectBtn->setEnabled(true);
    else
        m_connectBtn->setEnabled(false);
}

void ConnectionManager::updateConnectionDropdown()
{
    //add all the list again to the combobox
    foreach (DevListItem d, m_devList)
    {
        m_availableDevList->addItem(d.getConName());
        m_availableDevList->setItemData(m_availableDevList->count()-1, d.getConName(), Qt::ToolTipRole);
        if(!m_ioDev && d.getConName().startsWith("USB"))
        {
            if(m_mainWindow->generalSettings()->autoConnect() || m_mainWindow->generalSettings()->autoSelect())
                m_availableDevList->setCurrentIndex(m_availableDevList->count() - 1);

            if(m_mainWindow->generalSettings()->autoConnect() && polling)
            {
                qDebug() << "Automatically opening device";
                connectDevice(d);
                qDebug()<<"ConnectionManager::devChanged autoconnected USB device";
            }
        }
    }
    if(m_ioDev)//if a device is connected make it the one selected on the dropbox
    {
        for(int x=0;x<m_availableDevList->count();++x)
        {
            if(m_connectionDevice.getConName()==m_availableDevList->itemData(x,Qt::ToolTipRole).toString())
                m_availableDevList->setCurrentIndex(x);
        }
    }
}

void Core::ConnectionManager::connectionsCallBack()
{
    foreach(IConnection * con,connectionBackup)
    {
        devChanged(con);
    }
    connectionBackup.clear();
    disconnect(ExtensionSystem::PluginManager::instance(),SIGNAL(pluginsLoadEnded()),this,SLOT(connectionsCallBack()));
}

} //namespace Core
