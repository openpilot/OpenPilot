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

UAVGadgetManagerPlaceHolder *UAVGadgetManagerPlaceHolder::m_current = 0;

UAVGadgetManagerPlaceHolder::UAVGadgetManagerPlaceHolder(Core::IMode *mode, QWidget *parent)
    : QWidget(parent), m_mode(mode)
{
    setLayout(new QVBoxLayout);
    layout()->setMargin(0);
    connect(Core::ModeManager::instance(), SIGNAL(currentModeChanged(Core::IMode *)),
            this, SLOT(currentModeChanged(Core::IMode *)));

    //Julien: this line is crashing (I guess when no mode are added yet)
    //currentModeChanged(Core::ModeManager::instance()->currentMode());
}

UAVGadgetManagerPlaceHolder::~UAVGadgetManagerPlaceHolder()
{
    if (m_current == this) {
        UAVGadgetManager::instance()->setParent(0);
        UAVGadgetManager::instance()->hide();
    }
}

void UAVGadgetManagerPlaceHolder::currentModeChanged(Core::IMode *mode)
{
    if (m_current == this) {
        m_current = 0;
        UAVGadgetManager::instance()->setParent(0);
        UAVGadgetManager::instance()->hide();
    }
    if (m_mode == mode) {
        m_current = this;
        layout()->addWidget(UAVGadgetManager::instance());
        UAVGadgetManager::instance()->show();
    }
}

UAVGadgetManagerPlaceHolder* UAVGadgetManagerPlaceHolder::current()
{
    return m_current;
}

// ---------------- UAVGadgetManager

namespace Core {


struct UAVGadgetManagerPrivate {
    explicit UAVGadgetManagerPrivate(ICore *core, QWidget *parent);
    ~UAVGadgetManagerPrivate();
    Internal::UAVGadgetView *m_view;
    Internal::SplitterOrView *m_splitterOrView;
    QPointer<IUAVGadget> m_currentUAVGadget;
    QPointer<SplitterOrView> m_currentView;

    ICore *m_core;

    // actions
    QAction *m_hideToolbarsAction;
    QAction *m_splitAction;
    QAction *m_splitSideBySideAction;
    QAction *m_removeCurrentSplitAction;
    QAction *m_removeAllSplitsAction;
    QAction *m_gotoOtherSplitAction;
    QAction *m_closeCurrentUAVGadgetAction;

    Internal::UAVGadgetClosingCoreListener *m_coreListener;
};
}

UAVGadgetManagerPrivate::UAVGadgetManagerPrivate(ICore *core, QWidget *parent) :
    m_view(0),
    m_splitterOrView(0),
    m_core(core),
    m_closeCurrentUAVGadgetAction(new QAction(UAVGadgetManager::tr("Close"), parent)),
    m_coreListener(0)
{
}

UAVGadgetManagerPrivate::~UAVGadgetManagerPrivate()
{
}

UAVGadgetManager *UAVGadgetManager::m_instance = 0;

/*static Command *createSeparator(ActionManager *am, QObject *parent,
                                const QString &name,
                                const QList<int> &context)
{
    QAction *tmpaction = new QAction(parent);
    tmpaction->setSeparator(true);
    Command *cmd = am->registerAction(tmpaction, name, context);
    return cmd;
}*/

