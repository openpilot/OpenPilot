/**
 ******************************************************************************
 *
 * @file       plugin1.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   
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

#include "plugin1.h"

#include <extensionsystem/pluginmanager.h>

#include <QtCore/qplugin.h>
#include <QtCore/QObject>

using namespace Plugin1;

MyPlugin1::MyPlugin1()
    : initializeCalled(false)
{
}

bool MyPlugin1::initialize(const QStringList & /*arguments*/, QString *errorString)
{
    initializeCalled = true;
    QObject *obj = new QObject(this);
    obj->setObjectName("MyPlugin1");
    addAutoReleasedObject(obj);

    bool found2 = false;
    bool found3 = false;
    foreach (QObject *object, ExtensionSystem::PluginManager::instance()->allObjects()) {
        if (object->objectName() == "MyPlugin2")
            found2 = true;
        else if (object->objectName() == "MyPlugin3")
            found3 = true;
    }
    if (found2 && found3)
        return true;
    if (errorString) {
        QString error = "object(s) missing from plugin(s):";
        if (!found2)
            error.append(" plugin2");
        if (!found3)
            error.append(" plugin3");
        *errorString = error;
    }
    return false;
}

void MyPlugin1::extensionsInitialized()
{
    if (!initializeCalled)
        return;
    // don't do this at home, it's just done here for the test
    QObject *obj = new QObject(this);
    obj->setObjectName("MyPlugin1_running");
    addAutoReleasedObject(obj);
}

Q_EXPORT_PLUGIN(MyPlugin1)
