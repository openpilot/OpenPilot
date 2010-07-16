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
    ModeManager(Internal::MainWindow *mainWindow, Internal::FancyTabWidget *modeStack);

    void init();
    static ModeManager *instance() { return m_instance; }

    IMode* currentMode() const;
    IMode* mode(const QString &id) const;

    void addAction(Command *command, int priority, QMenu *menu = 0);
    void addWidget(QWidget *widget);
    void updateModeNameIcon(IMode *mode, const QIcon &icon, const QString &label);

signals:
    void currentModeAboutToChange(Core::IMode *mode);
    void currentModeChanged(Core::IMode *mode);

public slots:
    void activateMode(const QString &id);
    void setFocusToCurrentMode();

private slots:
    void objectAdded(QObject *obj);
    void aboutToRemoveObject(QObject *obj);
    void currentTabAboutToChange(int index);
    void currentTabChanged(int index);
    void updateModeToolTip();

private:
    int indexOf(const QString &id) const;

    static ModeManager *m_instance;
    Internal::MainWindow *m_mainWindow;
    Internal::FancyTabWidget *m_modeStack;
    Internal::FancyActionBar *m_actionBar;
    QMap<Command*, int> m_actions;
    QVector<IMode*> m_modes;
    QVector<Command*> m_modeShortcuts;
    QSignalMapper *m_signalMapper;
    QList<int> m_addedContexts;
};

} // namespace Core

#endif // MODEMANAGER_H
