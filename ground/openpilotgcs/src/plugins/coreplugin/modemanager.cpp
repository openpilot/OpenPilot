/**
 ******************************************************************************
 *
 * @file       modemanager.cpp
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

#include "modemanager.h"

#include "fancytabwidget.h"
#include "fancyactionbar.h"
#include "utils/mytabwidget.h"
#include "icore.h"
#include "mainwindow.h"

#include <aggregation/aggregate.h>

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/imode.h>
#include <coreplugin/uniqueidmanager.h>

#include <extensionsystem/pluginmanager.h>

#include <utils/qtcassert.h>

#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <QtCore/QSignalMapper>
#include <QtGui/QShortcut>

#include <QtGui/QAction>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>

using namespace Core;
using namespace Core::Internal;

ModeManager *ModeManager::m_instance = 0;

ModeManager::ModeManager(Internal::MainWindow *mainWindow, MyTabWidget *modeStack) :
    m_mainWindow(mainWindow),
    m_modeStack(modeStack),
    m_signalMapper(new QSignalMapper(this)),
    m_isReprioritizing(false)
{
    m_instance = this;

//    connect((m_modeStack), SIGNAL(currentAboutToShow(int)), SLOT(currentTabAboutToChange(int)));
    connect(m_modeStack, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
    connect(m_modeStack, SIGNAL(tabMoved(int,int)), this, SLOT(tabMoved(int,int)));
    connect(m_signalMapper, SIGNAL(mapped(QString)), this, SLOT(activateMode(QString)));
}

void ModeManager::init()
{
    QObject::connect(ExtensionSystem::PluginManager::instance(), SIGNAL(objectAdded(QObject*)),
                     this, SLOT(objectAdded(QObject*)));
    QObject::connect(ExtensionSystem::PluginManager::instance(), SIGNAL(aboutToRemoveObject(QObject*)),
                     this, SLOT(aboutToRemoveObject(QObject*)));
}

void ModeManager::addWidget(QWidget *widget)
{
    Q_UNUSED(widget);

    // We want the actionbar to stay on the bottom
    // so m_modeStack->cornerWidgetCount() -1 inserts it at the position immediately above
    // the actionbar
//    m_modeStack->insertCornerWidget(m_modeStack->cornerWidgetCount() -1, widget);
}

IMode *ModeManager::currentMode() const
{
    if (m_modes.count() > m_modeStack->currentIndex() )
        return m_modes.at(m_modeStack->currentIndex());
    else
        m_modeStack->setCurrentIndex(0); // Fix illegal Index.
    return 0;
}

int ModeManager::indexOf(const QString &id) const
{
    for (int i = 0; i < m_modes.count(); ++i) {
        if (m_modes.at(i)->uniqueModeName() == id)
            return i;
    }
//    qDebug() << "Warning, no such mode:" << id;
    return -1;
}

IMode *ModeManager::mode(const QString &id) const
{
    const int index = indexOf(id);
    if (index >= 0)
        return m_modes.at(index);
    return 0;
}

void ModeManager::activateMode(const QString &id)
{
    const int index = indexOf(id);
    if (index >= 0)
        m_modeStack->setCurrentIndex(index);
}

void ModeManager::activateModeByWorkspaceName(const QString &id)
{
    for (int i = 0; i < m_modes.count(); ++i) {
        if (m_modes.at(i)->name() == id)
        {
            m_modeStack->setCurrentIndex(i);
            return;
        }
    }
}

void ModeManager::objectAdded(QObject *obj)
{
    IMode *mode = Aggregation::query<IMode>(obj);
    if (!mode)
        return;

    m_mainWindow->addContextObject(mode);

    // Count the number of modes with a higher priority
    int index = 0;
    foreach (const IMode *m, m_modes)
        if (m->priority() > mode->priority())
            ++index;

    m_modes.insert(index, mode);
    m_modeStack->insertTab(index, mode->widget(), mode->icon(), mode->name());

    // Register mode shortcut
    ActionManager *am = m_mainWindow->actionManager();
    const QString shortcutId = QLatin1String("GCS.Mode.") + mode->uniqueModeName();
    QShortcut *shortcut = new QShortcut(m_mainWindow);
    shortcut->setWhatsThis(tr("Switch to %1 mode").arg(mode->name()));
    Command *cmd = am->registerShortcut(shortcut, shortcutId, QList<int>() << Constants::C_GLOBAL_ID);

    m_modeShortcuts.insert(index, cmd);
    connect(cmd, SIGNAL(keySequenceChanged()), this, SLOT(updateModeToolTip()));

    setDefaultKeyshortcuts();

    m_signalMapper->setMapping(shortcut, mode->uniqueModeName());
    connect(shortcut, SIGNAL(activated()), m_signalMapper, SLOT(map()));
}

void ModeManager::setDefaultKeyshortcuts() {
    for (int i = 0; i < m_modeShortcuts.size(); ++i) {
        Command *currentCmd = m_modeShortcuts.at(i);
        bool currentlyHasDefaultSequence = (currentCmd->keySequence()
                                            == currentCmd->defaultKeySequence());
#ifdef Q_WS_MAC
        currentCmd->setDefaultKeySequence(QKeySequence(QString("Meta+%1").arg(i+1)));
#else
        currentCmd->setDefaultKeySequence(QKeySequence(QString("Ctrl+%1").arg(i+1)));
#endif
        if (currentlyHasDefaultSequence)
            currentCmd->setKeySequence(currentCmd->defaultKeySequence());
    }
}

void ModeManager::updateModeToolTip()
{
    Command *cmd = qobject_cast<Command *>(sender());
    if (cmd) {
        int index = m_modeShortcuts.indexOf(cmd);
        if (index != -1)
            m_modeStack->setTabToolTip(index, cmd->stringWithAppendedShortcut(cmd->shortcut()->whatsThis()));
    }
}

void ModeManager::updateModeNameIcon(IMode *mode, const QIcon &icon, const QString &label)
{
    int index = indexOf(mode->uniqueModeName());
    if (index < 0)
        return;
    m_modeStack->setTabIcon(index, icon);
    m_modeStack->setTabText(index, label);
}

void ModeManager::aboutToRemoveObject(QObject *obj)
{
    IMode *mode = Aggregation::query<IMode>(obj);
    if (!mode)
        return;

    const int index = m_modes.indexOf(mode);
    m_modes.remove(index);
    m_modeShortcuts.remove(index);
    disconnect(m_modeStack, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
    m_modeStack->removeTab(index);
    connect(m_modeStack, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

    m_mainWindow->removeContextObject(mode);
}

void ModeManager::addAction(Command *command, int priority, QMenu *menu)
{
    Q_UNUSED(menu);

    m_actions.insert(command, priority);

    // Count the number of commands with a higher priority
    int index = 0;
    foreach (int p, m_actions.values())
        if (p > priority)
            ++index;

//    m_actionBar->insertAction(index, command->action(), menu);
}

void ModeManager::currentTabAboutToChange(int index)
{
    if (index >= 0) {
        IMode *mode = m_modes.at(index);
        if (mode)
            emit currentModeAboutToChange(mode);
    }
}

void ModeManager::currentTabChanged(int index)
{
//    qDebug() << "Current tab changed " << index;
    // Tab index changes to -1 when there is no tab left.
    if (index >= 0) {
        IMode *mode = m_modes.at(index);

        // FIXME: This hardcoded context update is required for the Debug and Edit modes, since
        // they use the editor widget, which is already a context widget so the main window won't
        // go further up the parent tree to find the mode context.
        ICore *core = ICore::instance();
        foreach (const int context, m_addedContexts)
            core->removeAdditionalContext(context);

        m_addedContexts = mode->context();
        foreach (const int context, m_addedContexts)
            core->addAdditionalContext(context);
        emit currentModeChanged(mode);
        core->updateContext();
    }
}

void ModeManager::tabMoved(int from, int to)
{
    IMode *mode = m_modes.at(from);
    m_modes.remove(from);
    m_modes.insert(to, mode);
    Command *cmd = m_modeShortcuts.at(from);
    m_modeShortcuts.remove(from);
    m_modeShortcuts.insert(to, cmd);
    setDefaultKeyshortcuts();
    // Reprioritize, high priority means show to the left
    if (!m_isReprioritizing) {
        for (int i = 0; i < m_modes.count(); ++i) {
            m_modes.at(i)->setPriority(100-i);
        }
        emit newModeOrder(m_modes);
    }    
}

void ModeManager::reorderModes(QMap<QString, int> priorities)
{
    foreach (IMode *mode, m_modes)
        mode->setPriority(priorities.value(QString(QLatin1String(mode->uniqueModeName())), mode->priority()));

    m_isReprioritizing = true;
    IMode *current = currentMode();
    // Bubble sort
    bool swapped = false;
    do {
        swapped = false;
        for (int i = 0; i < m_modes.count()-1; ++i) {
            IMode *mode1 = m_modes.at(i);
            IMode *mode2 = m_modes.at(i+1);
//            qDebug() << "Comparing " << i << " to " << i+1 << " p1 " << mode1->priority() << " p2 " << mode2->priority();
            if (mode2->priority() > mode1->priority()) {
                m_modeStack->moveTab(i, i+1);
//                qDebug() << "Tab moved from " << i << " to " << i+1;
                swapped = true;
            }
        }
    } while (swapped);
    m_isReprioritizing = false;
    m_modeStack->setCurrentIndex(0);
    activateMode(current->uniqueModeName());
    emit newModeOrder(m_modes);
}


void ModeManager::setFocusToCurrentMode()
{
    IMode *mode = currentMode();
    QTC_ASSERT(mode, return);
    QWidget *widget = mode->widget();
    if (widget) {
        QWidget *focusWidget = widget->focusWidget();
        if (focusWidget)
            focusWidget->setFocus();
        else
            widget->setFocus();
    }
}

void ModeManager::triggerAction(const QString &actionId)
{
    foreach(Command * command, m_actions.keys()){
        if(command->action()->objectName() == actionId) {
            command->action()->trigger();
            break;
        }
    }
}
