/**
 ******************************************************************************
 *
 * @file       uavgadgetmanager.cpp
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

#include "uavgadgetmanager.h"
#include "uavgadgetview.h"
#include "splitterorview.h"
#include "uavgadgetmode.h"
#include "uavgadgetinstancemanager.h"
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
        gm->showToolbars(gm->toolbarsShown());
        gm->show();
    }
}

// ---------------- UAVGadgetManager

namespace Core {


struct UAVGadgetManagerPrivate {
    explicit UAVGadgetManagerPrivate(ICore *core, QWidget *parent);
    ~UAVGadgetManagerPrivate();

    // The root splitter or view.
    QPointer<Internal::SplitterOrView> m_splitterOrView;

    // The gadget which is currently 'active'.
    QPointer<IUAVGadget> m_currentGadget;

    QPointer<ICore> m_core;

    QPointer<Internal::UAVGadgetClosingCoreListener> m_coreListener;

    // actions
    static QAction *m_showToolbarsAction;
    static QAction *m_splitAction;
    static QAction *m_splitSideBySideAction;
    static QAction *m_removeCurrentSplitAction;
    static QAction *m_removeAllSplitsAction;
    static QAction *m_gotoOtherSplitAction;
};
}

QAction *UAVGadgetManagerPrivate::m_showToolbarsAction = 0;
QAction *UAVGadgetManagerPrivate::m_splitAction = 0;
QAction *UAVGadgetManagerPrivate::m_splitSideBySideAction = 0;
QAction *UAVGadgetManagerPrivate::m_removeCurrentSplitAction = 0;
QAction *UAVGadgetManagerPrivate::m_removeAllSplitsAction = 0;
QAction *UAVGadgetManagerPrivate::m_gotoOtherSplitAction = 0;

UAVGadgetManagerPrivate::UAVGadgetManagerPrivate(ICore *core, QWidget *parent) :
    m_splitterOrView(0),
    m_currentGadget(0),
    m_core(core),
    m_coreListener(0)
{
    Q_UNUSED(parent);
}

UAVGadgetManagerPrivate::~UAVGadgetManagerPrivate()
{
}

UAVGadgetManager::UAVGadgetManager(ICore *core, QWidget *parent) :
    QWidget(parent),
    m_showToolbars(false),
    m_d(new UAVGadgetManagerPrivate(core, parent)),
    m_uavGadgetMode(0)
{

    connect(m_d->m_core, SIGNAL(contextAboutToChange(Core::IContext *)),
            this, SLOT(handleContextChange(Core::IContext *)));
    const QList<int> uavGadgetManagerContext =
            QList<int>() << m_d->m_core->uniqueIDManager()->uniqueIdentifier(Constants::C_UAVGADGETMANAGER);

    ActionManager *am = m_d->m_core->actionManager();

    //Window Menu
    ActionContainer *mwindow = am->actionContainer(Constants::M_WINDOW);
    Command *cmd;

    // The actions m_d->m_showToolbarsAction etc are common to all instances of UAVGadgetManager
    // which means that they share the menu items/signals in the Window menu.
    // That is, they all connect their slots to the same signal and have to check in the slot
    // if the current mode is their mode, otherwise they just ignore the signal.
    // The first UAVGadgetManager creates the actions, and the following just use them
    // (This also implies that they share the same context.)
    if (m_d->m_showToolbarsAction == 0)
    {
        //Window menu separators
        QAction *tmpaction = new QAction(this);
        tmpaction->setSeparator(true);
        cmd = am->registerAction(tmpaction, QLatin1String("OpenPilot.Window.Sep.Split"), uavGadgetManagerContext);
        mwindow->addAction(cmd, Constants::G_WINDOW_HIDE_TOOLBAR);

        m_d->m_showToolbarsAction = new QAction(tr("Edit Gadgets Mode"), this);
        m_d->m_showToolbarsAction->setCheckable(true);
        cmd = am->registerAction(m_d->m_showToolbarsAction, Constants::HIDE_TOOLBARS, uavGadgetManagerContext);
        cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F10")));
        mwindow->addAction(cmd, Constants::G_WINDOW_HIDE_TOOLBAR);

        //Window menu separators
        QAction *tmpaction2 = new QAction(this);
        tmpaction2->setSeparator(true);
        cmd = am->registerAction(tmpaction2, QLatin1String("OpenPilot.Window.Sep.Split2"), uavGadgetManagerContext);
        mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    }
    connect(m_d->m_showToolbarsAction, SIGNAL(triggered(bool)), this, SLOT(showToolbars(bool)));

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
    // SplitterOrView with 0 as gadget calls our setCurrentGadget, which relies on currentSplitterOrView(),
    // which needs our m_splitterorView to be set, which isn't set yet at that time.
    // So directly set our currentGadget to 0, and do it again.
    m_d->m_currentGadget = 0;
    setCurrentGadget(m_d->m_splitterOrView->view()->gadget());

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_d->m_splitterOrView);

    showToolbars(m_showToolbars);
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
    if (uavGadget)
        setCurrentGadget(uavGadget);
    updateActions();
}

void UAVGadgetManager::setCurrentGadget(IUAVGadget *uavGadget)
{
    if (m_d->m_currentGadget == uavGadget)
        return;

    SplitterOrView *oldView = currentSplitterOrView();
    m_d->m_currentGadget = uavGadget;
    SplitterOrView *view = currentSplitterOrView();
    if (oldView != view) {
        if (oldView)
            oldView->update();
        if (view)
            view->update();
    }
    uavGadget->widget()->setFocus();
    emit currentGadgetChanged(uavGadget);
    updateActions();
//    emit currentGadgetChanged(uavGadget);
}

/* Contract: return current SplitterOrView.
 * Implications: must not return SplitterOrView that is splitter.
 */