UAVGadgetManager::UAVGadgetManager(ICore *core, QWidget *parent) :
    QWidget(parent),
    m_d(new UAVGadgetManagerPrivate(core, parent))
{
  //qDebug() << Q_FUNC_INFO;
    m_instance = this;

//  //qDebug() << Q_FUNC_INFO << m_d->m_core->resourcePath();
    connect(m_d->m_core, SIGNAL(contextAboutToChange(Core::IContext *)),
            this, SLOT(handleContextChange(Core::IContext *)));
    const QList<int> gc =  QList<int>() << Constants::C_GLOBAL_ID;
    const QList<int> uavGadgetManagerContext =
            QList<int>() << m_d->m_core->uniqueIDManager()->uniqueIdentifier(Constants::C_UAVGADGETMANAGER);

    ActionManager *am = m_d->m_core->actionManager();

    //Window Menu
    ActionContainer *mwindow = am->actionContainer(Constants::M_WINDOW);

    //Window menu separators
    QAction *tmpaction = new QAction(this);
    tmpaction->setSeparator(true);
    Command *cmd = am->registerAction(tmpaction, QLatin1String("OpenPilot.Window.Sep.Split"), uavGadgetManagerContext);
    mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);

    m_d->m_hideToolbarsAction = new QAction(tr("Hide toolbars"), this);
    m_d->m_hideToolbarsAction->setCheckable(true);
    cmd = am->registerAction(m_d->m_hideToolbarsAction, Constants::HIDE_TOOLBARS, uavGadgetManagerContext);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+Shift+F10"));
    mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    connect(m_d->m_hideToolbarsAction, SIGNAL(triggered(bool)), this, SLOT(hideToolbars(bool)));

//    //Window menu separators
    QAction *tmpaction2 = new QAction(this);
    tmpaction2->setSeparator(true);
    cmd = am->registerAction(tmpaction2, QLatin1String("OpenPilot.Window.Sep.Split2"), uavGadgetManagerContext);
    mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);

    //Close Action
//  //qDebug() << Q_FUNC_INFO << "close cmd" << m_d->m_closeCurrentUAVGadgetAction;
    cmd = am->registerAction(m_d->m_closeCurrentUAVGadgetAction, Constants::CLOSE, uavGadgetManagerContext);
//    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+W")));
    cmd->setAttribute(Core::Command::CA_UpdateText);
    cmd->setDefaultText(m_d->m_closeCurrentUAVGadgetAction->text());
    connect(m_d->m_closeCurrentUAVGadgetAction, SIGNAL(triggered()), this, SLOT(closeUAVGadget()));

#ifdef Q_WS_MAC
    QString prefix = tr("Meta+E");
#else
    QString prefix = tr("Ctrl+E");
#endif


    m_d->m_splitAction = new QAction(tr("Split"), this);
    cmd = am->registerAction(m_d->m_splitAction, Constants::SPLIT, uavGadgetManagerContext);
    cmd->setDefaultKeySequence(QKeySequence(tr("%1,2").arg(prefix)));
    mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    connect(m_d->m_splitAction, SIGNAL(triggered()), this, SLOT(split()));

    m_d->m_splitSideBySideAction = new QAction(tr("Split Side by Side"), this);
    cmd = am->registerAction(m_d->m_splitSideBySideAction, Constants::SPLIT_SIDE_BY_SIDE, uavGadgetManagerContext);
    cmd->setDefaultKeySequence(QKeySequence(tr("%1,3").arg(prefix)));
    mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    connect(m_d->m_splitSideBySideAction, SIGNAL(triggered()), this, SLOT(splitSideBySide()));

    m_d->m_removeCurrentSplitAction = new QAction(tr("Remove Current Split"), this);
    cmd = am->registerAction(m_d->m_removeCurrentSplitAction, Constants::REMOVE_CURRENT_SPLIT, uavGadgetManagerContext);
    cmd->setDefaultKeySequence(QKeySequence(tr("%1,0").arg(prefix)));
    mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    connect(m_d->m_removeCurrentSplitAction, SIGNAL(triggered()), this, SLOT(removeCurrentSplit()));

    m_d->m_removeAllSplitsAction = new QAction(tr("Remove All Splits"), this);
    cmd = am->registerAction(m_d->m_removeAllSplitsAction, Constants::REMOVE_ALL_SPLITS, uavGadgetManagerContext);
    cmd->setDefaultKeySequence(QKeySequence(tr("%1,1").arg(prefix)));
    mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    connect(m_d->m_removeAllSplitsAction, SIGNAL(triggered()), this, SLOT(removeAllSplits()));

    m_d->m_gotoOtherSplitAction = new QAction(tr("Goto Other Split"), this);
    cmd = am->registerAction(m_d->m_gotoOtherSplitAction, Constants::GOTO_OTHER_SPLIT, uavGadgetManagerContext);
    cmd->setDefaultKeySequence(QKeySequence(tr("%1,o").arg(prefix)));
    mwindow->addAction(cmd, Constants::G_WINDOW_SPLIT);
    connect(m_d->m_gotoOtherSplitAction, SIGNAL(triggered()), this, SLOT(gotoOtherSplit()));


    // other setup
    m_d->m_splitterOrView = new SplitterOrView(0, true);
    m_d->m_view = m_d->m_splitterOrView->view();


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
  //qDebug() << Q_FUNC_INFO;
    QList<int> context;
    context << m_d->m_core->uniqueIDManager()->uniqueIdentifier("OpenPilot.UAVGadgetManager");

    m_d->m_coreListener = new UAVGadgetClosingCoreListener(this);

    pluginManager()->addObject(m_d->m_coreListener);
}

