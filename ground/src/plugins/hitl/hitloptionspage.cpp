/**
 ******************************************************************************
 *
 * @file       hitloptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitl
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

#include "hitloptionspage.h"
#include "hitlconfiguration.h"
#include "ui_hitloptionspage.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

HITLOptionsPage::HITLOptionsPage(HITLConfiguration *config, QObject *parent) :
    IOptionsPage(parent),
    m_config(config)
{
}

QWidget *HITLOptionsPage::createPage(QWidget *parent)
{
    // Create page
    m_optionsPage = new Ui::HITLOptionsPage();
    QWidget* optionsPageWidget = new QWidget;
    m_optionsPage->setupUi(optionsPageWidget);

    m_optionsPage->executablePathChooser->setExpectedKind(Utils::PathChooser::File);
    m_optionsPage->executablePathChooser->setPromptDialogTitle(tr("Choose FlightGear executable"));
    m_optionsPage->dataDirectoryPathChooser->setExpectedKind(Utils::PathChooser::Directory);
    m_optionsPage->dataDirectoryPathChooser->setPromptDialogTitle(tr("Choose FlightGear data directory"));

    // Restore the contents from the settings:
    m_optionsPage->executablePathChooser->setPath(m_config->fgPathBin());
    m_optionsPage->dataDirectoryPathChooser->setPath(m_config->fgPathData());
    m_optionsPage->fgManualControl->setChecked(m_config->fgManualControl());


    return optionsPageWidget;
}

void HITLOptionsPage::apply()
{
    m_config->setFGPathBin( m_optionsPage->executablePathChooser->path());
    m_config->setFGPathData( m_optionsPage->dataDirectoryPathChooser->path());
    m_config->setFGManualControl( m_optionsPage->fgManualControl->isChecked());
}

void HITLOptionsPage::finish()
{
    delete m_optionsPage;
}
