/**
 ******************************************************************************
 *
 * @file       messagemanager.h
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

#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include "core_global.h"
#include <QtCore/QObject>

namespace Core {

namespace Internal {
class MessageOutputWindow;
}

class CORE_EXPORT MessageManager : public QObject
{
    Q_OBJECT

public:
    MessageManager();
    ~MessageManager();

    void init();

    static MessageManager *instance() { return m_instance; }

    void displayStatusBarMessage(const QString &text, int ms = 0);
    void showOutputPane();

public slots:
    void printToOutputPane(const QString &text, bool bringToForeground);
    void printToOutputPanePopup(const QString &text); // pops up
    void printToOutputPane(const QString &text);

private:
    Internal::MessageOutputWindow *m_messageOutputWindow;

    static MessageManager *m_instance;
};

} // namespace Core

#endif // MESSAGEMANAGER_H