void UAVGadgetManager::removeUAVGadget(IUAVGadget *uavGadget)
{
  //qDebug() << Q_FUNC_INFO << uavGadget;
    if (!uavGadget)
        return;
  //qDebug() << Q_FUNC_INFO << uavGadget;
    m_d->m_core->removeContextObject(qobject_cast<IContext*>(uavGadget));
}

void UAVGadgetManager::handleContextChange(Core::IContext *context)
{
  //qDebug() << Q_FUNC_INFO;
    if (debugUAVGadgetManager)
      qDebug() << Q_FUNC_INFO << context;
    IUAVGadget *uavGadget = context ? qobject_cast<IUAVGadget*>(context) : 0;
    if (uavGadget) {
        setCurrentUAVGadget(uavGadget);
    } else {
        updateActions();
    }
}

void UAVGadgetManager::setCurrentUAVGadget(IUAVGadget *uavGadget)
{
  //qDebug() << Q_FUNC_INFO;
    if (uavGadget)
        setCurrentView(0);

    if (m_d->m_currentUAVGadget == uavGadget)
        return;

    m_d->m_currentUAVGadget = uavGadget;
    if (uavGadget) {
        if (SplitterOrView *splitterOrView = m_d->m_splitterOrView->findView(uavGadget))
            splitterOrView->view()->setCurrentUAVGadget(uavGadget);
    }
    updateActions();
//    emit currentUAVGadgetChanged(uavGadget);
}


void UAVGadgetManager::setCurrentView(Core::Internal::SplitterOrView *view)
{
  //qDebug() << Q_FUNC_INFO;
    if (view == m_d->m_currentView)
        return;

    SplitterOrView *old = m_d->m_currentView;
    m_d->m_currentView = view;

    if (old)
        old->update();
    if (view)
        view->update();

    if (view && !view->uavGadget())
        view->setFocus();
}

Core::Internal::SplitterOrView *UAVGadgetManager::currentSplitterOrView() const
{
    SplitterOrView *view = m_d->m_currentView;
    if (!view)
        view = m_d->m_currentUAVGadget?
               m_d->m_splitterOrView->findView(m_d->m_currentUAVGadget):
               m_d->m_splitterOrView->findFirstView();
    if (!view)
        return m_d->m_splitterOrView;
    return view;
}

Core::Internal::UAVGadgetView *UAVGadgetManager::currentUAVGadgetView() const
{
  //qDebug() << Q_FUNC_INFO;
    return currentSplitterOrView()->view();
}

IUAVGadget *UAVGadgetManager::currentUAVGadget() const
{
  //qDebug() << Q_FUNC_INFO;
    return m_d->m_currentUAVGadget;
}

void UAVGadgetManager::emptyView(Core::Internal::UAVGadgetView *view)
{
  //qDebug() << Q_FUNC_INFO << view;
    if (!view)
        return;

    IUAVGadget *uavGadget = view->currentUAVGadget();
  //qDebug() << Q_FUNC_INFO << view << uavGadget;
//    emit uavGadgetAboutToClose(uavGadget);
    removeUAVGadget(uavGadget);
  //qDebug() << Q_FUNC_INFO << view << uavGadget;
    view->removeUAVGadget();
  //qDebug() << Q_FUNC_INFO << view << uavGadget;

//    emit uavGadgetsClosed(uavGadgets);
    if (uavGadget)
        delete uavGadget; // FIXME correct?
}

