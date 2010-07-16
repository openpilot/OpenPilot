/**
 ******************************************************************************
 *
 * @file       workspacesettings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#ifndef WORKSPACESETTINGS_H
#define WORKSPACESETTINGS_H

#include <coreplugin/dialogs/ioptionspage.h>
#include <QtCore/QObject>
#include <QtCore/QStringList>

class QSettings;

namespace Core {

    class ModeManager;

namespace Internal {

namespace Ui {
    class WorkspaceSettings;
}

class WorkspaceSettings : public IOptionsPage
{
Q_OBJECT
public:
    WorkspaceSettings(QObject *parent = 0);
    ~WorkspaceSettings();

    // IOptionsPage
    QString id() const;
    QString trName() const;
    QString category() const;
    QString trCategory() const;

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

signals:

public slots:
    void selectWorkspace(int index, bool store = false);
    void numberOfWorkspacesChanged(int value);
    void textEdited(QString string);
    void iconChanged();
private:
    Ui::WorkspaceSettings *m_page;
    QSettings *m_settings;
    ModeManager *m_modeManager;
    QStringList m_iconNames;
    QStringList m_names;
    int m_currentIndex;
    static const int MAX_WORKSPACES;

};

} // namespace Internal
} // namespace Core

#endif // WORKSPACESETTINGS_H
