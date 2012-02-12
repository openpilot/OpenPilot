/**
 ******************************************************************************
 *
 * @file       welcomeplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup WelcomePlugin Welcome Plugin
 * @{
 * @brief The GCS Welcome plugin
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

#include "welcomeplugin.h"

#include "welcomemode.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/basemode.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>

#include <QtCore/QDebug>
#include <QtCore/QtPlugin>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

using namespace Welcome::Internal;

WelcomePlugin::WelcomePlugin()
  : m_welcomeMode(0)
{
}

WelcomePlugin::~WelcomePlugin()
{
    if (m_welcomeMode) {
        removeObject(m_welcomeMode);
        delete m_welcomeMode;
    }
}

/*! Initializes the plugin. Returns true on success.
    Plugins want to register objects with the plugin manager here.

    \a error_message can be used to pass an error message to the plugin system,
       if there was any.
*/
bool WelcomePlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments)
    Q_UNUSED(error_message)

    m_welcomeMode = new WelcomeMode;
    addObject(m_welcomeMode);

    return true;
}

/*! Notification that all extensions that this plugin depends on have been
    initialized. The dependencies are defined in the plugins .qwp file.

    Normally this method is used for things that rely on other plugins to have
    added objects to the plugin manager, that implement interfaces that we're
    interested in. These objects can now be requested through the
    PluginManagerInterface.

    The WelcomePlugin doesn't need things from other plugins, so it does
    nothing here.
*/
void WelcomePlugin::extensionsInitialized()
{
    Core::ModeManager::instance()->activateMode(m_welcomeMode->uniqueModeName());
}

Q_EXPORT_PLUGIN(WelcomePlugin)
