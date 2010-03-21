/**
 ******************************************************************************
 *
 * @file       mainwindow.h
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core_global.h"

#include "eventfilteringmainwindow.h"

#include <QtCore/QMap>

QT_BEGIN_NAMESPACE
class QSettings;
class QShortcut;
class QToolButton;
QT_END_NAMESPACE

namespace Core {

class ActionManager;
class BaseMode;
class BaseView;
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
class ViewManagerInterface;
class UAVGadgetManager;

namespace Internal {

class ActionManagerPrivate;
class CoreImpl;
class FancyTabWidget;
class GeneralSettings;
class ShortcutSettings;
class ViewManager;
class VersionDialog;

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

    void openFiles(const QStringList &fileNames);

    Core::ActionManager *actionManager() const;
    Core::UniqueIDManager *uniqueIDManager() const;
    Core::MessageManager *messageManager() const;
    Core::UAVGadgetManager *uavGadgetManager() const;
    Core::ConnectionManager *connectionManager() const;
    Core::VariableManager *variableManager() const;
    Core::ModeManager *modeManager() const;
    Core::MimeDatabase *mimeDatabase() const;

    inline QSettings *settings() const { return m_settings; }
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
    void aboutOpenPilogGCS();
    void aboutPlugins();
    void updateFocusWidget(QWidget *old, QWidget *now);
    void destroyVersionDialog();
    void modeChanged(Core::IMode *mode);

private:
    void updateContextObject(IContext *context);
    void registerDefaultContainers();
    void registerDefaultActions();

    void readSettings();
    void writeSettings();

    CoreImpl *m_coreImpl;
    UniqueIDManager *m_uniqueIDManager;
    QList<int> m_globalContext;
    QList<int> m_additionalContexts;
    QSettings *m_settings;
    SettingsDatabase *m_settingsDatabase;
    ActionManagerPrivate *m_actionManager;
    MessageManager *m_messageManager;
    VariableManager *m_variableManager;
    ViewManager *m_viewManager;
    ModeManager *m_modeManager;
    UAVGadgetManager *m_uavGadgetManager;
    ConnectionManager *m_connectionManager;
    MimeDatabase *m_mimeDatabase;
    FancyTabWidget *m_modeStack;
//    RightPaneWidget *m_rightPaneWidget;
    Core::BaseView *m_outputView;
    VersionDialog *m_versionDialog;

    IContext * m_activeContext;

    QMap<QWidget *, IContext *> m_contextWidgets;

    GeneralSettings *m_generalSettings;
    ShortcutSettings *m_shortcutSettings;

    // actions
    QShortcut *m_focusToEditor;
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_openWithAction;
    QAction *m_saveAllAction;
    QAction *m_exitAction;
    QAction *m_optionsAction;
    QAction *m_toggleFullScreenAction;
#ifdef Q_WS_MAC
    QAction *m_minimizeAction;
    QAction *m_zoomAction;
#endif

};

} // namespace Internal
} // namespace Core

#endif // MAINWINDOW_H