Core::Internal::SplitterOrView *UAVGadgetManager::currentSplitterOrView() const
{
    if (!m_d->m_splitterOrView) // this is only for startup
        return 0;
    SplitterOrView *view = m_d->m_currentGadget ?
                           m_d->m_splitterOrView->findView(m_d->m_currentGadget) :
                           0;
    return view;
}

IUAVGadget *UAVGadgetManager::currentGadget() const
{
    return m_d->m_currentGadget;
}

void UAVGadgetManager::emptyView(Core::Internal::UAVGadgetView *view)
{
    if (!view)
        return;

    IUAVGadget *uavGadget = view->gadget();
//    emit uavGadgetAboutToClose(uavGadget);
    removeGadget(uavGadget);
    view->removeGadget();
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

    IUAVGadget *gadget = view->gadget();
    emptyView(view);
    UAVGadgetInstanceManager *im = ICore::instance()->uavGadgetInstanceManager();
    im->removeGadget(gadget);

    SplitterOrView *splitter = m_d->m_splitterOrView->findSplitter(splitterOrView);
    Q_ASSERT(splitterOrView->hasGadget() == false);
    Q_ASSERT(splitter->isSplitter() == true);
    splitterOrView->hide();
    delete splitterOrView;

    splitter->unsplit();

    SplitterOrView *newCurrent = splitter->findFirstView();
    Q_ASSERT(newCurrent);
    if (newCurrent)
        setCurrentGadget(newCurrent->gadget());
}

void UAVGadgetManager::addGadgetToContext(IUAVGadget *gadget)
{
    if (!gadget)
        return;
    m_d->m_core->addContextObject(gadget);

//   emit uavGadgetOpened(uavGadget);
}

void UAVGadgetManager::removeGadget(IUAVGadget *gadget)
{
    if (!gadget)
        return;
    m_d->m_core->removeContextObject(qobject_cast<IContext*>(gadget));
}

