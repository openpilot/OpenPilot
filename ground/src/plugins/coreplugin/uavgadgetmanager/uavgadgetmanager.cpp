/**
 ******************************************************************************
 *
 * @file       uavgadgetmanager.cpp
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

#include "uavgadgetmanager.h"
#include "uavgadgetview.h"
#include "uavgadgetmode.h"
#include "iuavgadgetfactory.h"
#include "iuavgadget.h"
#include "icore.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/baseview.h>
#include <coreplugin/imode.h>
#include <coreplugin/settingsdatabase.h>
#include <coreplugin/variablemanager.h>

#include <extensionsystem/pluginmanager.h>

#include <utils/consoleprocess.h>
#include <utils/qtcassert.h>

#include <QtCore/QDebug>
#include <QtCore/QMap>
#include <QtCore/QProcess>
#include <QtCore/QSet>
#include <QtCore/QSettings>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QLayout>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QSplitter>
#include <QtGui/QStackedLayout>

Q_DECLARE_METATYPE(Core::IUAVGadget*)

using namespace Core;
using namespace Core::Internal;
using namespace Utils;

enum { debugUAVGadgetManager=0 };

static inline ExtensionSystem::PluginManager *pluginManager()
{
    return ExtensionSystem::PluginManager::instance();
}

//===================UAVGadgetManager=====================

//UAVGadgetManagerPlaceHolder *UAVGadgetManagerPlaceHolder::m_current = 0;

UAVGadgetManagerPlaceHolder::UAVGadgetManagerPlaceHolder(Core::Internal::UAVGadgetMode *mode, QWidget *parent)
    : QWidget(parent),
    m_uavGadgetMode(mode),
    m_current(0)
{
    m_mode = dynamic_cast<Core::IMode*>(mode);
    setLayout(new QVBoxLayout);
    layout()->setMargin(0);
    connect(Core::ModeManager::instance(), SIGNAL(currentModeChanged(Core::IMode *)),
            this, SLOT(currentModeChanged(Core::IMode *)));

    currentModeChanged(Core::ModeManager::instance()->currentMode());
}

UAVGadgetManagerPlaceHolder::~UAVGadgetManagerPlaceHolder()
{
    if (m_current == this) {
        m_uavGadgetMode->uavGadgetManager()->setParent(0);
        m_uavGadgetMode->uavGadgetManager()->hide();
    }
}

void UAVGadgetManagerPlaceHolder::currentModeChanged(Core::IMode *mode)
{
    UAVGadgetManager *gm = m_uavGadgetMode->uavGadgetManager();
    if (m_current == this) {
        m_current = 0;
        gm->setParent(0);
        gm->hide();
    }
    if (m_mode == mode) {
        m_current = this;
        layout()->addWidget(gm);
        gm->hideToolbars(gm->toolbarsHidden());
        gm->show();
    }
}

// ---------------- UAVGadgetManager

namespace Core {


struct UAVGadgetManagerPrivate {
    explicit UAVGadgetManagerPrivate(ICore *core, QWidget *parent);
    ~UAVGadgetManagerPrivate();
    Internal::UAVGadgetView *m_view;
    Internal::SplitterOrView *m_splitterOrView;
    QPointer<IUAVGadget> m_currentUAVGadget;

    ICore *m_core;

    // actions
    static QAction *m_hideToolbarsAction;
    static QAction *m_splitAction;
    static QAction *m_splitSideBySideAction;
    static QAction *m_removeCurrentSplitAction;
    static QAction *m_removeAllSplitsAction;
    static QAction *m_gotoOtherSplitAction;

    Internal::UAVGadgetClosingCoreListener *m_coreListener;
};
}

QAction *UAVGadgetManagerPrivate::m_hideToolbarsAction = 0;
QAction *UAVGadgetManagerPrivate::m_splitAction = 0;
QAction *UAVGadgetManagerPrivate::m_splitSideBySideAction = 0;
QAction *UAVGadgetManagerPrivate::m_removeCurrentSplitAction = 0;
QAction *UAVGadgetManagerPrivate::m_removeAllSplitsAction = 0;
QAction *UAVGadgetManagerPrivate::m_gotoOtherSplitAction = 0;

UAVGadgetManagerPrivate::UAVGadgetManagerPrivate(ICore *core, QWidget *parent) :
    m_view(0),
    m_splitterOrView(0),
    m_core(core),
    m_coreListener(0)
{
}

UAVGadgetManagerPrivate::~UAVGadgetManagerPrivate()
{
}

UAVGadgetManager::UAVGadgetManager(ICore *core, QWidget *parent) :
    QWidget(parent),
    m_hidden(false),
    m_d(new UAVGadgetManagerPrivate(core, parent))
{

    connect(m_d->m_core, SIGNAL(contextAboutToChange(Core::IContext *)),
            this, SLOT(handleContextChange(Core::IContext *)));
    const QList<int> uavGadgetManagerContext =
            QList<int>() << m_d->m_core->uniqueIDManager()->uniqueIdentifier(Constants::C_UAVGADGETMANAGER);

    ActionManager *am = m_d->m_core->actionManager();

    //Window Menu
    ActionContainer *mwindow = am->actionContainer(Constants::M_WINDOW);
    Command *cmd;

    // The actions m_d->m_hideToolbarsAction etc are common to all instances of UAVGadgetManager
    // which means that they share the menu items/signals in the Window menu.
    // That is, they all connect their slots to the same signal and have to check in the slot
    // if the current mode is their mode, otherwise they just ignore the signal.
    // The first UAVGadgetManager creates the actions, and the following just use them
    // (This also implies that they share the same context.)
    if (m_d->m_hideToolbarsAction == 0)
    {
        //Window menu separators
        QAction *tmpaction = new QAction(this);
        tmpaction->setSeparator(true);
        cmd = am->registerAction(tmpaction, QLatin1String("OpenPilot.Window.Sep.Split"), uavGadgetManagerContext);
        mwindow->addAction(cmd, Constants::G_WINDOW_HIDE_TOOLBAR);

        m_d->m_hideToolbarsAction = new QAction(tr("Hide toolbars"), this);
        m_d->m_hideToolbarsAction->setCheckable(true);
        cmd = am->registerAction(m_d->m_hideToolbarsAction, Constants::HIDE_TOOLBARS, uavGadgetManagerContext);
        cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F10")));
        mwindow->addAction(cmd, Constants::G_WINDOW_HIDE_TOOLBAR);

        //Window menu separators
        QAction *tmpaction2 = new QAction(this);
        tmpaction2->setSeparator(true);
        cmd = am->registerAction(tmpaction2, QLatin1String("OpenPilot.Window.Sep.Split2"), uavGadgetManagerContext);
        mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    }
    connect(m_d->m_hideToolbarsAction, SIGNAL(triggered(bool)), this, SLOT(hideToolbars(bool)));

#ifdef Q_WS_MAC
    QString prefix = tr("Meta+Shift");
#else
    QString prefix = tr("Ctrl+Shift");
#endif

    if (m_d->m_splitAction == 0)
    {
        m_d->m_splitAction = new QAction(tr("Split"), this);
        cmd = am->registerAction(m_d->m_splitAction, Constants::SPLIT, uavGadgetManagerContext);
        cmd->setDefaultKeySequence(QKeySequence(tr("%1+Down").arg(prefix)));
        mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    }
    connect(m_d->m_splitAction, SIGNAL(triggered()), this, SLOT(split()));

    if (m_d->m_splitSideBySideAction == 0)
    {
        m_d->m_splitSideBySideAction = new QAction(tr("Split Side by Side"), this);
        cmd = am->registerAction(m_d->m_splitSideBySideAction, Constants::SPLIT_SIDE_BY_SIDE, uavGadgetManagerContext);
        cmd->setDefaultKeySequence(QKeySequence(tr("%1+Right").arg(prefix)));
        mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    }
    connect(m_d->m_splitSideBySideAction, SIGNAL(triggered()), this, SLOT(splitSideBySide()));

    if (m_d->m_removeCurrentSplitAction == 0)
    {
        m_d->m_removeCurrentSplitAction = new QAction(tr("Close Current View"), this);
        cmd = am->registerAction(m_d->m_removeCurrentSplitAction, Constants::REMOVE_CURRENT_SPLIT, uavGadgetManagerContext);
        cmd->setDefaultKeySequence(QKeySequence(tr("%1+C").arg(prefix)));
        mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    }
    connect(m_d->m_removeCurrentSplitAction, SIGNAL(triggered()), this, SLOT(removeCurrentSplit()));

    if (m_d->m_removeAllSplitsAction == 0)
    {
        m_d->m_removeAllSplitsAction = new QAction(tr("Close All Other Views"), this);
        cmd = am->registerAction(m_d->m_removeAllSplitsAction, Constants::REMOVE_ALL_SPLITS, uavGadgetManagerContext);
        cmd->setDefaultKeySequence(QKeySequence(tr("%1+A").arg(prefix)));
        mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    }
    connect(m_d->m_removeAllSplitsAction, SIGNAL(triggered()), this, SLOT(removeAllSplits()));

    if (m_d->m_gotoOtherSplitAction == 0)
    {
        m_d->m_gotoOtherSplitAction = new QAction(tr("Goto Next View"), this);
        cmd = am->registerAction(m_d->m_gotoOtherSplitAction, Constants::GOTO_OTHER_SPLIT, uavGadgetManagerContext);
        cmd->setDefaultKeySequence(QKeySequence(tr("%1+N").arg(prefix)));
        mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    }
    connect(m_d->m_gotoOtherSplitAction, SIGNAL(triggered()), this, SLOT(gotoOtherSplit()));

    // other setup
    m_d->m_splitterOrView = new SplitterOrView(this, 0, true);
    m_d->m_view = m_d->m_splitterOrView->view();
    setCurrentUAVGadget(m_d->m_view->uavGadget());

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_d->m_splitterOrView);

    updateActions();
}

UAVGadgetManager::~UAVGadgetManager()
{
    if (m_d->m_core) {
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        if (m_d->m_coreListener) {
            pm->removeObject(m_d->m_coreListener);
            delete m_d->m_coreListener;
        }
    }
    delete m_d;
}

void UAVGadgetManager::init()
{
    QList<int> context;
    context << m_d->m_core->uniqueIDManager()->uniqueIdentifier("OpenPilot.UAVGadgetManager");

    m_d->m_coreListener = new UAVGadgetClosingCoreListener(this);

    pluginManager()->addObject(m_d->m_coreListener);
}

void UAVGadgetManager::handleContextChange(Core::IContext *context)
{
//    if (debugUAVGadgetManager)
//        qDebug() << Q_FUNC_INFO << context;
    IUAVGadget *uavGadget = context ? qobject_cast<IUAVGadget*>(context) : 0;
    if (uavGadget) {
        setCurrentUAVGadget(uavGadget);
    } else {
        updateActions();
    }
}

void UAVGadgetManager::setCurrentUAVGadget(IUAVGadget *uavGadget)
{
    if (m_d->m_currentUAVGadget == uavGadget)
        return;

    SplitterOrView *oldView = currentSplitterOrView();
    m_d->m_currentUAVGadget = uavGadget;
    SplitterOrView *view = currentSplitterOrView();
    if (oldView != view) {
        if (oldView)
            oldView->update();
        if (view)
            view->update();
    }
    uavGadget->widget()->setFocus();
    updateActions();
//    emit currentUAVGadgetChanged(uavGadget);
}

/* Contract: return current SplitterOrView.
 * Implications: must not return SplitterOrView that is splitter.
 */
