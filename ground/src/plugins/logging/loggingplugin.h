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

#include <extensionsystem/iplugin.h>
#include <uavobjects/uavobjectmanager.h>
#include <QThread>
#include <QTime>
#include <QReadWriteLock>

class LoggingPlugin;

class LoggingThread : public QThread
{
Q_OBJECT
public:
    bool openFile(QString file, LoggingPlugin * parent);

private slots:
    void objectUpdated(UAVObject * obj);

public slots:
    void stopLogging();

protected:
    void run();
    QFile logFile;
    QTime myTime;
    QReadWriteLock lock;
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

signals:
    void stopLoggingSignal(void);

protected:
    enum {IDLE, LOGGING} state;
    LoggingThread * loggingThread;

private slots:
    void toggleLogging();
    void startLogging(QString file);
    void stopLogging();
    void loggingStopped();
};
#endif /* LoggingPLUGIN_H_ */
/**
 * @}
 * @}
 */
