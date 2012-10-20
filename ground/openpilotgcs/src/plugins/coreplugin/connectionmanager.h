/**
 ******************************************************************************
 *
 * @file       connectionmanager.h
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

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QWidget>
#include "mainwindow.h"
#include "generalsettings.h"
#include "telemetrymonitorwidget.h"
#include <coreplugin/iconnection.h>
#include <QtCore/QVector>
#include <QtCore/QIODevice>
#include <QtCore/QLinkedList>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>

#include "core_global.h"
#include <QTimer>

QT_BEGIN_NAMESPACE
class QTabWidget;
QT_END_NAMESPACE

namespace Core {

    class IConnection;

namespace Internal {
    class FancyTabWidget;
    class FancyActionBar;
    class MainWindow;
} // namespace Internal


class DevListItem
{
public:
    DevListItem(IConnection *c, IConnection::device d) :
        connection(c), device(d) { }

    DevListItem() : connection(NULL) { }

    QString getConName() {
        if (connection == NULL)
            return "";
        return connection->shortName() + ": " + device.displayName;
    }

    bool operator==(const DevListItem &rhs) {
        return connection == rhs.connection && device == rhs.device;
    }

    IConnection *connection;
    IConnection::device device;
};


class CORE_EXPORT ConnectionManager : public QWidget
{
    Q_OBJECT

public:
    ConnectionManager(Internal::MainWindow *mainWindow, QTabWidget *modeStack);
    virtual ~ConnectionManager();

    void init();

    QIODevice* getCurrentConnection() { return m_ioDev; }
    DevListItem getCurrentDevice() { return m_connectionDevice; }
    DevListItem findDevice(const QString &devName);

    QLinkedList<DevListItem> getAvailableDevices() { return m_devList; }

    bool isConnected() { return m_ioDev != 0; }

    bool connectDevice(DevListItem device);
    bool disconnectDevice();
    void suspendPolling();
    void resumePolling();

protected:
    void updateConnectionList(IConnection *connection);
    void registerDevice(IConnection *conn, IConnection::device device);
    void updateConnectionDropdown();

signals:
    void deviceConnected(QIODevice *device);
    void deviceAboutToDisconnect();
    void deviceDisconnected();
    void availableDevicesChanged(const QLinkedList<Core::DevListItem> devices);

public slots:
    void telemetryConnected();
    void telemetryDisconnected();
    void telemetryUpdated(double txRate, double rxRate);

private slots:
    void objectAdded(QObject *obj);
    void aboutToRemoveObject(QObject *obj);

    void onConnectClicked();
    void devChanged(IConnection *connection);

    void onConnectionDestroyed(QObject *obj);
    void connectionsCallBack(); //used to call devChange after all the plugins are loaded
    void reconnectSlot();
    void reconnectCheckSlot();

protected:
    QComboBox *m_availableDevList;
    QPushButton *m_connectBtn;
    QLinkedList<DevListItem> m_devList;
    QList<IConnection*> m_connectionsList;

    //tx/rx telemetry monitor
    TelemetryMonitorWidget* m_monitorWidget;

    //currently connected connection plugin
    DevListItem m_connectionDevice;

    //currently connected QIODevice
    QIODevice *m_ioDev;

private:
	bool connectDevice();
    bool polling;
    Internal::MainWindow *m_mainWindow;
    QList <IConnection *> connectionBackup;
    QTimer *reconnect;
    QTimer *reconnectCheck;

};

} //namespace Core

#endif // CONNECTIONMANAGER_H