Core::Internal::SplitterOrView *UAVGadgetManager::currentSplitterOrView() const
{
    if (!m_d->m_splitterOrView) // this is only for startup
        return 0;
    SplitterOrView *view = m_d->m_currentUAVGadget ?
                           m_d->m_splitterOrView->findView(m_d->m_currentUAVGadget) :
                           0;
    return view;
}

IUAVGadget *UAVGadgetManager::currentUAVGadget() const
{
    return m_d->m_currentUAVGadget;
}

void UAVGadgetManager::emptyView(Core::Internal::UAVGadgetView *view)
{
    if (!view)
        return;

    IUAVGadget *uavGadget = view->uavGadget();
//    emit uavGadgetAboutToClose(uavGadget);
    removeUAVGadget(uavGadget);
    view->removeUAVGadget();
//    emit uavGadgetsClosed(uavGadgets);
}


void UAVGadgetManager::closeView(Core::Internal::UAVGadgetView *view)
{
    if (!view)
        return;
    SplitterOrView *splitterOrView = m_d->m_splitterOrView->findView(view);
    Q_ASSERT(splitterOrView);
    Q_ASSERT(splitterOrView->view() == view);
    if (splitterOrView == m_d->m_splitterOrView)
        return;

    emptyView(view);

    SplitterOrView *splitter = m_d->m_splitterOrView->findSplitter(splitterOrView);
    Q_ASSERT(splitterOrView->hasUAVGadget() == false);
    Q_ASSERT(splitter->isSplitter() == true);
    splitterOrView->hide();
    delete splitterOrView;

    splitter->unsplit();

    SplitterOrView *newCurrent = splitter->findFirstView();
    Q_ASSERT(newCurrent);
    if (newCurrent)
        setCurrentUAVGadget(newCurrent->uavGadget());
    updateActions();
}

