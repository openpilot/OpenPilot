/**
 ******************************************************************************
 *
 * @file       actionmanager_p.h
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

#ifndef ACTIONMANAGERPRIVATE_H
#define ACTIONMANAGERPRIVATE_H

#include <coreplugin/actionmanager/actionmanager.h>

#include <QtCore/QMap>
#include <QtCore/QHash>
#include <QtCore/QMultiHash>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

struct CommandLocation
{
    int m_container;
    int m_position;
};

namespace Core {

class UniqueIDManager;

namespace Internal {

class ActionContainerPrivate;
class MainWindow;
class CommandPrivate;

class ActionManagerPrivate : public Core::ActionManager
{
    Q_OBJECT

public:
    explicit ActionManagerPrivate(MainWindow *mainWnd);
    ~ActionManagerPrivate();

    void setContext(const QList<int> &context);
    static ActionManagerPrivate *instance();

    void saveSettings(QSettings *settings);
    QList<int> defaultGroups() const;

    QList<CommandPrivate *> commands() const;
    QList<ActionContainerPrivate *> containers() const;

    bool hasContext(int context) const;

    Command *command(int uid) const;
    ActionContainer *actionContainer(int uid) const;

    void readSettings(QSettings *settings);

    //ActionManager Interface
    ActionContainer *createMenu(const QString &id);
    ActionContainer *createMenuBar(const QString &id);

    Command *registerAction(QAction *action, const QString &id,
        const QList<int> &context);
    Command *registerShortcut(QShortcut *shortcut, const QString &id,
        const QList<int> &context);

    Core::Command *command(const QString &id) const;
    Core::ActionContainer *actionContainer(const QString &id) const;

private:
    bool hasContext(QList<int> context) const;
    Command *registerOverridableAction(QAction *action, const QString &id,
        bool checkUnique);

    static ActionManagerPrivate* m_instance;
    QList<int> m_defaultGroups;

    typedef QHash<int, CommandPrivate *> IdCmdMap;
    IdCmdMap m_idCmdMap;

    typedef QHash<int, ActionContainerPrivate *> IdContainerMap;
    IdContainerMap m_idContainerMap;

//    typedef QMap<int, int> GlobalGroupMap;
//    GlobalGroupMap m_globalgroups;
//
    QList<int> m_context;

    MainWindow *m_mainWnd;
};

} // namespace Internal
} // namespace Core

#endif // ACTIONMANAGERPRIVATE_H
