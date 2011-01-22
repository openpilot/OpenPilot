/**
 ******************************************************************************
 *
 * @file       systemhealthgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup SystemHealthPlugin System Health Plugin
 * @{
 * @brief The System Health gadget plugin
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

    Q_UNUSED(parent);
    options_page = new Ui::SystemHealthGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);

    // Restore the contents from the settings:
    options_page->svgFilePathChooser->setExpectedKind(Utils::PathChooser::File);
    options_page->svgFilePathChooser->setPromptDialogFilter(tr("SVG image (*.svg)"));
    options_page->svgFilePathChooser->setPromptDialogTitle(tr("Choose SVG image"));
    options_page->svgFilePathChooser->setPath(m_config->getSystemFile());

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
    m_config->setSystemFile(options_page->svgFilePathChooser->path());
}


void SystemHealthGadgetOptionsPage::finish()
{
    delete options_page;
}