void UAVGadgetManager::addUAVGadget(IUAVGadget *uavGadget)
{
    if (!uavGadget)
        return;
    m_d->m_core->addContextObject(uavGadget);

//   emit uavGadgetOpened(uavGadget);
}

void UAVGadgetManager::removeUAVGadget(IUAVGadget *uavGadget)
{
    if (!uavGadget)
        return;
    m_d->m_core->removeContextObject(qobject_cast<IContext*>(uavGadget));
}

void UAVGadgetManager::ensureUAVGadgetManagerVisible()
{
    if (!isVisible())
        m_d->m_core->modeManager()->activateMode(m_uavGadgetMode->uniqueModeName());
}

void UAVGadgetManager::updateActions()
{
    if (!m_d->m_splitterOrView) // this is only for startup
        return;
    // Splitting is only possible when the toolbars are shown
    bool hidden = m_d->m_hideToolbarsAction->isChecked();
    bool hasSplitter = m_d->m_splitterOrView->isSplitter();
    m_d->m_removeCurrentSplitAction->setEnabled(!hidden && hasSplitter);
    m_d->m_removeAllSplitsAction->setEnabled(!hidden && hasSplitter);
    m_d->m_gotoOtherSplitAction->setEnabled(!hidden && hasSplitter);
}

UAVGadgetFactoryList UAVGadgetManager::uavGadgetFactories() const
{
    UAVGadgetFactoryList rc = pluginManager()->getObjects<Core::IUAVGadgetFactory>();
    //if (debugUAVGadgetManager)
        //qDebug() << Q_FUNC_INFO << rc;
    return rc;
}

