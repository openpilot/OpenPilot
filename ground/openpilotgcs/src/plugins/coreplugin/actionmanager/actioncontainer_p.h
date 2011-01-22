/**
 ******************************************************************************
 *
 * @file       actioncontainer_p.h
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

#ifndef ACTIONCONTAINER_P_H
#define ACTIONCONTAINER_P_H

#include "actionmanager_p.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/command.h>

namespace Core {
namespace Internal {

class ActionContainerPrivate : public Core::ActionContainer
{
public:
    ActionContainerPrivate(int id);
    virtual ~ActionContainerPrivate() {}

    void setEmptyAction(EmptyAction ea);
    bool hasEmptyAction(EmptyAction ea) const;

    QAction *insertLocation(const QString &group) const;
    void appendGroup(const QString &group);
    void addAction(Command *action, const QString &group = QString());
    void addMenu(ActionContainer *menu, const QString &group = QString());

    int id() const;

    QMenu *menu() const;
    QMenuBar *menuBar() const;

    virtual void insertAction(QAction *before, QAction *action) = 0;
    virtual void insertMenu(QAction *before, QMenu *menu) = 0;

    QList<Command *> commands() const { return m_commands; }
    QList<ActionContainer *> subContainers() const { return m_subContainers; }
protected:
    bool canAddAction(Command *action) const;
    bool canAddMenu(ActionContainer *menu) const;
    virtual bool canBeAddedToMenu() const = 0;

    void addAction(Command *action, int pos, bool setpos);
    void addMenu(ActionContainer *menu, int pos, bool setpos);

private:
    QAction *beforeAction(int pos, int *prevKey) const;
    int calcPosition(int pos, int prevKey) const;

    QList<int> m_groups;
    int m_data;
    int m_id;
    QMap<int, int> m_posmap;
    QList<ActionContainer *> m_subContainers;
    QList<Command *> m_commands;
};

class MenuActionContainer : public ActionContainerPrivate
{
public:
    MenuActionContainer(int id);

    void setMenu(QMenu *menu);
    QMenu *menu() const;

    void setLocation(const CommandLocation &location);
    CommandLocation location() const;

    void insertAction(QAction *before, QAction *action);
    void insertMenu(QAction *before, QMenu *menu);
    bool update();

protected:
    bool canBeAddedToMenu() const;
private:
    QMenu *m_menu;
    CommandLocation m_location;
};

class MenuBarActionContainer : public ActionContainerPrivate
{
public:
    MenuBarActionContainer(int id);

    void setMenuBar(QMenuBar *menuBar);
    QMenuBar *menuBar() const;

    void insertAction(QAction *before, QAction *action);
    void insertMenu(QAction *before, QMenu *menu);
    bool update();

protected:
    bool canBeAddedToMenu() const;
private:
    QMenuBar *m_menuBar;
};

} // namespace Internal
} // namespace Core

#endif // ACTIONCONTAINER_P_H
