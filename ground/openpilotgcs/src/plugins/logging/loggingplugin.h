/**
 ******************************************************************************
 * @file       loggingplugin.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup loggingplugin
 * @{
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

#ifndef LOGGINGPLUGIN_H_
#define LOGGINGPLUGIN_H_

#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/iconnection.h>
#include <extensionsystem/iplugin.h>
#include "uavobjectmanager.h"
#include "gcstelemetrystats.h"
#include <uavtalk/uavtalk.h>
#include <logfile.h>

#include <QThread>
#include <QQueue>
#include <QReadWriteLock>

class LoggingPlugin;
class LoggingGadgetFactory;

/**
*   Define a connection via the IConnection interface
*   Plugin will add a instance of this class to the pool,
*   so the connection manager can use it.
*/
class LoggingConnection
    : public Core::IConnection
{
    Q_OBJECT
public:
    LoggingConnection();
    virtual ~LoggingConnection();

    virtual QList <Core::IConnection::device> availableDevices();
    virtual QIODevice *openDevice(const QString &deviceName);
    virtual void closeDevice(const QString &deviceName);

    virtual QString connectionName();
    virtual QString shortName();

    bool deviceOpened() {return m_deviceOpened;}
    LogFile* getLogfile() { return &logFile;}


private:
    LogFile logFile;


protected slots:
    void onEnumerationChanged();
    void startReplay(QString file);

protected:
    bool m_deviceOpened;
};



class LoggingThread : public QThread
{
Q_OBJECT
public:
    bool openFile(QString file, LoggingPlugin * parent);

private slots:
    void objectUpdated(UAVObject * obj);
    void transactionCompleted(UAVObject* obj, bool success);

public slots:
    void stopLogging();

protected:
    void run();
    QReadWriteLock lock;
    LogFile logFile;
    UAVTalk * uavTalk;

private:
    QQueue<UAVDataObject*> queue;

    void retrieveSettings();
    void retrieveNextObject();

};

class LoggingPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT

public:
    LoggingPlugin();
    ~LoggingPlugin();

    void extensionsInitialized();
    bool initialize(const QStringList & arguments, QString * errorString);
    void shutdown();

    LoggingConnection* getLogConnection() { return logConnection; };
    LogFile* getLogfile() { return logConnection->getLogfile();}
    void setLogMenuTitle(QString str);


signals:
    void stopLoggingSignal(void);
    void stopReplaySignal(void);
    void stateChanged(QString);


protected:
    enum {IDLE, LOGGING, REPLAY} state;
    LoggingThread * loggingThread;

    // These are used for replay, logging in its own thread
    LoggingConnection* logConnection;

private slots:
    void toggleLogging();
    void startLogging(QString file);
    void stopLogging();
    void loggingStopped();
    void replayStarted();
    void replayStopped();

private:
    LoggingGadgetFactory *mf;
    Core::Command* cmd;

};
#endif /* LoggingPLUGIN_H_ */
/**
 * @}
 * @}
 */
