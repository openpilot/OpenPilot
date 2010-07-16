/**
 ******************************************************************************
 *
 * @file       modelviewgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ModelViewPlugin ModelView Plugin
 * @{
 * @brief A gadget that displays a 3D representation of the UAV 
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

#include "modelviewgadgetoptionspage.h"
#include "modelviewgadgetconfiguration.h"

#include "ui_modelviewoptionspage.h"


ModelViewGadgetOptionsPage::ModelViewGadgetOptionsPage(ModelViewGadgetConfiguration *config, QObject *parent) :
    IOptionsPage(parent),
    m_config(config)
{
}

QWidget *ModelViewGadgetOptionsPage::createPage(QWidget *parent)
{
    m_page = new Ui::ModelViewOptionsPage();
    QWidget *w = new QWidget(parent);
    m_page->setupUi(w);

    m_page->modelPathChooser->setExpectedKind(Utils::PathChooser::File);
    m_page->modelPathChooser->setPromptDialogFilter(tr("3D model (*.dae *.3ds)"));
    m_page->modelPathChooser->setPromptDialogTitle(tr("Choose 3D model"));
    m_page->backgroundPathChooser->setExpectedKind(Utils::PathChooser::File);
    m_page->backgroundPathChooser->setPromptDialogFilter(tr("Images (*.png *.jpg *.bmp *.xpm)"));
    m_page->backgroundPathChooser->setPromptDialogTitle(tr("Choose background image"));


    m_page->modelPathChooser->setPath(m_config->acFilename());
    m_page->backgroundPathChooser->setPath(m_config->bgFilename());
    m_page->enableVbo->setChecked(m_config->vboEnabled());


    return w;
}

void ModelViewGadgetOptionsPage::apply()
{
    m_config->setAcFilename(m_page->modelPathChooser->path());
    m_config->setBgFilename(m_page->backgroundPathChooser->path());
    m_config->setVboEnabled(m_page->enableVbo->isChecked());
}

void ModelViewGadgetOptionsPage::finish()
{
    delete m_page;
}

