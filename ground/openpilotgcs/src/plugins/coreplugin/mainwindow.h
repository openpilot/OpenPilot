/**
 ******************************************************************************
 *
 * @file       mainwindow.h
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core_global.h"

#include "eventfilteringmainwindow.h"

#include <QtCore/QMap>
#include <QSettings>

QT_BEGIN_NAMESPACE
class QSettings;
class QShortcut;
class QToolButton;
class MyTabWidget;
QT_END_NAMESPACE

namespace Core {

class ActionManager;
class BaseMode;
class BaseView;
class IConfigurablePlugin;
class IContext;
class IMode;
class IWizard;
class ConnectionManager;
class MessageManager;
class MimeDatabase;
class ModeManager;
class RightPaneWidget;
class SettingsDatabase;
class UniqueIDManager;
class VariableManager;
class ThreadManager;
class ViewManagerInterface;
class UAVGadgetManager;
class UAVGadgetInstanceManager;


namespace Internal {

class ActionManagerPrivate;
class CoreImpl;
class FancyTabWidget;
class GeneralSettings;
class ShortcutSettings;
class WorkspaceSettings;
class VersionDialog;
class AuthorsDialog;

class CORE_EXPORT MainWindow : public EventFilteringMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    bool init(QString *errorMessage);
    void extensionsInitialized();
    void shutdown();

    IContext *contextObject(QWidget *widget);
    void addContextObject(IContext *contex);
    void removeContextObject(IContext *contex);
    void resetContext();
    void readSettings(QSettings* qs = 0, bool workspaceDiffOnly = false);
    void saveSettings(QSettings* qs = 0);
    void readSettings(IConfigurablePlugin* plugin, QSettings* qs = 0);
    void saveSettings(IConfigurablePlugin* plugin, QSettings* qs = 0);
    void deleteSettings();
    void openFiles(const QStringList &fileNames);

    Core::ActionManager *actionManager() const;
    Core::UniqueIDManager *uniqueIDManager() const;
    Core::MessageManager *messageManager() const;
    QList<UAVGadgetManager*> uavGadgetManagers() const;
    UAVGadgetInstanceManager *uavGadgetInstanceManager() const;
    Core::ConnectionManager *connectionManager() const;
    Core::VariableManager *variableManager() const;
    Core::ThreadManager *threadManager() const;
    Core::ModeManager *modeManager() const;
    Core::MimeDatabase *mimeDatabase() const;
    Internal::GeneralSettings *generalSettings() const;
    QSettings *settings(QSettings::Scope scope) const;
    inline SettingsDatabase *settingsDatabase() const { return m_settingsDatabase; }
    IContext * currentContextObject() const;
    QStatusBar *statusBar() const;
    void addAdditionalContext(int context);
    void removeAdditionalContext(int context);
    bool hasContext(int context) const;

    void updateContext();

    void setSuppressNavigationWidget(bool suppress);

signals:
    void windowActivated();

public slots:
    void newFile();
    void openFileWith();
    void exit();
    void setFullScreen(bool on);

    bool showOptionsDialog(const QString &category = QString(),
                           const QString &page = QString(),
                           QWidget *parent = 0);

    bool showWarningWithOptions(const QString &title, const QString &text,
                                const QString &details = QString(),
                                const QString &settingsCategory = QString(),
                                const QString &settingsId = QString(),
                                QWidget *parent = 0);

protected:
    virtual void changeEvent(QEvent *e);
    virtual void closeEvent(QCloseEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private slots:
    void openFile();
    void aboutToShowRecentFiles();
    void openRecentFile();
    void setFocusToEditor();
    void saveAll();
    void aboutOpenPilotGCS();
    void aboutPlugins();
    void aboutOpenPilotAuthors();
    void updateFocusWidget(QWidget *old, QWidget *now);
    void destroyVersionDialog();
    void destroyAuthorsDialog();
    void modeChanged(Core::IMode *mode);
    void showUavGadgetMenus(bool show, bool hasSplitter);
    void applyTabBarSettings(QTabWidget::TabPosition pos, bool movable);
    void showHelp();

private:
    void updateContextObject(IContext *context);
    void registerDefaultContainers();
    void registerDefaultActions();
    void createWorkspaces(QSettings* qs, bool diffOnly = false);
    void loadStyleSheet(QString name);

    CoreImpl *m_coreImpl;
    UniqueIDManager *m_uniqueIDManager;
    QList<int> m_globalContext;
    QList<int> m_additionalContexts;
    QSettings *m_settings;
    QSettings *m_globalSettings;
    SettingsDatabase *m_settingsDatabase;
    bool m_dontSaveSettings; // In case of an Error or if we reset the settings, never save them.
    ActionManagerPrivate *m_actionManager;
    MessageManager *m_messageManager;
    VariableManager *m_variableManager;
    ThreadManager *m_threadManager;
    ModeManager *m_modeManager;
    QList<UAVGadgetManager*> m_uavGadgetManagers;
    UAVGadgetInstanceManager *m_uavGadgetInstanceManager;
    ConnectionManager *m_connectionManager;
    MimeDatabase *m_mimeDatabase;
    MyTabWidget *m_modeStack;
    Core::BaseView *m_outputView;
    VersionDialog *m_versionDialog;
    AuthorsDialog *m_authorsDialog;

    IContext * m_activeContext;

    QMap<QWidget *, IContext *> m_contextWidgets;

    GeneralSettings *m_generalSettings;
    ShortcutSettings *m_shortcutSettings;
    WorkspaceSettings *m_workspaceSettings;

    // actions
    QShortcut *m_focusToEditor;
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_openWithAction;
    QAction *m_saveAllAction;
    QAction *m_exitAction;
    QAction *m_optionsAction;
    QAction *m_toggleFullScreenAction;
    // UavGadgetManager actions
    QAction *m_showToolbarsAction;
    QAction *m_splitAction;
    QAction *m_splitSideBySideAction;
    QAction *m_removeCurrentSplitAction;
    QAction *m_removeAllSplitsAction;
    QAction *m_gotoOtherSplitAction;

    QString m_config_description;
    QString m_config_details;
    QString m_config_stylesheet;
#ifdef Q_WS_MAC
    QAction *m_minimizeAction;
    QAction *m_zoomAction;
#endif

};

} // namespace Internal
} // namespace Core

#endif // MAINWINDOW_H
