/**
 ******************************************************************************
 *
 * @file       generalsettings.cpp
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

#include "generalsettings.h"

#include <utils/stylehelper.h>
#include <utils/qtcolorbutton.h>
#include <utils/consoleprocess.h>
#include <coreplugin/icore.h>
#include <QtGui/QMessageBox>

#include "ui_generalsettings.h"

using namespace Utils;
using namespace Core::Internal;

GeneralSettings::GeneralSettings():
    m_dialog(0)
{
}

QString GeneralSettings::id() const
{
    return QLatin1String("General");
}

QString GeneralSettings::trName() const
{
    return tr("General");
}

QString GeneralSettings::category() const
{
    return QLatin1String("Environment");
}

QString GeneralSettings::trCategory() const
{
    return tr("Environment");
}

QWidget *GeneralSettings::createPage(QWidget *parent)
{
    m_page = new Ui::GeneralSettings();
    QWidget *w = new QWidget(parent);
    m_page->setupUi(w);

    m_page->colorButton->setColor(StyleHelper::baseColor());
#ifdef Q_OS_UNIX
    m_page->terminalEdit->setText(ConsoleProcess::terminalEmulator(Core::ICore::instance()->settings()));
#else
    m_page->terminalLabel->hide();
    m_page->terminalEdit->hide();
    m_page->resetTerminalButton->hide();
#endif

    connect(m_page->resetButton, SIGNAL(clicked()),
            this, SLOT(resetInterfaceColor()));
    connect(m_page->resetEditorButton, SIGNAL(clicked()),
            this, SLOT(resetExternalEditor()));
    connect(m_page->helpExternalEditorButton, SIGNAL(clicked()),
            this, SLOT(showHelpForExternalEditor()));
#ifdef Q_OS_UNIX
    connect(m_page->resetTerminalButton, SIGNAL(clicked()),
            this, SLOT(resetTerminal()));
#endif

    return w;
}

void GeneralSettings::apply()
{
    // Apply the new base color if accepted
    StyleHelper::setBaseColor(m_page->colorButton->color());
#ifdef Q_OS_UNIX
	ConsoleProcess::setTerminalEmulator(Core::ICore::instance()->settings(),
                                        m_page->terminalEdit->text());
#endif
}

void GeneralSettings::finish()
{
    delete m_page;
}

void GeneralSettings::resetInterfaceColor()
{
    m_page->colorButton->setColor(0x666666);
}

void GeneralSettings::resetExternalEditor()
{
}

#ifdef Q_OS_UNIX
void GeneralSettings::resetTerminal()
{
    m_page->terminalEdit->setText(ConsoleProcess::defaultTerminalEmulator() + QLatin1String(" -e"));
}
#endif

void GeneralSettings::showHelpForExternalEditor()
{
    if (m_dialog) {
        m_dialog->show();
        m_dialog->raise();
        m_dialog->activateWindow();
        return;
    }
#if 0
    QMessageBox *mb = new QMessageBox(QMessageBox::Information,
                                  tr("Variables"),
                                  EditorManager::instance()->externalEditorHelpText(),
                                  QMessageBox::Cancel,
                                  m_page->helpExternalEditorButton);
    mb->setWindowModality(Qt::NonModal);
    m_dialog = mb;
    mb->show();
#endif
}
