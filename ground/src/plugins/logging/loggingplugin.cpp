/**
 ******************************************************************************
 *
 * @file       logging.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @brief      Import/Export Plugin
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup   Logging
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

#include "loggingplugin.h"
#include "logginggadgetfactory.h"
#include <QDebug>
#include <QtPlugin>
#include <QThread>
#include <QStringList>
#include <QDir>
#include <QFileDialog>
#include <QList>
#include <QErrorMessage>
#include <QWriteLocker>

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icore.h>
#include <QKeySequence>
#include <uavobjects/uavobjectmanager.h>

/**
  * Sets the file to use for logging and takes the parent plugin
  * to connect to stop logging signal
  * @param[in] file File name to write to
  * @param[in] parent plugin
  */
bool LoggingThread::openFile(QString file, LoggingPlugin * parent)
{
    logFile.setFileName(file);
    logFile.open(QIODevice::WriteOnly);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    uavTalk = new UAVTalk(&logFile, objManager);
    connect(parent,SIGNAL(stopLoggingSignal()),this,SLOT(stopLogging()));

    return true;
};

/**
  * Logs an object update to the file.  Data format is the
  * timestamp as a 32 bit uint counting ms from start of
  * file writing (flight time will be embedded in stream),
  * then object packet size, then the packed UAVObject.
  */
void LoggingThread::objectUpdated(UAVObject * obj)
{
    QWriteLocker locker(&lock);
    if(!uavTalk->sendObject(obj,false,false) )
        qDebug() << "Error logging " << obj->getName();
};

/**
  * Connect signals from all the objects updates to the write routine then
  * run event loop
  */
void LoggingThread::run()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    QList< QList<UAVObject*> > list;
    list = objManager->getObjects();
    QList< QList<UAVObject*> >::const_iterator i;
    QList<UAVObject*>::const_iterator j;
    int objects = 0;

    for (i = list.constBegin(); i != list.constEnd(); ++i)
    {
        for (j = (*i).constBegin(); j != (*i).constEnd(); ++j)
        {
            connect(*j, SIGNAL(objectUpdated(UAVObject*)), (LoggingThread*) this, SLOT(objectUpdated(UAVObject*)));
            objects++;
            //qDebug() << "Detected " << j[0];
        }
    }

    exec();
}


/**
  * Pass this command to the correct thread then close the file
  */
void LoggingThread::stopLogging()
{
    QWriteLocker locker(&lock);
    logFile.close();
    qDebug() << "File closed";
    quit();
}

LoggingPlugin::LoggingPlugin() : state(IDLE)
{
    // Do nothing
}

LoggingPlugin::~LoggingPlugin()
{
    // Do nothing
}

/**
  * Add Logging plugin to File menu
  */
bool LoggingPlugin::initialize(const QStringList& args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);

    // Add Menu entry
    Core::ActionManager* am = Core::ICore::instance()->actionManager();
    Core::ActionContainer* ac = am->actionContainer(Core::Constants::M_TOOLS);

    // Command to start logging
    Core::Command* cmd = am->registerAction(new QAction(this),
                                            "LoggingPlugin.Logging",
                                            QList<int>() <<
                                            Core::Constants::C_GLOBAL_ID);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+L"));
    cmd->action()->setText("Logging...");

    ac->menu()->addSeparator();
    ac->appendGroup("Logging");
    ac->addAction(cmd, "Logging");

    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(toggleLogging()));

    // Command to replay logging
    Core::Command* cmd2 = am->registerAction(new QAction(this),
                                            "LoggingPlugin.Playback",
                                            QList<int>() <<
                                            Core::Constants::C_GLOBAL_ID);
    cmd2->setDefaultKeySequence(QKeySequence("Ctrl+R"));
    cmd2->action()->setText("Replay...");

    ac->appendGroup("Replay");
    ac->addAction(cmd2, "Replay");

    connect(cmd2->action(), SIGNAL(triggered(bool)), this, SLOT(toggleReplay()));

    mf = new LoggingGadgetFactory(this);
    addAutoReleasedObject(mf);

    // Map signal from end of replay to replay stopped
    connect(&logFile,SIGNAL(replayFinished()), this, SLOT(replayStopped()));

    return true;
}

/**
  * The action that is triggered by the menu item which opens the
  * file and begins logging if successful
  */
void LoggingPlugin::toggleLogging()
{
    if(state == IDLE)
    {
        QFileDialog * fd = new QFileDialog();
        fd->setAcceptMode(QFileDialog::AcceptSave);
        fd->setNameFilter("OpenPilot Log (*.opl)");
        connect(fd, SIGNAL(fileSelected(QString)), this, SLOT(startLogging(QString)));
        fd->exec();
    }
    else if(state == LOGGING)
    {
        stopLogging();
    }
}

/**
  * The action that is triggered by the menu item which opens the
  * file and begins replay if successful
  */
void LoggingPlugin::toggleReplay()
{
    if(state == IDLE)
    {
        QFileDialog * fd = new QFileDialog();
        fd->setAcceptMode(QFileDialog::AcceptOpen);
        fd->setNameFilter("OpenPilot Log (*.opl)");
        connect(fd, SIGNAL(fileSelected(QString)), this, SLOT(startReplay(QString)));
        fd->exec();
    }
    else if(state == REPLAY)
    {
        logFile.stopReplay();
    }
}


/**
  * Starts the logging thread to a certain file
  */
void LoggingPlugin::startLogging(QString file)
{
    qDebug() << "Logging to " << file;
    loggingThread = new LoggingThread();
    if(loggingThread->openFile(file,this))
    {
        connect(loggingThread,SIGNAL(finished()),this,SLOT(loggingStopped()));
        state = LOGGING;
        loggingThread->start();
        emit stateChanged("LOGGING");
    } else {
        QErrorMessage err;
        err.showMessage("Unable to open file for logging");
        err.exec();
    }
}

/**
  * Starts the logging thread replaying a certain file
  */
void LoggingPlugin::startReplay(QString file)
{

    logFile.setFileName(file);
    if(logFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Replaying " << file;
        state = REPLAY;

        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

        uavTalk = new UAVTalk(&logFile, objManager);
        logFile.startReplay();

        emit stateChanged("REPLAY");
    } else {
        QErrorMessage err;
        err.showMessage("Unable to open file for replay");
        err.exec();
    }
}

/**
  * Send the stop logging signal to the LoggingThread
  */
void LoggingPlugin::stopLogging()
{
    emit stopLoggingSignal();
}


/**
  * Receive the logging stopped signal from the LoggingThread
  * and change status to not logging
  */
void LoggingPlugin::loggingStopped()
{
    if(state == LOGGING)
        state = IDLE;

    emit stateChanged("IDLE");

    free(loggingThread);
    loggingThread = NULL;
}

/**
  * Received the replay stoppedsignal from the LogFile
  */
void LoggingPlugin::replayStopped()
{
    Q_ASSERT(state == REPLAY);

    if(uavTalk)
        delete(uavTalk);

    logFile.close();

    uavTalk = 0;
    state = IDLE;

    emit stateChanged("IDLE");
}


void LoggingPlugin::extensionsInitialized()
{
    // Do nothing
}

void LoggingPlugin::shutdown()
{
    // Do nothing
}
Q_EXPORT_PLUGIN(LoggingPlugin)

/**
 * @}
 * @}
 */