void UAVGadgetManager::ensureUAVGadgetManagerVisible()
{
    if (!isVisible())
        m_d->m_core->modeManager()->activateMode(m_uavGadgetMode->uniqueModeName());
}

void UAVGadgetManager::updateActions()
{
    if (m_d->m_core->modeManager()->currentMode() != m_uavGadgetMode)
        return;
    if (!m_d->m_splitterOrView) // this is only for startup
        return;
    // Splitting is only possible when the toolbars are shown
    bool shown = m_d->m_showToolbarsAction->isChecked();
    bool hasSplitter = m_d->m_splitterOrView->isSplitter();
    m_d->m_removeCurrentSplitAction->setEnabled(shown && hasSplitter);
    m_d->m_removeAllSplitsAction->setEnabled(shown && hasSplitter);
    m_d->m_gotoOtherSplitAction->setEnabled(shown && hasSplitter);
}

void UAVGadgetManager::saveState(QSettings* qSettings) const
{
    qSettings->setValue("version","UAVGadgetManagerV1");
    qSettings->setValue("showToolbars",m_showToolbars);
    qSettings->beginGroup("splitter");
    m_d->m_splitterOrView->saveState(qSettings);
    qSettings->endGroup();
}

bool UAVGadgetManager::restoreState(QSettings* qSettings)
{
    removeAllSplits();

    UAVGadgetInstanceManager *im = ICore::instance()->uavGadgetInstanceManager();
    IUAVGadget *gadget = m_d->m_splitterOrView->view()->gadget();
    emptyView(m_d->m_splitterOrView->view());
    im->removeGadget(gadget);

    QString version = qSettings->value("version").toString();
    if (version != "UAVGadgetManagerV1") {
        return false;
    }

    m_showToolbars = qSettings->value("showToolbars").toBool();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    qSettings->beginGroup("splitter");
    m_d->m_splitterOrView->restoreState(qSettings);
    qSettings->endGroup();

    QApplication::restoreOverrideCursor();
    return true;
}

bool UAVGadgetManager::restoreState(const QByteArray &state)
{
    removeAllSplits();

    UAVGadgetInstanceManager *im = ICore::instance()->uavGadgetInstanceManager();
    IUAVGadget *gadget = m_d->m_splitterOrView->view()->gadget();
    emptyView(m_d->m_splitterOrView->view());
    im->removeGadget(gadget);
    QDataStream stream(state);

    QByteArray version;
    stream >> version;

    if (version != "UAVGadgetManagerV1")
        return false;

    stream >> m_showToolbars;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QByteArray splitterstates;
    stream >> splitterstates;
    m_d->m_splitterOrView->restoreState(splitterstates);

    QApplication::restoreOverrideCursor();
    return true;
}

void UAVGadgetManager::saveSettings(QSettings *qs)
{
    qs->beginGroup("UAVGadgetManager");
    qs->beginGroup(m_uavGadgetMode->uniqueModeName());

    QString defaultUAVGadgetManagerKey =  "DefaultSettings";
    QString uavGadgetManagerKey = "Settings";

    // The default group can be done in a better way,
    // remove them for now. Since we no longer have two
    // possible groups, we can remove the non default one too.
    // TODO: Remove this code, and support for reading default group.
    if (qs->childKeys().contains(defaultUAVGadgetManagerKey) ||
        qs->childGroups().contains(defaultUAVGadgetManagerKey)) {
        qs->remove(defaultUAVGadgetManagerKey);
    }

    if (qs->allKeys().contains(uavGadgetManagerKey) ||
        qs->childGroups().contains(uavGadgetManagerKey)) {
        qs->remove(uavGadgetManagerKey);
    }

    // Do actual saving
    saveState(qs);

    qs->endGroup();
    qs->endGroup();
}

