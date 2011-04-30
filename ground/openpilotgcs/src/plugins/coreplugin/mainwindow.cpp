/**
 ******************************************************************************
 *
 * @file       mainwindow.cpp
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

#include "mainwindow.h"
#include "actioncontainer.h"
#include "actionmanager_p.h"
#include "basemode.h"
#include "coreimpl.h"
#include "coreconstants.h"
#include "fancytabwidget.h"
#include "generalsettings.h"
#include "messagemanager.h"
#include "modemanager.h"
#include "mimedatabase.h"
#include "outputpane.h"
#include "plugindialog.h"
#include "shortcutsettings.h"
#include "workspacesettings.h"
#include "modemanager.h"
#include "uavgadgetmode.h"
#include "uavgadgetmanager.h"
#include "uavgadgetinstancemanager.h"
#include "connectionmanager.h"
#include "qxtlogger.h"
#include "qxtbasicstdloggerengine.h"

#include "settingsdialog.h"
#include "variablemanager.h"
#include "threadmanager.h"
#include "versiondialog.h"
#include "authorsdialog.h"
#include "viewmanager.h"
#include "uniqueidmanager.h"
#include "manhattanstyle.h"
#include "dialogs/iwizard.h"
#include "rightpane.h"
#include "baseview.h"
#include "ioutputpane.h"
#include "icorelistener.h"
#include "iconfigurableplugin.h"

#include <coreplugin/settingsdatabase.h>
#include <utils/pathchooser.h>
#include <utils/stylehelper.h>
#include <extensionsystem/pluginmanager.h>

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QtPlugin>
#include <QtCore/QUrl>

#include <QtGui/QApplication>
#include <QtGui/QCloseEvent>
#include <QtGui/QMenu>
#include <QtGui/QPixmap>
#include <QtGui/QShortcut>
#include <QtGui/QStatusBar>
#include <QtGui/QWizard>
#include <QtGui/QToolButton>
#include <QtGui/QMessageBox>

/*
#ifdef Q_OS_UNIX
#include <signal.h>
extern "C" void handleSigInt(int sig)
{
    Q_UNUSED(sig)
    Core::ICore::instance()->exit();
    qDebug() << "SIGINT caught. Shutting down.";
}
#endif
*/

using namespace Core;
using namespace Core::Internal;

static const char *uriListMimeFormatC = "text/uri-list";

enum { debugMainWindow = 0 };

MainWindow::MainWindow() :
    EventFilteringMainWindow(),
    m_coreImpl(new CoreImpl(this)),
    m_uniqueIDManager(new UniqueIDManager()),
    m_globalContext(QList<int>() << Constants::C_GLOBAL_ID),
    m_additionalContexts(m_globalContext),
    // keep this in sync with main() in app/main.cpp
    m_settings(new QSettings(QSettings::IniFormat, QSettings::UserScope,
                             QLatin1String("OpenPilot"), QLatin1String("OpenPilotGCS"), this)),
    m_globalSettings(new QSettings(QSettings::IniFormat, QSettings::SystemScope,
                                 QLatin1String("OpenPilot"), QLatin1String("OpenPilotGCS"), this)),
    m_settingsDatabase(new SettingsDatabase(QFileInfo(m_settings->fileName()).path(),
                                            QLatin1String("OpenPilotGCS"),
                                            this)),
    m_dontSaveSettings(false),
    m_actionManager(new ActionManagerPrivate(this)),
    m_variableManager(new VariableManager(this)),
    m_threadManager(new ThreadManager(this)),
    m_viewManager(0),
    m_modeManager(0),
    m_connectionManager(0),
    m_mimeDatabase(new MimeDatabase),
//    m_rightPaneWidget(0),
    m_versionDialog(0),
    m_authorsDialog(0),
    m_activeContext(0),
    m_generalSettings(new GeneralSettings),
    m_shortcutSettings(new ShortcutSettings),
    m_workspaceSettings(new WorkspaceSettings),
    m_focusToEditor(0),
    m_newAction(0),
    m_openAction(0),
    m_openWithAction(0),
    m_saveAllAction(0),
    m_exitAction(0),
    m_optionsAction(0),
#ifdef Q_WS_MAC
    m_minimizeAction(0),
    m_zoomAction(0),
#endif
    m_toggleFullScreenAction(0)
{
    setWindowTitle(tr("OpenPilot GCS"));
#ifndef Q_WS_MAC
    qApp->setWindowIcon(QIcon(":/core/images/openpilot_logo_128.png"));
#endif
    QCoreApplication::setApplicationName(QLatin1String("OpenPilotGCS"));
    QCoreApplication::setApplicationVersion(QLatin1String(Core::Constants::GCS_VERSION_LONG));
    QCoreApplication::setOrganizationName(QLatin1String("OpenPilot"));
    QCoreApplication::setOrganizationDomain(QLatin1String("openpilot.org"));
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QString baseName = qApp->style()->objectName();
#ifdef Q_WS_X11
    if (baseName == QLatin1String("windows")) {
        // Sometimes we get the standard windows 95 style as a fallback
        // e.g. if we are running on a KDE4 desktop
        QByteArray desktopEnvironment = qgetenv("DESKTOP_SESSION");
        if (desktopEnvironment == "kde")
            baseName = QLatin1String("plastique");
        else
            baseName = QLatin1String("cleanlooks");
    }
#endif
    qApp->setStyle(new ManhattanStyle(baseName));

    setDockNestingEnabled(true);

    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);

    registerDefaultContainers();
    registerDefaultActions();

    m_modeStack = new FancyTabWidget(this);
    m_modeManager = new ModeManager(this, m_modeStack);

    m_connectionManager = new ConnectionManager(this, m_modeStack);

    m_viewManager = new ViewManager(this);
    m_messageManager = new MessageManager;
    setCentralWidget(m_modeStack);

    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(updateFocusWidget(QWidget*,QWidget*)));

