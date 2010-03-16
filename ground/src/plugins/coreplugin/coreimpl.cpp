/**
 ******************************************************************************
 *
 * @file       coreimpl.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   coreplugin
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

#include "coreimpl.h"

#include <QtCore/QDir>
#include <QtCore/QCoreApplication>

namespace Core {
namespace Internal {

// The Core Singleton
static CoreImpl *m_instance = 0;

} // namespace Internal
} // namespace Core


using namespace Core;
using namespace Core::Internal;


ICore* ICore::instance()
{
    return m_instance;
}

CoreImpl::CoreImpl(MainWindow *mainwindow)
{
    m_instance = this;
    m_mainwindow = mainwindow;
}

bool CoreImpl::showOptionsDialog(const QString &group, const QString &page, QWidget *parent)
{
    return m_mainwindow->showOptionsDialog(group, page, parent);
}

bool CoreImpl::showWarningWithOptions(const QString &title, const QString &text,
                                      const QString &details,
                                      const QString &settingsCategory,
                                      const QString &settingsId,
                                      QWidget *parent)
{
    return m_mainwindow->showWarningWithOptions(title, text,
                                                details, settingsCategory,
                                                settingsId, parent);
}

ActionManager *CoreImpl::actionManager() const
{
    return m_mainwindow->actionManager();
}

UniqueIDManager *CoreImpl::uniqueIDManager() const
{
    return m_mainwindow->uniqueIDManager();
}

MessageManager *CoreImpl::messageManager() const
{
    return m_mainwindow->messageManager();
}

UAVGadgetManager *CoreImpl::uavGadgetManager() const
{
    return m_mainwindow->uavGadgetManager();
}


VariableManager *CoreImpl::variableManager() const
{
    return m_mainwindow->variableManager();
}

ModeManager *CoreImpl::modeManager() const
{
    return m_mainwindow->modeManager();
}

MimeDatabase *CoreImpl::mimeDatabase() const
{
    return m_mainwindow->mimeDatabase();
}

QSettings *CoreImpl::settings() const
{
    return m_mainwindow->settings();
}

SettingsDatabase *CoreImpl::settingsDatabase() const
{
    return m_mainwindow->settingsDatabase();
}

#ifdef Q_OS_MAC
#  define SHARE_PATH "/../Resources"
#else
#  define SHARE_PATH "/../share/qtcreator"
#endif

QString CoreImpl::resourcePath() const
{
    return QDir::cleanPath(QCoreApplication::applicationDirPath() + QLatin1String(SHARE_PATH));
}

IContext *CoreImpl::currentContextObject() const
{
    return m_mainwindow->currentContextObject();
}


QMainWindow *CoreImpl::mainWindow() const
{
    return m_mainwindow;
}

// adds and removes additional active contexts, this context is appended to the
// currently active contexts. call updateContext after changing
void CoreImpl::addAdditionalContext(int context)
{
    m_mainwindow->addAdditionalContext(context);
}

void CoreImpl::removeAdditionalContext(int context)
{
    m_mainwindow->removeAdditionalContext(context);
}

bool CoreImpl::hasContext(int context) const
{
    return m_mainwindow->hasContext(context);
}

void CoreImpl::addContextObject(IContext *context)
{
    m_mainwindow->addContextObject(context);
}

void CoreImpl::removeContextObject(IContext *context)
{
    m_mainwindow->removeContextObject(context);
}

void CoreImpl::updateContext()
{
    return m_mainwindow->updateContext();
}

void CoreImpl::openFiles(const QStringList &arguments)
{
    m_mainwindow->openFiles(arguments);
}

