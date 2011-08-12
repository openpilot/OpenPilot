/**
 ******************************************************************************
 *
 * @file       coreimpl.h
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

#ifndef COREIMPL_H
#define COREIMPL_H

#include "icore.h"
#include "mainwindow.h"

namespace Core {
namespace Internal {

class CoreImpl : public ICore
{
    Q_OBJECT

public:
    CoreImpl(MainWindow *mainwindow);
    ~CoreImpl() {}

    bool showOptionsDialog(const QString &group = QString(),
                           const QString &page = QString(),
                           QWidget *parent = 0);
    bool showWarningWithOptions(const QString &title, const QString &text,
                                const QString &details = QString(),
                                const QString &settingsCategory = QString(),
                                const QString &settingsId = QString(),
                                QWidget *parent = 0);

    ActionManager *actionManager() const;
    UniqueIDManager *uniqueIDManager() const;
    MessageManager *messageManager() const;
    ConnectionManager *connectionManager() const;
    UAVGadgetInstanceManager *uavGadgetInstanceManager() const;
    VariableManager *variableManager() const;
    ThreadManager *threadManager() const;
    ModeManager *modeManager() const;
    MimeDatabase *mimeDatabase() const;

    QSettings *settings(QSettings::Scope scope = QSettings::UserScope) const;
    SettingsDatabase *settingsDatabase() const;
    void readMainSettings(QSettings* qs, bool workspaceDiffOnly);
    void saveMainSettings(QSettings* qs);
    void readSettings(IConfigurablePlugin* plugin, QSettings* qs = 0 );
    void saveSettings(IConfigurablePlugin* plugin, QSettings* qs = 0 );
    void deleteSettings();

    QString resourcePath() const;

    IContext *currentContextObject() const;

    QMainWindow *mainWindow() const;

    // adds and removes additional active contexts, this context is appended to the
    // currently active contexts. call updateContext after changing
    void addAdditionalContext(int context);
    void removeAdditionalContext(int context);
    bool hasContext(int context) const;
    void addContextObject(IContext *contex);
    void removeContextObject(IContext *contex);

    void updateContext();

    void openFiles(const QStringList &fileNames);

private:
    MainWindow *m_mainwindow;
    friend class MainWindow;
};

} // namespace Internal
} // namespace Core

#endif // COREIMPL_H
