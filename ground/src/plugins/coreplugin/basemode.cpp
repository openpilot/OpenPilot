/**
 ******************************************************************************
 *
 * @file       basemode.cpp
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

#include "basemode.h"

#include <extensionsystem/pluginmanager.h>

#include <QtGui/QWidget>

using namespace Core;

/*!
    \class BaseMode
    \mainclass
    \inheaderfile basemode.h
    \brief A base implementation of the mode interface IMode.

    The BaseMode class can be used directly for most IMode implementations. It has setter functions
    for the mode properties and a convenience constructor.

    The ownership of the widget is given to the BaseMode, so when the BaseMode is destroyed it
    deletes its widget.

    A typical use case is to do the following in the init method of a plugin:
    \code
    bool MyPlugin::init(QString *error_message)
    {
        [...]
        addObject(new Core::BaseMode("mymode",
            "MyPlugin.UniqueModeName",
            icon,
            50, // priority
            new MyWidget));
        [...]
    }
    \endcode
*/

/*!
    \fn BaseMode::BaseMode(QObject *parent)

    Creates a mode with empty name, no icon, lowest priority and no widget. You should use the
    setter functions to give the mode a meaning.

    \a parent
*/
BaseMode::BaseMode(QObject *parent):
    IMode(parent),
    m_priority(0),
    m_widget(0)
{
}

/*!
    \fn BaseMode::~BaseMode()
*/
BaseMode::~BaseMode()
{
    delete m_widget;
}
