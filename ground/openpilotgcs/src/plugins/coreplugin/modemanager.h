/**
 ******************************************************************************
 *
 * @file       modemanager.h
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

#ifndef MODEMANAGER_H
#define MODEMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QVector>

#include <coreplugin/core_global.h>

QT_BEGIN_NAMESPACE
class QSignalMapper;
class QMenu;
class QIcon;
class MyTabWidget;
QT_END_NAMESPACE

namespace Core {

class Command;
class IMode;

namespace Internal {
class FancyTabWidget;
class FancyActionBar;
class MainWindow;
} // namespace Internal

class CORE_EXPORT ModeManager : public QObject
{
    Q_OBJECT

public:
    ModeManager(Internal::MainWindow *mainWindow, MyTabWidget *modeStack);

    void init();
    static ModeManager *instance() { return m_instance; }

    IMode* currentMode() const;
    IMode* mode(const QString &id) const;

    void addAction(Command *command, int priority, QMenu *menu = 0);
    void addWidget(QWidget *widget);
    void updateModeNameIcon(IMode *mode, const QIcon &icon, const QString &label);
    QVector<IMode*> modes() const { return m_modes; }
    void reorderModes(QMap<QString, int> priorities);

signals:
    void currentModeAboutToChange(Core::IMode *mode);
    void currentModeChanged(Core::IMode *mode);
    void newModeOrder(QVector<IMode*> modes);

public slots:
    void activateMode(const QString &id);
    void activateModeByWorkspaceName(const QString &id);
    void setFocusToCurrentMode();
    void triggerAction(const QString &actionId);

private slots:
    void objectAdded(QObject *obj);
    void aboutToRemoveObject(QObject *obj);
    void currentTabAboutToChange(int index);
    void currentTabChanged(int index);
    void updateModeToolTip();
    void tabMoved(int from, int to);

private:
    int indexOf(const QString &id) const;
    void setDefaultKeyshortcuts();

    static ModeManager *m_instance;
    Internal::MainWindow *m_mainWindow;
    MyTabWidget *m_modeStack;
    QMap<Command*, int> m_actions;
    QVector<IMode*> m_modes;
    QVector<Command*> m_modeShortcuts;
    QSignalMapper *m_signalMapper;
    QList<int> m_addedContexts;
    QList<int> m_tabOrder;
    bool m_isReprioritizing;
};

} // namespace Core

#endif // MODEMANAGER_H
