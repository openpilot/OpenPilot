/**
 ******************************************************************************
 *
 * @file       shortcutsettings.h
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

#ifndef SHORTCUTSETTINGS_H
#define SHORTCUTSETTINGS_H

#include <coreplugin/dialogs/ioptionspage.h>

#include <QtCore/QObject>
#include <QtGui/QKeySequence>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QKeyEvent>

QT_BEGIN_NAMESPACE
class Ui_ShortcutSettings;
QT_END_NAMESPACE

namespace Core {

class Command;

namespace Internal {

class ActionManagerPrivate;
class MainWindow;

struct ShortcutItem
{
    Command *m_cmd;
    QKeySequence m_key;
    QTreeWidgetItem *m_item;
};


class ShortcutSettings : public Core::IOptionsPage
{
    Q_OBJECT

public:
    ShortcutSettings(QObject *parent = 0);
    ~ShortcutSettings();

    // IOptionsPage
    QString id() const;
    QString trName() const;
    QString category() const;
    QString trCategory() const;

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void commandChanged(QTreeWidgetItem *current);
    void filterChanged(const QString &f);
    void keyChanged();
    void resetKeySequence();
    void removeKeySequence();
    void importAction();
    void exportAction();
    void defaultAction();

private:
    void setKeySequence(const QKeySequence &key);
    bool filter(const QString &f, const QTreeWidgetItem *item);
    void initialize();

    void handleKeyEvent(QKeyEvent *e);
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);

    QList<ShortcutItem *> m_scitems;
    ActionManagerPrivate *m_am;
    int m_key[4], m_keyNum;
    Ui_ShortcutSettings *m_page;
};

} // namespace Internal
} // namespace Core

#endif // SHORTCUTSETTINGS_H
