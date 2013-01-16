/**
 ******************************************************************************
 *
 * @file       videogadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup VideoPlugin Video Plugin
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

#include "videogadgetoptionspage.h"
#include "videogadgetconfiguration.h"
#include "helpdialog.h"

#include "ui_videooptionspage.h"

VideoGadgetOptionsPage::VideoGadgetOptionsPage(VideoGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent), m_config(config)
{
    m_page = 0;
}

QWidget *VideoGadgetOptionsPage::createPage(QWidget *parent)
{
    m_page = new Ui::VideoOptionsPage();
    QWidget *w = new QWidget(parent);
    m_page->setupUi(w);

    m_page->respectAspectRatioCheckBox->setVisible(false);

    m_page->displayVideoCheckBox->setChecked(m_config->displayVideo());
    m_page->displayControlsCheckBox->setChecked(m_config->displayControls());
    m_page->autoStartCheckBox->setChecked(m_config->autoStart());
    m_page->respectAspectRatioCheckBox->setChecked(m_config->respectAspectRatio());
    m_page->descPlainTextEdit->setPlainText(m_config->pipelineDesc());
    m_page->infoPlainTextEdit->setPlainText(m_config->pipelineInfo());

    connect(m_page->helpButton, SIGNAL(clicked()), this, SLOT(openHelpDialog()));

    return w;
}

void VideoGadgetOptionsPage::apply()
{
    m_config->setDisplayVideo(m_page->displayVideoCheckBox->isChecked());
    m_config->setDisplayControls(m_page->displayControlsCheckBox->isChecked());
    m_config->setAutoStart(m_page->autoStartCheckBox->isChecked());
    m_config->setRespectAspectRatio(m_page->respectAspectRatioCheckBox->isChecked());
    m_config->setPipelineDesc(m_page->descPlainTextEdit->toPlainText());
    m_config->setPipelineInfo(m_page->infoPlainTextEdit->toPlainText());
}

void VideoGadgetOptionsPage::finish()
{
    delete m_page;
}

void VideoGadgetOptionsPage::openHelpDialog()
{
    HelpDialog dlg(0);
    dlg.execDialog();
}
