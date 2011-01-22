/**
 ******************************************************************************
 *
 * @file       actioncontainer.h
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

#ifndef ACTIONCONTAINER_H
#define ACTIONCONTAINER_H

#include <QtCore/QObject>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>

namespace Core {

class Command;

class ActionContainer : public QObject
{
public:
    enum EmptyAction {
        EA_Mask             = 0xFF00,
        EA_None             = 0x0100,
        EA_Hide             = 0x0200,
        EA_Disable          = 0x0300
    };

    virtual void setEmptyAction(EmptyAction ea) = 0;

    virtual int id() const = 0;

    virtual QMenu *menu() const = 0;
    virtual QMenuBar *menuBar() const = 0;

    virtual QAction *insertLocation(const QString &group) const = 0;
    virtual void appendGroup(const QString &group) = 0;
    virtual void addAction(Core::Command *action, const QString &group = QString()) = 0;
    virtual void addMenu(Core::ActionContainer *menu, const QString &group = QString()) = 0;

    virtual bool update() = 0;
    virtual ~ActionContainer() {}
};

} // namespace Core

#endif // ACTIONCONTAINER_H
