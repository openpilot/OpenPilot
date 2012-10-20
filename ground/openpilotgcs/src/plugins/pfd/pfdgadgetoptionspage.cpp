/**
 ******************************************************************************
 *
 * @file       pfdgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin Primary Flight Display Plugin
 * @{
 * @brief The Primary Flight Display Gadget 
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

#include "pfdgadgetoptionspage.h"
#include "pfdgadgetconfiguration.h"
#include "ui_pfdgadgetoptionspage.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavdataobject.h"


#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

PFDGadgetOptionsPage::PFDGadgetOptionsPage(PFDGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget (uses the UI file)
QWidget *PFDGadgetOptionsPage::createPage(QWidget *parent)
{
    Q_UNUSED(parent);

    options_page = new Ui::PFDGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);



    // Restore the contents from the settings:
    options_page->svgSourceFile->setExpectedKind(Utils::PathChooser::File);
    options_page->svgSourceFile->setPromptDialogFilter(tr("SVG image (*.svg)"));
    options_page->svgSourceFile->setPromptDialogTitle(tr("Choose SVG image"));
    options_page->svgSourceFile->setPath(m_config->dialFile());
    options_page->useOpenGL->setChecked(m_config->useOpenGL());
    options_page->hqText->setChecked(m_config->getHqFonts());
    options_page->smoothUpdates->setChecked(m_config->getBeSmooth());

    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void PFDGadgetOptionsPage::apply()
{
    m_config->setDialFile(options_page->svgSourceFile->path());
    m_config->setUseOpenGL(options_page->useOpenGL->checkState());
    m_config->setHqFonts(options_page->hqText->checkState());
    m_config->setBeSmooth(options_page->smoothUpdates->checkState());
}



void PFDGadgetOptionsPage::finish()
{
}