void UAVGadgetManager::closeView(Core::Internal::UAVGadgetView *view)
{
  //qDebug() << Q_FUNC_INFO;
    if (!view)
        return;

    if (view == m_d->m_view) {
        if (IUAVGadget *e = view->currentUAVGadget())
            closeUAVGadgets(QList<IUAVGadget *>() << e);
        return;
    }

    emptyView(view);

    SplitterOrView *splitterOrView = m_d->m_splitterOrView->findView(view);
    Q_ASSERT(splitterOrView);
    Q_ASSERT(splitterOrView->view() == view);
    SplitterOrView *splitter = m_d->m_splitterOrView->findSplitter(splitterOrView);
    Q_ASSERT(splitterOrView->hasUAVGadgets() == false);
    splitterOrView->hide();
    delete splitterOrView;

    splitter->unsplit();

    SplitterOrView *newCurrent = splitter->findFirstView();
    if (newCurrent) {
        if (newCurrent->uavGadget())
            activateUAVGadget(newCurrent->view(), newCurrent->uavGadget());
        else
            setCurrentView(newCurrent);
    }
}

bool UAVGadgetManager::closeAllUAVGadgets()
{
//    if (closeUAVGadgets(openedUAVGadgets())) {
//        m_d->clearNavigationHistory();
        return true;
//    }
    return true;
}

// SLOT connected to action
// since this is potentially called in the event handler of the uavGadget
// we simply postpone it with a single shot timer
void UAVGadgetManager::closeUAVGadget()
{
  //qDebug() << Q_FUNC_INFO;
    closeUAVGadget(m_d->m_currentUAVGadget);
}

void UAVGadgetManager::closeUAVGadget(Core::IUAVGadget *uavGadget)
{
  //qDebug() << Q_FUNC_INFO;
    if (!uavGadget)
        return;
    closeUAVGadgets(QList<IUAVGadget *>() << uavGadget);
}

bool UAVGadgetManager::closeUAVGadgets(const QList<IUAVGadget*> uavGadgetsToClose)
{
  //qDebug() << Q_FUNC_INFO;
    if (uavGadgetsToClose.isEmpty())
        return true;

    SplitterOrView *currentSplitterOrView = this->currentSplitterOrView();

    QList<UAVGadgetView*> closedViews;

    // remove the uavGadgets
    foreach (IUAVGadget *uavGadget, uavGadgetsToClose) {
//        emit uavGadgetAboutToClose(uavGadget);

        removeUAVGadget(uavGadget);
        if (SplitterOrView *view = m_d->m_splitterOrView->findView(uavGadget)) {
            if (uavGadget == view->view()->currentUAVGadget())
                closedViews += view->view();
            view->view()->removeUAVGadget();
        }
    }

//    emit uavGadgetsClosed(acceptedUAVGadgets);

    foreach (IUAVGadget *uavGadget, uavGadgetsToClose) {
        delete uavGadget;
    }

    if (currentSplitterOrView) {
        if (IUAVGadget *uavGadget = currentSplitterOrView->uavGadget())
            activateUAVGadget(currentSplitterOrView->view(), uavGadget);
    }

    if (!currentUAVGadget()) {}
//        emit currentUAVGadgetChanged(0);

    return true;
}

Core::IUAVGadget *UAVGadgetManager::pickUnusedUAVGadget() const
{
  //qDebug() << Q_FUNC_INFO;
    //TODO
//    foreach (IUAVGadget *uavGadget, openedUAVGadgets()) {
//        SplitterOrView *view = m_d->m_splitterOrView->findView(uavGadget);
//        if (!view || view->uavGadget() != uavGadget)
//            return uavGadget;
//    }
    return 0;
}