void UAVGadgetManager::readSettings(QSettings *qs)
{
    QString uavGadgetManagerRootKey = "UAVGadgetManager";
    if(!qs->childGroups().contains(uavGadgetManagerRootKey)) {
        return;
    }
    qs->beginGroup(uavGadgetManagerRootKey);

    if(!qs->childGroups().contains(m_uavGadgetMode->uniqueModeName())) {
        qs->endGroup();
        return;
    }
    qs->beginGroup(m_uavGadgetMode->uniqueModeName());

    QString defaultUAVGadgetManagerKey = "DefaultSettings";
    QString uavGadgetManagerKey = "Settings";

    if (qs->childGroups().contains(uavGadgetManagerKey)) {
        // TODO: Remove.
        qs->beginGroup(uavGadgetManagerKey);
        restoreState(qs);
        qs->endGroup();
    } else if (qs->childGroups().contains(defaultUAVGadgetManagerKey)) {
        // TODO: Remove.
        qs->beginGroup(defaultUAVGadgetManagerKey);
        restoreState(qs);
        qs->endGroup();
    } else if (qs->contains(uavGadgetManagerKey)) {
        // TODO: Remove.
        restoreState(QByteArray::fromBase64(qs->value(uavGadgetManagerKey).toByteArray()));
    } else if (qs->contains(defaultUAVGadgetManagerKey)) {
        // TODO: Remove.
        restoreState(QByteArray::fromBase64(qs->value(defaultUAVGadgetManagerKey).toByteArray()));
    } else {
        // TODO: Make this the only way of loading.
        restoreState(qs);
    }

    showToolbars(m_showToolbars);
    updateActions();

    qs->endGroup();
    qs->endGroup();
}

void UAVGadgetManager::split(Qt::Orientation orientation)
{
    if (m_d->m_core->modeManager()->currentMode() != m_uavGadgetMode)
        return;

    IUAVGadget *uavGadget = m_d->m_currentGadget;
    Q_ASSERT(uavGadget);
    SplitterOrView *view = currentSplitterOrView();
    Q_ASSERT(view);
    view->split(orientation);

    SplitterOrView *sor = m_d->m_splitterOrView->findView(uavGadget);
    SplitterOrView *next = m_d->m_splitterOrView->findNextView(sor);
    setCurrentGadget(next->gadget());
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
}

void UAVGadgetManager::removeAllSplits()
{
    if (m_d->m_core->modeManager()->currentMode() != m_uavGadgetMode)
        return;

    if (!m_d->m_splitterOrView->isSplitter())
        return;
    IUAVGadget *uavGadget = m_d->m_currentGadget;
    QList<IUAVGadget*> gadgets = m_d->m_splitterOrView->gadgets();
    gadgets.removeOne(uavGadget);

    m_d->m_currentGadget = 0; // trigger update below
    m_d->m_splitterOrView->unsplitAll();
    m_d->m_splitterOrView->view()->setGadget(uavGadget);
    setCurrentGadget(uavGadget);
    UAVGadgetInstanceManager *im = ICore::instance()->uavGadgetInstanceManager();
    foreach (IUAVGadget *g, gadgets) {
        im->removeGadget(g);
    }
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
            setCurrentGadget(view->gadget());
        }
    }
}

void UAVGadgetManager::showToolbars(bool show)
{
     if (m_d->m_core->modeManager()->currentMode() != m_uavGadgetMode)
         return;

     m_d->m_showToolbarsAction->setChecked(show);
     m_showToolbars = show;
     SplitterOrView *next = m_d->m_splitterOrView->findFirstView();
     do {
         next->view()->showToolbar(show);
         next = m_d->m_splitterOrView->findNextView(next);
     } while (next);

    m_d->m_splitAction->setEnabled(show);
    m_d->m_splitSideBySideAction->setEnabled(show);
    m_d->m_removeCurrentSplitAction->setEnabled(show);
    m_d->m_removeAllSplitsAction->setEnabled(show);
    m_d->m_gotoOtherSplitAction->setEnabled(show);
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