QByteArray UAVGadgetManager::saveState() const
{
    QByteArray bytes;
//    QDataStream stream(&bytes, QIODevice::WriteOnly);
//
//    stream << QByteArray("UAVGadgetManagerV4");
//
//    QList<IUAVGadget *> uavGadgets = openedUAVGadgets();
//    foreach (IUAVGadget *uavGadget, uavGadgets) {
//        if (!uavGadget->file()->fileName().isEmpty()) {
//            QByteArray state = uavGadget->saveState();
//            if (!state.isEmpty())
//                m_d->m_uavGadgetStates.insert(uavGadget->file()->fileName(), QVariant(state));
//        }
//    }
//
//    stream << m_d->m_uavGadgetStates;
//
//    QList<OpenUAVGadgetsModel::Entry> entries = m_d->m_uavGadgetModel->entries();
//    stream << entries.count();
//
//    foreach (OpenUAVGadgetsModel::Entry entry, entries) {
//        stream << entry.fileName() << entry.displayName() << entry.kind();
//    }
//
//    stream << m_d->m_splitterOrView->saveState();

    return bytes;
}

bool UAVGadgetManager::restoreState(const QByteArray &state)
{
//    closeAllUAVGadgets(true);
//    removeAllSplits();
//    QDataStream stream(state);
//
//    QByteArray version;
//    stream >> version;
//
//    if (version != "UAVGadgetManagerV4")
//        return false;
//
//    QMap<QString, QVariant> uavGadgetstates;
//
//    QApplication::setOverrideCursor(Qt::WaitCursor);
//
//    stream >> uavGadgetstates;
//
//    QMapIterator<QString, QVariant> i(uavGadgetstates);
//    while (i.hasNext()) {
//        i.next();
//        m_d->m_uavGadgetStates.insert(i.key(), i.value());
//    }
//
//    int uavGadgetCount = 0;
//    stream >> uavGadgetCount;
//    while (--uavGadgetCount >= 0) {
//        QString fileName;
//        stream >> fileName;
//        QString displayName;
//        stream >> displayName;
//        QByteArray kind;
//        stream >> kind;
//
//        if (!fileName.isEmpty() && !displayName.isEmpty()){
//            m_d->m_uavGadgetModel->addRestoredUAVGadget(fileName, displayName, kind);
//        }
//    }
//
//    QByteArray splitterstates;
//    stream >> splitterstates;
//    m_d->m_splitterOrView->restoreState(splitterstates);
//
//    // splitting and stuff results in focus trouble, that's why we set the focus again after restoration
//    ensureUAVGadgetManagerVisible();
//    if (m_d->m_currentUAVGadget) {
//        m_d->m_currentUAVGadget->widget()->setFocus();
//    } else if (Core::Internal::SplitterOrView *view = currentSplitterOrView()) {
//        if (IUAVGadget *e = view->uavGadget())
//            e->widget()->setFocus();
//        else if (view->view())
//            view->view()->setFocus();
//    }
//
//    QApplication::restoreOverrideCursor();

    return true;
}