//    setUnifiedTitleAndToolBarOnMac(true);
#ifdef Q_OS_UNIX
     //signal(SIGINT, handleSigInt);
#endif

    statusBar()->setProperty("p_styled", true);
    setAcceptDrops(true);
    foreach (QString engine, qxtLog->allLoggerEngines())
        qxtLog->removeLoggerEngine(engine);
    qxtLog->addLoggerEngine("std", new QxtBasicSTDLoggerEngine());
    qxtLog->installAsMessageHandler();
    qxtLog->enableAllLogLevels();
}

MainWindow::~MainWindow()
{
	if (m_connectionManager)	// Pip
	{
		m_connectionManager->disconnectDevice();
		m_connectionManager->suspendPolling();
	}

	hide();

	qxtLog->removeAsMessageHandler();
    foreach (QString engine, qxtLog->allLoggerEngines())
        qxtLog->removeLoggerEngine(engine);
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    if (m_uavGadgetModes.count() > 0) {
        foreach (UAVGadgetMode *mode, m_uavGadgetModes)
        {
            pm->removeObject(mode);
            delete mode;
        }
    }

    pm->removeObject(m_shortcutSettings);
    pm->removeObject(m_generalSettings);
    pm->removeObject(m_workspaceSettings);
    delete m_messageManager;
    m_messageManager = 0;
    delete m_shortcutSettings;
    m_shortcutSettings = 0;
    delete m_generalSettings;
    m_generalSettings = 0;
    delete m_workspaceSettings;
    m_workspaceSettings = 0;
    delete m_settings;
    m_settings = 0;
    delete m_uniqueIDManager;
    m_uniqueIDManager = 0;

    delete m_viewManager;
    m_viewManager = 0;

    pm->removeObject(m_coreImpl);
    delete m_coreImpl;
    m_coreImpl = 0;

//    delete m_rightPaneWidget;
//    m_rightPaneWidget = 0;

    delete m_modeManager;
    m_modeManager = 0;
    delete m_mimeDatabase;
    m_mimeDatabase = 0;
}

bool MainWindow::init(QString *errorMessage)
{
    Q_UNUSED(errorMessage)

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    pm->addObject(m_coreImpl);
    m_viewManager->init();
    m_modeManager->init();
    m_connectionManager->init();

    pm->addObject(m_generalSettings);
    pm->addObject(m_shortcutSettings);
    pm->addObject(m_workspaceSettings);

    return true;
}

void MainWindow::modeChanged(Core::IMode */*mode*/)
{

}

