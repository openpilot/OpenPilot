/**
 ******************************************************************************
 *
 * @file       systemhealthgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      System Health Plugin Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   systemhealth
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

#include "systemhealthgadgetoptionspage.h"
#include "systemhealthgadgetconfiguration.h"
#include "ui_systemhealthgadgetoptionspage.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

SystemHealthGadgetOptionsPage::SystemHealthGadgetOptionsPage(SystemHealthGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget (uses the UI file)
QWidget *SystemHealthGadgetOptionsPage::createPage(QWidget *parent)
{

    options_page = new Ui::SystemHealthGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);

    // Restore the contents from the settings:
    options_page->svgSourceFile->setText(m_config->getSystemFile());

    connect(options_page->loadFile, SIGNAL(clicked()), this, SLOT(on_loadFile_clicked()));

    return optionsPageWidget;
}
/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void SystemHealthGadgetOptionsPage::apply()
{
    m_config->setSystemFile(options_page->svgSourceFile->text());
}

/**

Opens an open file dialog.

*/
void SystemHealthGadgetOptionsPage::on_loadFile_clicked()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(this),
                                                    tr("QFileDialog::getOpenFileName()"),
                                                    options_page->svgSourceFile->text(),
                                                    tr("All Files (*);;SVG Files (*.svg)"),
                                                    &selectedFilter,
                                                    options);
    if (!fileName.isEmpty()) options_page->svgSourceFile->setText(fileName);

}


void SystemHealthGadgetOptionsPage::finish()
{

}