void UAVGadgetManager::saveSettings()
{
//    SettingsDatabase *settings = m_d->m_core->settingsDatabase();
//    settings->setValue(QLatin1String(documentStatesKey), m_d->m_uavGadgetStates);
//    settings->setValue(QLatin1String(externalUAVGadgetKey), m_d->m_externalUAVGadget);
//    settings->setValue(QLatin1String(reloadBehaviorKey), m_d->m_reloadBehavior);
}

void UAVGadgetManager::readSettings()
{
//    // Backward compatibility to old locations for these settings
//    QSettings *qs = m_d->m_core->settings();
//    if (qs->contains(QLatin1String(documentStatesKey))) {
//        m_d->m_uavGadgetStates = qs->value(QLatin1String(documentStatesKey))
//            .value<QMap<QString, QVariant> >();
//        qs->remove(QLatin1String(documentStatesKey));
//    }
//    if (qs->contains(QLatin1String(externalUAVGadgetKey))) {
//        m_d->m_externalUAVGadget = qs->value(QLatin1String(externalUAVGadgetKey)).toString();
//        qs->remove(QLatin1String(externalUAVGadgetKey));
//    }
//
//    SettingsDatabase *settings = m_d->m_core->settingsDatabase();
//    if (settings->contains(QLatin1String(documentStatesKey)))
//        m_d->m_uavGadgetStates = settings->value(QLatin1String(documentStatesKey))
//            .value<QMap<QString, QVariant> >();
//    if (settings->contains(QLatin1String(externalUAVGadgetKey)))
//        m_d->m_externalUAVGadget = settings->value(QLatin1String(externalUAVGadgetKey)).toString();
//
//    if (settings->contains(QLatin1String(reloadBehaviorKey)))
//        m_d->m_reloadBehavior = (IFile::ReloadBehavior)settings->value(QLatin1String(reloadBehaviorKey)).toInt();
}

