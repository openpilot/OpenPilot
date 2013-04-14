/**
 ******************************************************************************
 *
 * @file       eventfilteringmainwindow.h
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

#ifndef EVENTFILTERINGMAINWINDOW_H
#define EVENTFILTERINGMAINWINDOW_H

#include <QtGui/QMainWindow>

namespace Core {
namespace Internal {

/*!
 * This class only exists because we can't include windows.h in mainwindow.cpp
 * because windows defines an IContext...
 */

class EventFilteringMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    EventFilteringMainWindow();

#ifdef Q_OS_WIN
protected:
    bool winEvent(MSG *message, long *result);
#endif

signals:
    void deviceChange();

};

} // Internal
} // Core

#endif // EVENTFILTERINGMAINWINDOW_H
