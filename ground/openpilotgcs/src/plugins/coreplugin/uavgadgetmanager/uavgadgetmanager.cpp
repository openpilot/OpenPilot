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

UAVGadgetManager::UAVGadgetManager(ICore *core, QString name, QIcon icon, int priority, QString uniqueName, QWidget *parent) :
    m_showToolbars(true),
    m_splitterOrView(0),
    m_currentGadget(0),
    m_core(core),
    m_name(name),
    m_icon(icon),
    m_priority(priority),
    m_widget(new QWidget(parent))
{

    // checking that the mode name is unique gives harmless
    // warnings on the console output
    ModeManager *modeManager = ModeManager::instance();
    if (!modeManager->mode(uniqueName)) {
        m_uniqueName = uniqueName;
    } else {
        // this shouldn't happen
        m_uniqueName = uniqueName + QString::number(quint64(this));
    }
    m_uniqueNameBA = m_uniqueName.toLatin1();
    m_uniqueModeName = m_uniqueNameBA.data();

    connect(m_core, SIGNAL(contextAboutToChange(Core::IContext *)),
            this, SLOT(handleContextChange(Core::IContext *)));

    connect(modeManager, SIGNAL(currentModeChanged(Core::IMode*)),
            this, SLOT(modeChanged(Core::IMode*)));

    // other setup
    m_splitterOrView = new SplitterOrView(this, 0);

    // SplitterOrView with 0 as gadget calls our setCurrentGadget, which relies on currentSplitterOrView(),
    // which needs our m_splitterorView to be set, which isn't set yet at that time.
    // So directly set our currentGadget to 0, and do it again.
    m_currentGadget = 0;
    setCurrentGadget(m_splitterOrView->view()->gadget());

    QHBoxLayout *layout = new QHBoxLayout(m_widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_splitterOrView);

    showToolbars(m_showToolbars);
}

UAVGadgetManager::~UAVGadgetManager()
{
}

QList<int> UAVGadgetManager::context() const
{
    static QList<int> contexts = QList<int>() <<
        UniqueIDManager::instance()->uniqueIdentifier(Constants::C_UAVGADGETMANAGER);
    return contexts;
}

void UAVGadgetManager::modeChanged(Core::IMode *mode)
{
    if (mode != this)
        return;

    m_currentGadget->widget()->setFocus();
    showToolbars(toolbarsShown());
}

void UAVGadgetManager::init()
{
    QList<int> context;
    context << m_core->uniqueIDManager()->uniqueIdentifier("OpenPilot.UAVGadgetManager");
}

void UAVGadgetManager::handleContextChange(Core::IContext *context)
{
//    if (debugUAVGadgetManager)
//        qDebug() << Q_FUNC_INFO << context;
    IUAVGadget *uavGadget = context ? qobject_cast<IUAVGadget*>(context) : 0;
    if (uavGadget)
        setCurrentGadget(uavGadget);
}

void UAVGadgetManager::setCurrentGadget(IUAVGadget *uavGadget)
{
    if (m_currentGadget == uavGadget)
        return;

    SplitterOrView *oldView = currentSplitterOrView();
    m_currentGadget = uavGadget;
    SplitterOrView *view = currentSplitterOrView();
    if (oldView != view) {
        if (oldView)
            oldView->update();
        if (view)
            view->update();
    }
    uavGadget->widget()->setFocus();
    emit currentGadgetChanged(uavGadget);
}

/* Contract: return current SplitterOrView.
 * Implications: must not return SplitterOrView that is splitter.
 */
Core::Internal::SplitterOrView *UAVGadgetManager::currentSplitterOrView() const
{
    if (!m_splitterOrView) // this is only for startup
        return 0;
    SplitterOrView *view = m_currentGadget ?
                           m_splitterOrView->findView(m_currentGadget) :
                           0;
    return view;
}

IUAVGadget *UAVGadgetManager::currentGadget() const
{
    return m_currentGadget;
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
    SplitterOrView *splitterOrView = m_splitterOrView->findView(view);
    Q_ASSERT(splitterOrView);
    Q_ASSERT(splitterOrView->view() == view);
    if (splitterOrView == m_splitterOrView)
        return;

    IUAVGadget *gadget = view->gadget();
    emptyView(view);
    UAVGadgetInstanceManager *im = ICore::instance()->uavGadgetInstanceManager();
    im->removeGadget(gadget);

    SplitterOrView *splitter = m_splitterOrView->findSplitter(splitterOrView);
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
    m_core->addContextObject(gadget);

//   emit uavGadgetOpened(uavGadget);
}

void UAVGadgetManager::removeGadget(IUAVGadget *gadget)
{
    if (!gadget)
        return;
    m_core->removeContextObject(qobject_cast<IContext*>(gadget));
}

void UAVGadgetManager::ensureUAVGadgetManagerVisible()
{
    if (!m_widget->isVisible())
        m_core->modeManager()->activateMode(this->uniqueModeName());
}