void UAVGadgetManager::split(Qt::Orientation orientation)
{
    if (m_d->m_core->modeManager()->currentMode() != m_uavGadgetMode)
        return;

    IUAVGadget *uavGadget = m_d->m_currentUAVGadget;
    Q_ASSERT(uavGadget);
    SplitterOrView *view = currentSplitterOrView();
    Q_ASSERT(view);
    view->split(orientation);

    SplitterOrView *sor = m_d->m_splitterOrView->findView(uavGadget);
    SplitterOrView *next = m_d->m_splitterOrView->findNextView(sor);
    setCurrentUAVGadget(next->uavGadget());
    updateActions();
}

void UAVGadgetManager::split()
{
    split(Qt::Vertical);
}

void UAVGadgetManager::splitSideBySide()
{
    split(Qt::Horizontal);
}

void UAVGadgetManager::removeCurrentSplit()
{
    if (m_d->m_core->modeManager()->currentMode() != m_uavGadgetMode)
        return;

    SplitterOrView *viewToClose = currentSplitterOrView();
    if (viewToClose == m_d->m_splitterOrView)
        return;
    closeView(viewToClose->view());
    updateActions();
}

void UAVGadgetManager::removeAllSplits()
{
    if (m_d->m_core->modeManager()->currentMode() != m_uavGadgetMode)
        return;

    if (!m_d->m_splitterOrView->isSplitter())
        return;
    IUAVGadget *uavGadget = m_d->m_currentUAVGadget;
    m_d->m_currentUAVGadget = 0; // trigger update below
    m_d->m_splitterOrView->unsplitAll();
    m_d->m_splitterOrView->view()->setUAVGadget(uavGadget);
    setCurrentUAVGadget(uavGadget);
}

void UAVGadgetManager::gotoOtherSplit()
{
    if (m_d->m_core->modeManager()->currentMode() != m_uavGadgetMode)
        return;

    if (m_d->m_splitterOrView->isSplitter()) {
        SplitterOrView *currentView = currentSplitterOrView();
        SplitterOrView *view = m_d->m_splitterOrView->findNextView(currentView);
        if (!view)
            view = m_d->m_splitterOrView->findFirstView();
        if (view) {
            setCurrentUAVGadget(view->uavGadget());
        }
    }
}

void UAVGadgetManager::hideToolbars(bool hide)
{
     if (m_d->m_core->modeManager()->currentMode() != m_uavGadgetMode)
         return;

     m_d->m_hideToolbarsAction->setChecked(hide);
     m_hidden = hide;
     SplitterOrView *next = m_d->m_splitterOrView->findFirstView();
     do {
         next->view()->hideToolbar(hide);
         next = m_d->m_splitterOrView->findNextView(next);
     } while (next);

    m_d->m_splitAction->setEnabled(!hide);
    m_d->m_splitSideBySideAction->setEnabled(!hide);
    m_d->m_removeCurrentSplitAction->setEnabled(!hide);
    m_d->m_removeAllSplitsAction->setEnabled(!hide);
    m_d->m_gotoOtherSplitAction->setEnabled(!hide);
}
//===================UAVGadgetClosingCoreListener======================

UAVGadgetClosingCoreListener::UAVGadgetClosingCoreListener(UAVGadgetManager *em)
        : m_em(em)
{
}

bool UAVGadgetClosingCoreListener::coreAboutToClose()
{
    // Do not ask for files to save.
    // MainWindow::closeEvent has already done that.
    return true;
}