Core::IUAVGadget *UAVGadgetManager::placeUAVGadget(Core::Internal::UAVGadgetView *view, Core::IUAVGadget *uavGadget)
{
  //qDebug() << Q_FUNC_INFO;
    Q_ASSERT(view && uavGadget);

    if (view->currentUAVGadget())
        uavGadget = view->currentUAVGadget();

    if (!view->hasUAVGadget(uavGadget)) {
        if (SplitterOrView *sourceView = m_d->m_splitterOrView->findView(uavGadget)) {
            sourceView->view()->removeUAVGadget();
            view->setCurrentUAVGadget(uavGadget);
            if (!sourceView->uavGadget()) {
                if (IUAVGadget *replacement = pickUnusedUAVGadget()) {
                    sourceView->view()->setCurrentUAVGadget(replacement);
                }
            }
            return uavGadget;
        }
        view->setCurrentUAVGadget(uavGadget);
    }
    return uavGadget;
}

Core::IUAVGadget *UAVGadgetManager::activateUAVGadget(Core::IUAVGadget *uavGadget)
{
  //qDebug() << Q_FUNC_INFO;
    return activateUAVGadget(0, uavGadget);
}

Core::IUAVGadget *UAVGadgetManager::activateUAVGadget(Core::Internal::UAVGadgetView *view, Core::IUAVGadget *uavGadget)
{
  //qDebug() << Q_FUNC_INFO;
    if (!view)
        view = currentUAVGadgetView();

    Q_ASSERT(view);

    if (!uavGadget) {
        if (!m_d->m_currentUAVGadget)
            setCurrentUAVGadget(0);
        return 0;
    }

    uavGadget = placeUAVGadget(view, uavGadget);

/*    if (!(flags & NoActivate)) {
        setCurrentUAVGadget(uavGadget);
        if (!(flags & NoModeSwitch))
            ensureUAVGadgetManagerVisible();
        if (isVisible())
            uavGadget->widget()->setFocus();
    }*/
    return uavGadget;
}


UAVGadgetManager::UAVGadgetFactoryList
    UAVGadgetManager::uavGadgetFactories() const
{
  //qDebug() << Q_FUNC_INFO;
    UAVGadgetFactoryList rc = pluginManager()->getObjects<IUAVGadgetFactory>();
    //if (debugUAVGadgetManager)
      //qDebug() << Q_FUNC_INFO << rc;
    return rc;
}


IUAVGadget *UAVGadgetManager::createUAVGadget(const QString &uavGadgetKind)
{
  //qDebug() << Q_FUNC_INFO;
    if (debugUAVGadgetManager)
      qDebug() << Q_FUNC_INFO << uavGadgetKind;


    UAVGadgetFactoryList factories = uavGadgetFactories();

    IUAVGadgetFactory *factory = 0;
    foreach(IUAVGadgetFactory *f, factories) {
        if (f->name() == uavGadgetKind) {
            factory = f;
            break;
        }
    }
    if (!factory) {
        qWarning("%s: unable to find an uavGadget factory for the uavGadget kind '%s'.",
                 Q_FUNC_INFO, uavGadgetKind.toUtf8().constData());
        return 0;
    }
    IUAVGadget *uavGadget = factory->createUAVGadget(this);
//    if (uavGadget)
//        connect(uavGadget, SIGNAL(changed()), this, SLOT(updateActions()));
//    if (uavGadget)
//        emit uavGadgetCreated(uavGadget, fileName);
    return uavGadget;
}

void UAVGadgetManager::addUAVGadget(IUAVGadget *uavGadget)
{
  //qDebug() << Q_FUNC_INFO;
    if (!uavGadget)
        return;
    m_d->m_core->addContextObject(uavGadget);

//   emit uavGadgetOpened(uavGadget);
}

void UAVGadgetManager::ensureUAVGadgetManagerVisible()
{
  //qDebug() << Q_FUNC_INFO;
    if (!isVisible())
        m_d->m_core->modeManager()->activateMode(Constants::MODE_UAVGADGET);
}