void UAVGadgetManager::showToolbars(bool show)
{
    if (m_core->modeManager()->currentMode() != this)
        return;

    m_showToolbars = show;
    SplitterOrView *next = m_splitterOrView->findFirstView();
    do {
        next->view()->showToolbar(show);
        next = m_splitterOrView->findNextView(next);
    } while (next);

    updateUavGadgetMenus();
}

void UAVGadgetManager::updateUavGadgetMenus()
{
    if (m_core->modeManager()->currentMode() != this)
        return;
    if (!m_splitterOrView) // this is only for startup
        return;
    // Splitting is only possible when the toolbars are shown
    bool hasSplitter = m_splitterOrView->isSplitter();
    emit showUavGadgetMenus(m_showToolbars, hasSplitter);
}

void UAVGadgetManager::saveState(QSettings* qSettings) const
{
    qSettings->setValue("version","UAVGadgetManagerV1");
    qSettings->setValue("showToolbars",m_showToolbars);
    qSettings->beginGroup("splitter");
    m_splitterOrView->saveState(qSettings);
    qSettings->endGroup();
}

bool UAVGadgetManager::restoreState(QSettings* qSettings)
{
    removeAllSplits();

    UAVGadgetInstanceManager *im = ICore::instance()->uavGadgetInstanceManager();
    IUAVGadget *gadget = m_splitterOrView->view()->gadget();
    emptyView(m_splitterOrView->view());
    im->removeGadget(gadget);

    QString version = qSettings->value("version").toString();
    if (version != "UAVGadgetManagerV1") {
        return false;
    }

    m_showToolbars = qSettings->value("showToolbars").toBool();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    qSettings->beginGroup("splitter");
    m_splitterOrView->restoreState(qSettings);
    qSettings->endGroup();

    QApplication::restoreOverrideCursor();
    return true;
}

void UAVGadgetManager::saveSettings(QSettings *qs)
{
    qs->beginGroup("UAVGadgetManager");
    qs->beginGroup(this->uniqueModeName());

    // Make sure the old tree is wiped.
    qs->remove("");

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

    if(!qs->childGroups().contains(uniqueModeName())) {
        qs->endGroup();
        return;
    }
    qs->beginGroup(uniqueModeName());

    restoreState(qs);

    showToolbars(m_showToolbars);

    qs->endGroup();
    qs->endGroup();
}

void UAVGadgetManager::split(Qt::Orientation orientation)
{
    if (m_core->modeManager()->currentMode() != this)
        return;

    IUAVGadget *uavGadget = m_currentGadget;
    Q_ASSERT(uavGadget);
    SplitterOrView *view = currentSplitterOrView();
    Q_ASSERT(view);
    view->split(orientation);

    SplitterOrView *sor = m_splitterOrView->findView(uavGadget);
    SplitterOrView *next = m_splitterOrView->findNextView(sor);
    setCurrentGadget(next->gadget());
    updateUavGadgetMenus();
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
    if (m_core->modeManager()->currentMode() != this)
        return;

    SplitterOrView *viewToClose = currentSplitterOrView();
    if (viewToClose == m_splitterOrView)
        return;
    closeView(viewToClose->view());
    updateUavGadgetMenus();
}

// Removes all gadgets and splits in the workspace, except the current/active gadget.
void UAVGadgetManager::removeAllSplits()
{
    if (m_core->modeManager()->currentMode() != this)
        return;

    if (!m_splitterOrView->isSplitter())
        return;

    // Use a QPointer, just in case we accidently delete the gadget we want to keep.
    QPointer<IUAVGadget> currentGadget = m_currentGadget;

    Q_ASSERT(currentGadget);
    QList<IUAVGadget*> gadgets = m_splitterOrView->gadgets();
    Q_ASSERT(gadgets.count(currentGadget) == 1);
    gadgets.removeOne(currentGadget);

    // Remove all splits and their gadgets, then create a new view with the current gadget.
    m_splitterOrView->unsplitAll(currentGadget);

    // Zeroing the current gadget means setCurrentGadget will do something when we call it.
    m_currentGadget = 0;
    setCurrentGadget(currentGadget);

    // Remove all other gadgets from the instance manager.
    UAVGadgetInstanceManager *im = ICore::instance()->uavGadgetInstanceManager();
    foreach (IUAVGadget *g, gadgets) {
        im->removeGadget(g);
    }
    updateUavGadgetMenus();
}

void UAVGadgetManager::gotoOtherSplit()
{
    if (m_core->modeManager()->currentMode() != this)
        return;

    if (m_splitterOrView->isSplitter()) {
        SplitterOrView *currentView = currentSplitterOrView();
        SplitterOrView *view = m_splitterOrView->findNextView(currentView);
        if (!view)
            view = m_splitterOrView->findFirstView();
        if (view) {
            setCurrentGadget(view->gadget());
        }
    }
}
