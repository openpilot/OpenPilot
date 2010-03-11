/**
 ******************************************************************************
 *
 * @file       generalsettings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   coreplugin
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

#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <coreplugin/dialogs/ioptionspage.h>
#include <QtCore/QPointer>
#include <QtGui/QWidget>

namespace Core {
namespace Internal {

namespace Ui {
    class GeneralSettings;
}

class GeneralSettings : public IOptionsPage
{
    Q_OBJECT

public:
    GeneralSettings();

    QString id() const;
    QString trName() const;
    QString category() const;
    QString trCategory() const;
    QWidget* createPage(QWidget *parent);
    void apply();
    void finish();

private slots:
    void resetInterfaceColor();
    void resetExternalEditor();
    void showHelpForExternalEditor();
#ifdef Q_OS_UNIX
    void resetTerminal();
#endif

private:
    Ui::GeneralSettings *m_page;
    QPointer<QWidget> m_dialog;
};

} // namespace Internal
} // namespace Core

#endif // GENERALSETTINGS_H
