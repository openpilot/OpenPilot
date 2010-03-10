/**
 ******************************************************************************
 *
 * @file       settingsdialog.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   
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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "ui_settingsdialog.h"

#include <QtCore/QList>

#include "coreplugin/dialogs/ioptionspage.h"

namespace Core {
namespace Internal {

class SettingsDialog : public QDialog, public ::Ui::SettingsDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent,
                   const QString &initialCategory = QString(),
                   const QString &initialPage = QString());
    ~SettingsDialog();

    // Run the dialog and return true if 'Ok' was choosen or 'Apply' was invoked
    // at least once
    bool execDialog();

public slots:
    void done(int);

private slots:
    void pageSelected();
    void accept();
    void reject();
    void apply();

private:
    QList<Core::IOptionsPage*> m_pages;
    bool m_applied;
    QString m_currentCategory;
    QString m_currentPage;
};

} // namespace Internal
} // namespace Core

#endif // SETTINGSDIALOG_H
