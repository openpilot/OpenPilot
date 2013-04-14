/**
 ******************************************************************************
 *
 * @file       coreplugin.cpp
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

#include "coreplugin.h"
#include "coreimpl.h"
#include "mainwindow.h"
#include <QtPlugin>
#include <extensionsystem/pluginmanager.h>

#include <QtCore/QtPlugin>

using namespace Core::Internal;

CorePlugin::CorePlugin() :
    m_mainWindow(new MainWindow)
{
}

CorePlugin::~CorePlugin()
{
    delete m_mainWindow;
}

bool CorePlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    const bool success = m_mainWindow->init(errorMessage);
    if (success) {
        // nothing right now
    }
    return success;
}

void CorePlugin::extensionsInitialized()
{
    m_mainWindow->extensionsInitialized();
}

void CorePlugin::remoteArgument(const QString& arg)
{
    // An empty argument is sent to trigger activation
    // of the window via QtSingleApplication. It should be
    // the last of a sequence.
    if (arg.isEmpty()) {
        m_mainWindow->activateWindow();
    } else {
        //m_mainWindow->openFiles(QStringList(arg));
    }
}

void CorePlugin::shutdown()
{
    m_mainWindow->shutdown();
}

Q_EXPORT_PLUGIN2(Core,CorePlugin)