void UAVGadgetManager::updateActions()
{
  //qDebug() << Q_FUNC_INFO;
    // Splitting is only possible when the toolbars are shown
    bool hidden = m_d->m_hideToolbarsAction->isChecked();
    bool hasSplitter = m_d->m_splitterOrView->isSplitter();
    m_d->m_removeCurrentSplitAction->setEnabled(!hidden && hasSplitter);
    m_d->m_removeAllSplitsAction->setEnabled(!hidden && hasSplitter);
    m_d->m_gotoOtherSplitAction->setEnabled(!hidden && hasSplitter);
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
  //qDebug() << Q_FUNC_INFO;
    SplitterOrView *view = m_d->m_currentView;
//  //qDebug() << Q_FUNC_INFO << view << m_d->m_currentUAVGadget;
    if (!view)
            view = m_d->m_currentUAVGadget ? m_d->m_splitterOrView->findView(m_d->m_currentUAVGadget)
                       : m_d->m_splitterOrView->findFirstView();
  //qDebug() << Q_FUNC_INFO << view << m_d->m_currentUAVGadget << view->splitter();
    if (view && !view->splitter()) {
        view->split(orientation);
    }
    updateActions();
}

void UAVGadgetManager::split()
{
  //qDebug() << Q_FUNC_INFO;
    split(Qt::Vertical);
}

void UAVGadgetManager::splitSideBySide()
{
  //qDebug() << Q_FUNC_INFO;
    split(Qt::Horizontal);
}

void UAVGadgetManager::removeCurrentSplit()
{
  //qDebug() << Q_FUNC_INFO;
    SplitterOrView *viewToClose = m_d->m_currentView;
    if (!viewToClose && m_d->m_currentUAVGadget)
        viewToClose = m_d->m_splitterOrView->findView(m_d->m_currentUAVGadget);

    if (!viewToClose || viewToClose->isSplitter() || viewToClose == m_d->m_splitterOrView)
        return;

    closeView(viewToClose->view());
    updateActions();
}

void UAVGadgetManager::removeAllSplits()
{
    if (!m_d->m_splitterOrView->isSplitter())
        return;
    IUAVGadget *uavGadget = m_d->m_currentUAVGadget;
    m_d->m_currentUAVGadget = 0; // trigger update below
    m_d->m_splitterOrView->unsplitAll();
    if (!uavGadget)
        uavGadget = pickUnusedUAVGadget();
    activateUAVGadget(uavGadget);
}

void UAVGadgetManager::gotoOtherSplit()
{
    if (m_d->m_splitterOrView->isSplitter()) {
        SplitterOrView *currentView = m_d->m_currentView;
        if (!currentView && m_d->m_currentUAVGadget)
            currentView = m_d->m_splitterOrView->findView(m_d->m_currentUAVGadget);
        if (!currentView)
            currentView = m_d->m_splitterOrView->findFirstView();
        SplitterOrView *view = m_d->m_splitterOrView->findNextView(currentView);
        if (!view)
            view = m_d->m_splitterOrView->findFirstView();
        if (view) {
            if (IUAVGadget *uavGadget = view->uavGadget()) {
                setCurrentUAVGadget(uavGadget);
                uavGadget->widget()->setFocus();
            } else {
                setCurrentView(view);
            }
        }
    }
}

void UAVGadgetManager::hideToolbars(bool hide)
{
  //qDebug() << Q_FUNC_INFO;
    QList<SplitterOrView*> views;
    SplitterOrView *next = m_d->m_splitterOrView->findFirstView();
  //qDebug() << Q_FUNC_INFO << next->m_isRoot;
    do {
        views.append(next);
        next = m_d->m_splitterOrView->findNextView(next);
    } while(next);
    foreach(SplitterOrView *s, views)
    {
//      //qDebug() << Q_FUNC_INFO << s;
        s->view()->hideToolbar(hide);
    }
    m_d->m_splitAction->setEnabled(!hide);
    m_d->m_splitSideBySideAction->setEnabled(!hide);
    m_d->m_removeCurrentSplitAction->setEnabled(!hide);
    m_d->m_removeAllSplitsAction->setEnabled(!hide);
    m_d->m_gotoOtherSplitAction->setEnabled(!hide);
//    m_d->m_splitAction->setVisible(!hide);
//    m_d->m_splitSideBySideAction->setVisible(!hide);
//    m_d->m_removeCurrentSplitAction->setVisible(!hide);
//    m_d->m_removeAllSplitsAction->setVisible(!hide);
//    m_d->m_gotoOtherSplitAction->setVisible(!hide);

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
    return m_em->closeAllUAVGadgets();
}