void MainWindow::extensionsInitialized()
{

    QSettings* qs = m_settings;
    QSettings defaultSettings(":/core/OpenPilotGCS.ini", QSettings::IniFormat);

    if ( ! qs->allKeys().count() ){
        QMessageBox msgBox;
        msgBox.setText(tr("No configuration file could be found."));
        msgBox.setInformativeText(tr("Do you want to load the default configuration?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        if ( msgBox.exec() == QMessageBox::Yes ){
            qDebug() << "Load default config from resource /core/OpenPilotGCS.ini";
            qs = &defaultSettings;
        }
    }

    m_uavGadgetInstanceManager = new UAVGadgetInstanceManager(this);
    m_uavGadgetInstanceManager->readSettings(qs);

    m_messageManager->init();
    readSettings(qs);

    updateContext();

    emit m_coreImpl->coreAboutToOpen();
    show();
    emit m_coreImpl->coreOpened();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if ( !m_generalSettings->saveSettingsOnExit() ){
        m_dontSaveSettings = true;
    }
    if ( !m_dontSaveSettings ){
        emit m_coreImpl->saveSettingsRequested();
    }

    const QList<ICoreListener *> listeners =
        ExtensionSystem::PluginManager::instance()->getObjects<ICoreListener>();
    foreach (ICoreListener *listener, listeners) {
        if (!listener->coreAboutToClose()) {
            event->ignore();
            return;
        }
    }

    emit m_coreImpl->coreAboutToClose();

    if ( !m_dontSaveSettings ){
        saveSettings(m_settings);
        m_uavGadgetInstanceManager->saveSettings(m_settings);
    }
    event->accept();
}

// Check for desktop file manager file drop events

static bool isDesktopFileManagerDrop(const QMimeData *d, QStringList *files = 0)
{
    if (files)
        files->clear();
    // Extract dropped files from Mime data.
    if (!d->hasFormat(QLatin1String(uriListMimeFormatC)))
        return false;
    const QList<QUrl> urls = d->urls();
    if (urls.empty())
        return false;
    // Try to find local files
    bool hasFiles = false;
    const QList<QUrl>::const_iterator cend = urls.constEnd();
    for (QList<QUrl>::const_iterator it = urls.constBegin(); it != cend; ++it) {
        const QString fileName = it->toLocalFile();
        if (!fileName.isEmpty()) {
            hasFiles = true;
            if (files) {
                files->push_back(fileName);
            } else {
                break; // No result list, sufficient for checking
            }
        }
    }
    return hasFiles;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (isDesktopFileManagerDrop(event->mimeData())) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QStringList files;
    if (isDesktopFileManagerDrop(event->mimeData(), &files)) {
        event->accept();
        //openFiles(files);
    } else {
        event->ignore();
    }
}

IContext *MainWindow::currentContextObject() const
{
    return m_activeContext;
}

QStatusBar *MainWindow::statusBar() const
{
    return m_modeStack->statusBar();
}

void MainWindow::registerDefaultContainers()
{
    ActionManagerPrivate *am = m_actionManager;

    ActionContainer *menubar = am->createMenuBar(Constants::MENU_BAR);

#ifndef Q_WS_MAC // System menu bar on Mac
    setMenuBar(menubar->menuBar());
#endif
    menubar->appendGroup(Constants::G_FILE);
    menubar->appendGroup(Constants::G_EDIT);
    menubar->appendGroup(Constants::G_VIEW);
    menubar->appendGroup(Constants::G_TOOLS);
    menubar->appendGroup(Constants::G_WINDOW);
    menubar->appendGroup(Constants::G_HELP);

    // File Menu
    ActionContainer *filemenu = am->createMenu(Constants::M_FILE);
    menubar->addMenu(filemenu, Constants::G_FILE);
    filemenu->menu()->setTitle(tr("&File"));
    filemenu->appendGroup(Constants::G_FILE_NEW);
    filemenu->appendGroup(Constants::G_FILE_OPEN);
    filemenu->appendGroup(Constants::G_FILE_PROJECT);
    filemenu->appendGroup(Constants::G_FILE_SAVE);
    filemenu->appendGroup(Constants::G_FILE_CLOSE);
    filemenu->appendGroup(Constants::G_FILE_OTHER);
    connect(filemenu->menu(), SIGNAL(aboutToShow()), this, SLOT(aboutToShowRecentFiles()));


    // Edit Menu
    ActionContainer *medit = am->createMenu(Constants::M_EDIT);
    menubar->addMenu(medit, Constants::G_EDIT);
    medit->menu()->setTitle(tr("&Edit"));
    medit->appendGroup(Constants::G_EDIT_UNDOREDO);
    medit->appendGroup(Constants::G_EDIT_COPYPASTE);
    medit->appendGroup(Constants::G_EDIT_SELECTALL);
    medit->appendGroup(Constants::G_EDIT_ADVANCED);
    medit->appendGroup(Constants::G_EDIT_FIND);
    medit->appendGroup(Constants::G_EDIT_OTHER);

    // Tools Menu
    ActionContainer *ac = am->createMenu(Constants::M_TOOLS);
    menubar->addMenu(ac, Constants::G_TOOLS);
    ac->menu()->setTitle(tr("&Tools"));

    // Window Menu
    ActionContainer *mwindow = am->createMenu(Constants::M_WINDOW);
    menubar->addMenu(mwindow, Constants::G_WINDOW);
    mwindow->menu()->setTitle(tr("&Window"));
    mwindow->appendGroup(Constants::G_WINDOW_SIZE);
    mwindow->appendGroup(Constants::G_WINDOW_HIDE_TOOLBAR);
    mwindow->appendGroup(Constants::G_WINDOW_PANES);
    mwindow->appendGroup(Constants::G_WINDOW_SPLIT);
    mwindow->appendGroup(Constants::G_WINDOW_NAVIGATE);
    mwindow->appendGroup(Constants::G_WINDOW_OTHER);

    // Help Menu
    ac = am->createMenu(Constants::M_HELP);
    menubar->addMenu(ac, Constants::G_HELP);
    ac->menu()->setTitle(tr("&Help"));
    ac->appendGroup(Constants::G_HELP_HELP);
    ac->appendGroup(Constants::G_HELP_ABOUT);
}

static Command *createSeparator(ActionManager *am, QObject *parent,
                                const QString &name,
                                const QList<int> &context)
{
    QAction *tmpaction = new QAction(parent);
    tmpaction->setSeparator(true);
    Command *cmd = am->registerAction(tmpaction, name, context);
    return cmd;
}

void MainWindow::registerDefaultActions()
{
    ActionManagerPrivate *am = m_actionManager;
    ActionContainer *mfile = am->actionContainer(Constants::M_FILE);
    ActionContainer *medit = am->actionContainer(Constants::M_EDIT);
    ActionContainer *mtools = am->actionContainer(Constants::M_TOOLS);
    ActionContainer *mwindow = am->actionContainer(Constants::M_WINDOW);
    ActionContainer *mhelp = am->actionContainer(Constants::M_HELP);

    // File menu separators
    Command *cmd = createSeparator(am, this, QLatin1String("QtCreator.File.Sep.Save"), m_globalContext);
    mfile->addAction(cmd, Constants::G_FILE_SAVE);

    cmd =  createSeparator(am, this, QLatin1String("QtCreator.File.Sep.Close"), m_globalContext);
    mfile->addAction(cmd, Constants::G_FILE_CLOSE);

    cmd = createSeparator(am, this, QLatin1String("QtCreator.File.Sep.Other"), m_globalContext);
    mfile->addAction(cmd, Constants::G_FILE_OTHER);

    // Edit menu separators
    cmd = createSeparator(am, this, QLatin1String("QtCreator.Edit.Sep.CopyPaste"), m_globalContext);
    medit->addAction(cmd, Constants::G_EDIT_COPYPASTE);

    cmd = createSeparator(am, this, QLatin1String("QtCreator.Edit.Sep.SelectAll"), m_globalContext);
    medit->addAction(cmd, Constants::G_EDIT_SELECTALL);

    cmd = createSeparator(am, this, QLatin1String("QtCreator.Edit.Sep.Find"), m_globalContext);
    medit->addAction(cmd, Constants::G_EDIT_FIND);

    cmd = createSeparator(am, this, QLatin1String("QtCreator.Edit.Sep.Advanced"), m_globalContext);
    medit->addAction(cmd, Constants::G_EDIT_ADVANCED);

    // Tools menu separators
    cmd = createSeparator(am, this, QLatin1String("QtCreator.Tools.Sep.Options"), m_globalContext);
    mtools->addAction(cmd, Constants::G_DEFAULT_THREE);

    // Help menu separators

    // Return to editor shortcut: Note this requires Qt to fix up
    // handling of shortcut overrides in menus, item views, combos....
    m_focusToEditor = new QShortcut(this);
    cmd = am->registerShortcut(m_focusToEditor, Constants::S_RETURNTOEDITOR, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence(Qt::Key_Escape));
    connect(m_focusToEditor, SIGNAL(activated()), this, SLOT(setFocusToEditor()));

    // New File Action

    /*
    m_newAction = new QAction(QIcon(Constants::ICON_NEWFILE), tr("&New File or Project..."), this);
    cmd = am->registerAction(m_newAction, Constants::NEW, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence::New);
    mfile->addAction(cmd, Constants::G_FILE_NEW);
    connect(m_newAction, SIGNAL(triggered()), this, SLOT(newFile()));
*/

    // Open Action
/*
    m_openAction = new QAction(QIcon(Constants::ICON_OPENFILE), tr("&Open File or Project..."), this);
    cmd = am->registerAction(m_openAction, Constants::OPEN, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence::Open);
    mfile->addAction(cmd, Constants::G_FILE_OPEN);
    connect(m_openAction, SIGNAL(triggered()), this, SLOT(openFile()));
*/

    // Open With Action
/*
    m_openWithAction = new QAction(tr("&Open File With..."), this);
    cmd = am->registerAction(m_openWithAction, Constants::OPEN_WITH, m_globalContext);
    mfile->addAction(cmd, Constants::G_FILE_OPEN);
    connect(m_openWithAction, SIGNAL(triggered()), this, SLOT(openFileWith()));
*/

        // File->Recent Files Menu
/*
    ActionContainer *ac = am->createMenu(Constants::M_FILE_RECENTFILES);
    mfile->addMenu(ac, Constants::G_FILE_OPEN);
    ac->menu()->setTitle(tr("Recent Files"));
*/
/*
    // Save Action
    QAction *tmpaction = new QAction(QIcon(Constants::ICON_SAVEFILE), tr("&Save"), this);
    cmd = am->registerAction(tmpaction, Constants::SAVE, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence::Save);
    cmd->setAttribute(Command::CA_UpdateText);
    cmd->setDefaultText(tr("&Save"));
    mfile->addAction(cmd, Constants::G_FILE_SAVE);

    // Save As Action
    tmpaction = new QAction(tr("Save &As..."), this);
    cmd = am->registerAction(tmpaction, Constants::SAVEAS, m_globalContext);
#ifdef Q_WS_MAC
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+S")));
#endif
    cmd->setAttribute(Command::CA_UpdateText);
    cmd->setDefaultText(tr("Save &As..."));
    mfile->addAction(cmd, Constants::G_FILE_SAVE);
    */

    // SaveAll Action
    m_saveAllAction = new QAction(tr("Save &GCS Default Settings"), this);
    cmd = am->registerAction(m_saveAllAction, Constants::SAVEALL, m_globalContext);
#ifndef Q_WS_MAC
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+S")));
#endif
    mfile->addAction(cmd, Constants::G_FILE_SAVE);
    connect(m_saveAllAction, SIGNAL(triggered()), this, SLOT(saveAll()));

    // Exit Action
    m_exitAction = new QAction(QIcon(Constants::ICON_EXIT), tr("E&xit"), this);
    cmd = am->registerAction(m_exitAction, Constants::EXIT, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Q")));
    mfile->addAction(cmd, Constants::G_FILE_OTHER);
    connect(m_exitAction, SIGNAL(triggered()), this, SLOT(exit()));

    // Undo Action
    QAction *tmpaction = new QAction(QIcon(Constants::ICON_UNDO), tr("&Undo"), this);
    cmd = am->registerAction(tmpaction, Constants::UNDO, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence::Undo);
    cmd->setAttribute(Command::CA_UpdateText);
    cmd->setDefaultText(tr("&Undo"));
    medit->addAction(cmd, Constants::G_EDIT_UNDOREDO);
    tmpaction->setEnabled(false);

    // Redo Action
    tmpaction = new QAction(QIcon(Constants::ICON_REDO), tr("&Redo"), this);
    cmd = am->registerAction(tmpaction, Constants::REDO, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence::Redo);
    cmd->setAttribute(Command::CA_UpdateText);
    cmd->setDefaultText(tr("&Redo"));
    medit->addAction(cmd, Constants::G_EDIT_UNDOREDO);
    tmpaction->setEnabled(false);

    // Cut Action
    tmpaction = new QAction(QIcon(Constants::ICON_CUT), tr("Cu&t"), this);
    cmd = am->registerAction(tmpaction, Constants::CUT, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence::Cut);
    medit->addAction(cmd, Constants::G_EDIT_COPYPASTE);
    tmpaction->setEnabled(false);

    // Copy Action
    tmpaction = new QAction(QIcon(Constants::ICON_COPY), tr("&Copy"), this);
    cmd = am->registerAction(tmpaction, Constants::COPY, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence::Copy);
    medit->addAction(cmd, Constants::G_EDIT_COPYPASTE);
    tmpaction->setEnabled(false);

    // Paste Action
    tmpaction = new QAction(QIcon(Constants::ICON_PASTE), tr("&Paste"), this);
    cmd = am->registerAction(tmpaction, Constants::PASTE, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence::Paste);
    medit->addAction(cmd, Constants::G_EDIT_COPYPASTE);
    tmpaction->setEnabled(false);

    // Select All
    tmpaction = new QAction(tr("&Select All"), this);
    cmd = am->registerAction(tmpaction, Constants::SELECTALL, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence::SelectAll);
    medit->addAction(cmd, Constants::G_EDIT_SELECTALL);
    tmpaction->setEnabled(false);

    // Options Action
    m_optionsAction = new QAction(QIcon(Constants::ICON_OPTIONS), tr("&Options..."), this);
    cmd = am->registerAction(m_optionsAction, Constants::OPTIONS, m_globalContext);
#ifdef Q_WS_MAC
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+,"));
    cmd->action()->setMenuRole(QAction::PreferencesRole);
#endif
    mtools->addAction(cmd, Constants::G_DEFAULT_THREE);
    connect(m_optionsAction, SIGNAL(triggered()), this, SLOT(showOptionsDialog()));

#ifdef Q_WS_MAC
    // Minimize Action
    m_minimizeAction = new QAction(tr("Minimize"), this);
    cmd = am->registerAction(m_minimizeAction, Constants::MINIMIZE_WINDOW, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+M"));
    mwindow->addAction(cmd, Constants::G_WINDOW_SIZE);
    connect(m_minimizeAction, SIGNAL(triggered()), this, SLOT(showMinimized()));

    // Zoom Action
    m_zoomAction = new QAction(tr("Zoom"), this);
    cmd = am->registerAction(m_zoomAction, Constants::ZOOM_WINDOW, m_globalContext);
    mwindow->addAction(cmd, Constants::G_WINDOW_SIZE);
    connect(m_zoomAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    // Window separator
    cmd = createSeparator(am, this, QLatin1String("QtCreator.Window.Sep.Size"), m_globalContext);
    mwindow->addAction(cmd, Constants::G_WINDOW_SIZE);
#endif

#ifndef Q_WS_MAC
    // Full Screen Action
    m_toggleFullScreenAction = new QAction(tr("Full Screen"), this);
    m_toggleFullScreenAction->setCheckable(true);
    cmd = am->registerAction(m_toggleFullScreenAction, Constants::TOGGLE_FULLSCREEN, m_globalContext);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+Shift+F11"));
    mwindow->addAction(cmd, Constants::G_WINDOW_SIZE);
    connect(m_toggleFullScreenAction, SIGNAL(triggered(bool)), this, SLOT(setFullScreen(bool)));
#endif

    //Help Action
    tmpaction = new QAction(QIcon(Constants::ICON_HELP), tr("&Help..."), this);
    cmd = am->registerAction(tmpaction, Constants::G_HELP_HELP, m_globalContext);
    mhelp->addAction(cmd, Constants::G_HELP_HELP);
    tmpaction->setEnabled(true);
#ifdef Q_WS_MAC
    cmd->action()->setMenuRole(QAction::ApplicationSpecificRole);
#endif
    connect(tmpaction, SIGNAL(triggered()), this,  SLOT(aboutPlugins()));

    // About sep
#ifndef Q_WS_MAC // doesn't have the "About" actions in the Help menu
    tmpaction = new QAction(this);
    tmpaction->setSeparator(true);
    cmd = am->registerAction(tmpaction, QLatin1String("QtCreator.Help.Sep.About"), m_globalContext);
    mhelp->addAction(cmd, Constants::G_HELP_ABOUT);
#endif

    //About Plugins Action
    tmpaction = new QAction(QIcon(Constants::ICON_PLUGIN), tr("About &Plugins..."), this);
    cmd = am->registerAction(tmpaction, Constants::ABOUT_PLUGINS, m_globalContext);
    mhelp->addAction(cmd, Constants::G_HELP_ABOUT);
    tmpaction->setEnabled(true);
#ifdef Q_WS_MAC
    cmd->action()->setMenuRole(QAction::ApplicationSpecificRole);
#endif
    connect(tmpaction, SIGNAL(triggered()), this,  SLOT(aboutPlugins()));

    // About GCS Action
#ifdef Q_WS_MAC
    tmpaction = new QAction(QIcon(Constants::ICON_OPENPILOT), tr("About &OpenPilot GCS"), this); // it's convention not to add dots to the about menu
#else
    tmpaction = new QAction(QIcon(Constants::ICON_OPENPILOT), tr("About &OpenPilot GCS..."), this);
#endif
    cmd = am->registerAction(tmpaction, Constants::ABOUT_OPENPILOTGCS, m_globalContext);
    mhelp->addAction(cmd, Constants::G_HELP_ABOUT);
    tmpaction->setEnabled(true);
#ifdef Q_WS_MAC
    cmd->action()->setMenuRole(QAction::ApplicationSpecificRole);
#endif
    connect(tmpaction, SIGNAL(triggered()), this,  SLOT(aboutOpenPilotGCS()));

    //Credits Action
    tmpaction = new QAction(QIcon(Constants::ICON_PLUGIN), tr("About &Authors..."), this);
    cmd = am->registerAction(tmpaction, Constants::ABOUT_AUTHORS, m_globalContext);
    mhelp->addAction(cmd, Constants::G_HELP_ABOUT);
    tmpaction->setEnabled(true);
#ifdef Q_WS_MAC
    cmd->action()->setMenuRole(QAction::ApplicationSpecificRole);
#endif
    connect(tmpaction, SIGNAL(triggered()), this,  SLOT(aboutOpenPilotAuthors()));


}

void MainWindow::newFile()
{
}

void MainWindow::openFile()
{
}

/*static QList<IFileFactory*> getNonEditorFileFactories()
{
    QList<IFileFactory*> tmp;
    return tmp;
}

static IFileFactory *findFileFactory(const QList<IFileFactory*> &fileFactories,
                                     const MimeDatabase *db,
                                     const QFileInfo &fi)
{
    if (const MimeType mt = db->findByFile(fi)) {
        const QString type = mt.type();
        foreach (IFileFactory *factory, fileFactories) {
            if (factory->mimeTypes().contains(type))
                return factory;
        }
    }
    return 0;
}

// opens either an editor or loads a project
void MainWindow::openFiles(const QStringList &fileNames)
{
    QList<IFileFactory*> nonEditorFileFactories = getNonEditorFileFactories();

    foreach (const QString &fileName, fileNames) {
        const QFileInfo fi(fileName);
        const QString absoluteFilePath = fi.absoluteFilePath();
        if (IFileFactory *fileFactory = findFileFactory(nonEditorFileFactories, mimeDatabase(), fi)) {
            fileFactory->open(absoluteFilePath);
        } else {

        }
    }
}*/

void MainWindow::setFocusToEditor()
{

}

bool MainWindow::showOptionsDialog(const QString &category,
                                   const QString &page,
                                   QWidget *parent)
{
    emit m_coreImpl->optionsDialogRequested();
    if (!parent)
        parent = this;
    SettingsDialog dlg(parent, category, page);
    return dlg.execDialog();
}

void MainWindow::saveAll()
{
    if ( m_dontSaveSettings) return;

    emit m_coreImpl->saveSettingsRequested();
    saveSettings(); // OpenPilot-specific.
}

void MainWindow::exit()
{
    // this function is most likely called from a user action
    // that is from an event handler of an object
    // since on close we are going to delete everything
    // so to prevent the deleting of that object we
    // just append it
    QTimer::singleShot(0, this,  SLOT(close()));
}

void MainWindow::openFileWith()
{

}

ActionManager *MainWindow::actionManager() const
{
    return m_actionManager;
}

UniqueIDManager *MainWindow::uniqueIDManager() const
{
    return m_uniqueIDManager;
}

MessageManager *MainWindow::messageManager() const
{
    return m_messageManager;
}

QSettings *MainWindow::settings(QSettings::Scope scope) const
{
    if (scope == QSettings::UserScope)
        return m_settings;
    else
        return m_globalSettings;
}

VariableManager *MainWindow::variableManager() const
{
     return m_variableManager;
}

ThreadManager *MainWindow::threadManager() const
{
     return m_threadManager;
}

ConnectionManager *MainWindow::connectionManager() const
{
    return m_connectionManager;
}

void MainWindow::addUAVGadgetManager(UAVGadgetManager *manager)
{
    if (!m_uavGadgetManagers.contains(manager))
        m_uavGadgetManagers.append(manager);
}

QList<UAVGadgetManager*> MainWindow::uavGadgetManagers() const
{
    return m_uavGadgetManagers;
}

UAVGadgetInstanceManager *MainWindow::uavGadgetInstanceManager() const
{
    return m_uavGadgetInstanceManager;
}


ModeManager *MainWindow::modeManager() const
{
    return m_modeManager;
}

MimeDatabase *MainWindow::mimeDatabase() const
{
    return m_mimeDatabase;
}

IContext *MainWindow::contextObject(QWidget *widget)
{
    return m_contextWidgets.value(widget);
}

void MainWindow::addContextObject(IContext *context)
{
    if (!context)
        return;
    QWidget *widget = context->widget();
    if (m_contextWidgets.contains(widget))
        return;

    m_contextWidgets.insert(widget, context);
}

void MainWindow::removeContextObject(IContext *context)
{
    if (!context)
        return;

    QWidget *widget = context->widget();
    if (!m_contextWidgets.contains(widget))
        return;

    m_contextWidgets.remove(widget);
    if (m_activeContext == context)
        updateContextObject(0);
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    if (e->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            if (debugMainWindow)
                qDebug() << "main window activated";
            emit windowActivated();
        }
    } else if (e->type() == QEvent::WindowStateChange) {
#ifdef Q_WS_MAC
        bool minimized = isMinimized();
        if (debugMainWindow)
            qDebug() << "main window state changed to minimized=" << minimized;
        m_minimizeAction->setEnabled(!minimized);
        m_zoomAction->setEnabled(!minimized);
#else
        bool isFullScreen = (windowState() & Qt::WindowFullScreen) != 0;
        m_toggleFullScreenAction->setChecked(isFullScreen);
#endif
    }
}

void MainWindow::updateFocusWidget(QWidget *old, QWidget *now)
{
    Q_UNUSED(old)

    // Prevent changing the context object just because the menu is activated
    if (qobject_cast<QMenuBar*>(now))
        return;

    IContext *newContext = 0;
    if (focusWidget()) {
        IContext *context = 0;
        QWidget *p = focusWidget();
        while (p) {
            context = m_contextWidgets.value(p);
            if (context) {
                newContext = context;
                break;
            }
            p = p->parentWidget();
        }
    }
    updateContextObject(newContext);
}

void MainWindow::updateContextObject(IContext *context)
{
    if (context == m_activeContext)
        return;
    IContext *oldContext = m_activeContext;
    m_activeContext = context;
    if (!context || oldContext != m_activeContext) {
        emit m_coreImpl->contextAboutToChange(context);
        updateContext();
        if (debugMainWindow)
            qDebug() << "new context object =" << context << (context ? context->widget() : 0)
                     << (context ? context->widget()->metaObject()->className() : 0);
        emit m_coreImpl->contextChanged(context);
    }
}

void MainWindow::resetContext()
{
    updateContextObject(0);
}

void MainWindow::shutdown()
{
    disconnect(QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)),
               this, SLOT(updateFocusWidget(QWidget*,QWidget*)));
    m_activeContext = 0;

    // We have to remove all the existing gagdets at his point, not
    // later!
    uavGadgetInstanceManager()->removeAllGadgets();
}

void MainWindow::createWorkspaces() {

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();

    UAVGadgetMode *uavGadgetMode;
    Core::UAVGadgetManager *uavGadgetManager;

    while (!m_uavGadgetModes.isEmpty()){
        pm->removeObject(m_uavGadgetModes.takeFirst());
    }
    while (!m_uavGadgetManagers.isEmpty()){
        Core::UAVGadgetManager* uavGadgetManager = m_uavGadgetManagers.takeLast();
        uavGadgetManager->removeAllSplits();
        // TODO Fixthis: This only happens, when settings are reloaded.
        //      then the old GadgetManagers should be deleted, but if
        //      I delete them the GCS segfaults.
        //delete uavGadgetManager;
    }
    m_uavGadgetManagers.clear();
    m_uavGadgetModes.clear();
    for (int i = 0; i < m_workspaceSettings->numberOfWorkspaces(); ++i) {

        uavGadgetManager = new Core::UAVGadgetManager(CoreImpl::instance(), this);
        uavGadgetManager->hide();
        const QString name     = m_workspaceSettings->name(i);
        const QString iconName = m_workspaceSettings->iconName(i);
        const QString modeName = m_workspaceSettings->modeName(i);

        uavGadgetMode = new UAVGadgetMode(uavGadgetManager, name,
                                          QIcon(iconName), 90-i+1, modeName);
        uavGadgetManager->setUAVGadgetMode(uavGadgetMode);
        m_uavGadgetModes.append(uavGadgetMode);
        pm->addObject(uavGadgetMode);
        addUAVGadgetManager(uavGadgetManager);
    }
}

static const char *settingsGroup = "MainWindow";
static const char *geometryKey = "Geometry";
static const char *colorKey = "Color";
static const char *maxKey = "Maximized";
static const char *fullScreenKey = "FullScreen";

void MainWindow::readSettings(QSettings* qs)
{
    if ( !qs ){
        qs = m_settings;
    }

    m_generalSettings->readSettings(qs);
    m_actionManager->readSettings(qs);

    qs->beginGroup(QLatin1String(settingsGroup));

    Utils::StyleHelper::setBaseColor(qs->value(QLatin1String(colorKey)).value<QColor>());

    const QVariant geom = qs->value(QLatin1String(geometryKey));
    if (geom.isValid()) {
        setGeometry(geom.toRect());
    } else {
        resize(750, 400);
    }
    if (qs->value(QLatin1String(maxKey), false).toBool())
        setWindowState(Qt::WindowMaximized);
    setFullScreen(qs->value(QLatin1String(fullScreenKey), false).toBool());

    qs->endGroup();

    m_workspaceSettings->readSettings(qs);

    createWorkspaces();

    foreach (UAVGadgetManager *manager, m_uavGadgetManagers) {
        manager->readSettings(qs);
    }

    m_viewManager->readSettings(qs);

}


void MainWindow::saveSettings(QSettings* qs)
{
    if ( m_dontSaveSettings ) return;

    if ( !qs ){
        qs = m_settings;
    }

    m_workspaceSettings->saveSettings(qs);

    qs->beginGroup(QLatin1String(settingsGroup));

    qs->setValue(QLatin1String(colorKey), Utils::StyleHelper::baseColor());

    if (windowState() & (Qt::WindowMaximized | Qt::WindowFullScreen)) {
        qs->setValue(QLatin1String(maxKey), (bool) (windowState() & Qt::WindowMaximized));
        qs->setValue(QLatin1String(fullScreenKey), (bool) (windowState() & Qt::WindowFullScreen));
    } else {
        qs->setValue(QLatin1String(maxKey), false);
        qs->setValue(QLatin1String(fullScreenKey), false);
        qs->setValue(QLatin1String(geometryKey), geometry());
    }

    qs->endGroup();

    foreach (UAVGadgetManager *manager, m_uavGadgetManagers) {
        manager->saveSettings(qs);
    }

    m_viewManager->saveSettings(qs);
    m_actionManager->saveSettings(qs);
    m_generalSettings->saveSettings(qs);

}

void MainWindow::readSettings(IConfigurablePlugin* plugin, QSettings* qs)
{
    if ( !qs ){
        qs = m_settings;
    }

    UAVConfigInfo configInfo;
    QObject* qo = reinterpret_cast<QObject *>(plugin);
    QString configName = qo->metaObject()->className();

    qs->beginGroup("Plugins");
    qs->beginGroup(configName);
    configInfo.read(qs);
    configInfo.setNameOfConfigurable("Plugin-"+configName);
    qs->beginGroup("data");
    plugin->readConfig(qs, &configInfo);

    qs->endGroup();
    qs->endGroup();
    qs->endGroup();

}

void MainWindow::saveSettings(IConfigurablePlugin* plugin, QSettings* qs)
{
    if ( m_dontSaveSettings ) return;
    if ( !qs ){
        qs = m_settings;
    }

    UAVConfigInfo configInfo;
    QString configName = plugin->metaObject()->className();

    qs->beginGroup("Plugins");
    qs->beginGroup(configName);
    qs->beginGroup("data");
    plugin->saveConfig(qs, &configInfo);
    qs->endGroup();
    configInfo.save(qs);
    qs->endGroup();
    qs->endGroup();

}

void MainWindow::deleteSettings()
{
    m_settings->clear();
    m_settings->sync();
    m_dontSaveSettings = true;
}

void MainWindow::addAdditionalContext(int context)
{
    if (context == 0)
        return;

    if (!m_additionalContexts.contains(context))
        m_additionalContexts.prepend(context);
}

void MainWindow::removeAdditionalContext(int context)
{
    if (context == 0)
        return;

    int index = m_additionalContexts.indexOf(context);
    if (index != -1)
        m_additionalContexts.removeAt(index);
}

bool MainWindow::hasContext(int context) const
{
    return m_actionManager->hasContext(context);
}

void MainWindow::updateContext()
{
    QList<int> contexts;

    if (m_activeContext)
        contexts += m_activeContext->context();

    contexts += m_additionalContexts;

    QList<int> uniquecontexts;
    for (int i = 0; i < contexts.size(); ++i) {
        const int c = contexts.at(i);
        if (!uniquecontexts.contains(c))
            uniquecontexts << c;
    }

    m_actionManager->setContext(uniquecontexts);
}

void MainWindow::aboutToShowRecentFiles()
{
    ActionContainer *aci =
        m_actionManager->actionContainer(Constants::M_FILE_RECENTFILES);
    if (aci) {
        aci->menu()->clear();

        bool hasRecentFiles = false;

        aci->menu()->setEnabled(hasRecentFiles);
    }
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;
    QString fileName = action->data().toString();
    if (!fileName.isEmpty()) {
    }
}

void MainWindow::aboutOpenPilotGCS()
{
    if (!m_versionDialog) {
        m_versionDialog = new VersionDialog(this);
        connect(m_versionDialog, SIGNAL(finished(int)),
                this, SLOT(destroyVersionDialog()));
    }
    m_versionDialog->show();
}

void MainWindow::destroyVersionDialog()
{
    if (m_versionDialog) {
        m_versionDialog->deleteLater();
        m_versionDialog = 0;
    }
}

void MainWindow::aboutOpenPilotAuthors()
{
    if (!m_authorsDialog) {
        m_authorsDialog = new AuthorsDialog(this);
        connect(m_authorsDialog, SIGNAL(finished(int)),
                this, SLOT(destroyAuthorsDialog()));
    }
    m_authorsDialog->show();
}

void MainWindow::destroyAuthorsDialog()
{
    if (m_authorsDialog) {
        m_authorsDialog->deleteLater();
        m_authorsDialog = 0;
    }
}


void MainWindow::aboutPlugins()
{
    PluginDialog dialog(this);
    dialog.exec();
}

void MainWindow::setFullScreen(bool on)
{
    if (bool(windowState() & Qt::WindowFullScreen) == on)
        return;

    if (on) {
        setWindowState(windowState() | Qt::WindowFullScreen);
        //statusBar()->hide();
        //menuBar()->hide();
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        //menuBar()->show();
        //statusBar()->show();
    }
}

// Display a warning with an additional button to open
// the debugger settings dialog if settingsId is nonempty.

bool MainWindow::showWarningWithOptions(const QString &title,
                                        const QString &text,
                                        const QString &details,
                                        const QString &settingsCategory,
                                        const QString &settingsId,
                                        QWidget *parent)
{
    if (parent == 0)
        parent = this;
    QMessageBox msgBox(QMessageBox::Warning, title, text,
                       QMessageBox::Ok, parent);
    if (details.isEmpty())
        msgBox.setDetailedText(details);
    QAbstractButton *settingsButton = 0;
    if (!settingsId.isEmpty() || !settingsCategory.isEmpty())
        settingsButton = msgBox.addButton(tr("Settings..."), QMessageBox::AcceptRole);
    msgBox.exec();
    if (settingsButton && msgBox.clickedButton() == settingsButton) {
        return showOptionsDialog(settingsCategory, settingsId);
    }
    return false;
}
