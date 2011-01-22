/**
 ******************************************************************************
 *
 * @file       plugin3.h
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

#ifndef PLUGIN3_H
#define PLUGIN3_H

#include <extensionsystem/iplugin.h>

#include <QtCore/QObject>

namespace Plugin3 {

class MyPlugin3 : public ExtensionSystem::IPlugin
{
    Q_OBJECT

public:
    MyPlugin3();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
};

} // namespace Plugin3

#endif // PLUGIN3_H
