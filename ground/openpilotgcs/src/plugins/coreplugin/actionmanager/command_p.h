/**
 ******************************************************************************
 *
 * @file       command_p.h
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

#ifndef COMMAND_P_H
#define COMMAND_P_H

#include "command.h"
#include "actionmanager_p.h"

#include <QtCore/QList>
#include <QtCore/QMultiMap>
#include <QtCore/QPointer>
#include <QtGui/QKeySequence>

namespace Core {
namespace Internal {

class CommandPrivate : public Core::Command
{
    Q_OBJECT
public:
    CommandPrivate(int id);
    virtual ~CommandPrivate() {}

    virtual QString name() const = 0;

    void setDefaultKeySequence(const QKeySequence &key);
    QKeySequence defaultKeySequence() const;

    void setDefaultText(const QString &text);
    QString defaultText() const;

    int id() const;

    QAction *action() const;
    QShortcut *shortcut() const;

    void setAttribute(CommandAttribute attr);
    void removeAttribute(CommandAttribute attr);
    bool hasAttribute(CommandAttribute attr) const;

    virtual bool setCurrentContext(const QList<int> &context) = 0;

    QString stringWithAppendedShortcut(const QString &str) const;

protected:
    QString m_category;
    int m_attributes;
    int m_id;
    QKeySequence m_defaultKey;
    QString m_defaultText;
};

class Shortcut : public CommandPrivate
{
    Q_OBJECT
public:
    Shortcut(int id);

    QString name() const;

    void setDefaultKeySequence(const QKeySequence &key);
    void setKeySequence(const QKeySequence &key);
    QKeySequence keySequence() const;

    virtual void setDefaultText(const QString &key);
    virtual QString defaultText() const;

    void setShortcut(QShortcut *shortcut);
    QShortcut *shortcut() const;

    void setContext(const QList<int> &context);
    QList<int> context() const;
    bool setCurrentContext(const QList<int> &context);

    bool isActive() const;
private:
    QList<int> m_context;
    QShortcut *m_shortcut;
    QString m_defaultText;
};

class Action : public CommandPrivate
{
    Q_OBJECT
public:
    Action(int id);

    QString name() const;

    void setDefaultKeySequence(const QKeySequence &key);
    void setKeySequence(const QKeySequence &key);
    QKeySequence keySequence() const;

    virtual void setAction(QAction *action);
    QAction *action() const;

    void setLocations(const QList<CommandLocation> &locations);
    QList<CommandLocation> locations() const;

protected:
    void updateToolTipWithKeySequence();
    
    QAction *m_action;
    QList<CommandLocation> m_locations;
    QString m_toolTip;
};

class OverrideableAction : public Action
{
    Q_OBJECT

public:
    OverrideableAction(int id);

    void setAction(QAction *action);
    bool setCurrentContext(const QList<int> &context);
    void addOverrideAction(QAction *action, const QList<int> &context);
    bool isActive() const;

private slots:
    void actionChanged();

private:
    QPointer<QAction> m_currentAction;
    QList<int> m_context;
    QMap<int, QPointer<QAction> > m_contextActionMap;
    bool m_active;
    bool m_contextInitialized;
};

} // namespace Internal
} // namespace Core

#endif // COMMAND_P_H
