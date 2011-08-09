/**
 ******************************************************************************
 *
 * @file       icore.h
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

#ifndef ICORE_H
#define ICORE_H

#include "core_global.h"
#include <QtCore/QObject>
#include <QSettings>

QT_BEGIN_NAMESPACE
class QMainWindow;
class QSettings;
template <class T> class QList;
QT_END_NAMESPACE

namespace Core {

class ActionManager;
class IContext;
class IWizard;
class ConnectionManager;
class MessageManager;
class MimeDatabase;
class ModeManager;
class SettingsDatabase;
class UniqueIDManager;
class VariableManager;
class ThreadManager;
class UAVGadgetManager;
class UAVGadgetInstanceManager;
class IConfigurablePlugin;

class CORE_EXPORT ICore : public QObject
{
    Q_OBJECT

public:
    ICore() {}
    virtual ~ICore() {}

    static ICore *instance();

    virtual bool showOptionsDialog(const QString &group = QString(),
                                   const QString &page = QString(),
                                   QWidget *parent = 0) = 0;

    virtual bool showWarningWithOptions(const QString &title, const QString &text,
                                       const QString &details = QString(),
                                       const QString &settingsCategory = QString(),
                                       const QString &settingsId = QString(),
                                       QWidget *parent = 0) = 0;

    virtual ActionManager *actionManager() const = 0;
    virtual UniqueIDManager *uniqueIDManager() const = 0;
    virtual MessageManager *messageManager() const = 0;
    virtual VariableManager *variableManager() const = 0;
    virtual ThreadManager *threadManager() const = 0;
    virtual ModeManager *modeManager() const = 0;
    virtual ConnectionManager *connectionManager() const = 0;
    virtual UAVGadgetInstanceManager *uavGadgetInstanceManager() const = 0;
    virtual MimeDatabase *mimeDatabase() const = 0;

    virtual QSettings *settings(QSettings::Scope scope = QSettings::UserScope) const = 0;
    virtual SettingsDatabase *settingsDatabase() const = 0;
    virtual void readMainSettings(QSettings* qs, bool workspaceDiffOnly = false) = 0;
    virtual void saveMainSettings(QSettings* qs) = 0;
    virtual void readSettings(IConfigurablePlugin* plugin, QSettings* qs = 0) = 0;
    virtual void saveSettings(IConfigurablePlugin* plugin, QSettings* qs = 0) = 0;
    virtual void deleteSettings() = 0;

    virtual QString resourcePath() const = 0;

    virtual QMainWindow *mainWindow() const = 0;

    // adds and removes additional active contexts, this context is appended to the
    // currently active contexts. call updateContext after changing
    virtual IContext *currentContextObject() const = 0;
    virtual void addAdditionalContext(int context) = 0;
    virtual void removeAdditionalContext(int context) = 0;
    virtual bool hasContext(int context) const = 0;
    virtual void addContextObject(IContext *context) = 0;
    virtual void removeContextObject(IContext *context) = 0;

    virtual void updateContext() = 0;

    virtual void openFiles(const QStringList &fileNames) = 0;

signals:
    void coreAboutToOpen();
    void coreOpened();
    void saveSettingsRequested();
    void optionsDialogRequested();
    void coreAboutToClose();
    void contextAboutToChange(Core::IContext *context);
    void contextChanged(Core::IContext *context);
};

} // namespace Core

#endif // ICORE_H
