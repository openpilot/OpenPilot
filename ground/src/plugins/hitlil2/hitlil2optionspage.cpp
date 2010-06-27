/**
 ******************************************************************************
 *
 * @file       hitlil2optionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlil2plugin
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

#include "hitlil2optionspage.h"
#include "hitlil2configuration.h"
#include "ui_hitlil2optionspage.h"


HITLIL2OptionsPage::HITLIL2OptionsPage(HITLIL2Configuration *config, QObject *parent) :
    IOptionsPage(parent),
    m_config(config)
{
}

QWidget *HITLIL2OptionsPage::createPage(QWidget *parent)
{
    // Create page
    m_optionsPage = new Ui::HITLIL2OptionsPage();
    QWidget* optionsPageWidget = new QWidget;
    m_optionsPage->setupUi(optionsPageWidget);


    // Restore the contents from the settings:
    m_optionsPage->Il2Port->setValue(m_config->Il2Port());
    m_optionsPage->Il2HostName->setText(m_config->Il2HostName());
    m_optionsPage->il2ManualControl->setChecked(m_config->il2ManualControl());


    return optionsPageWidget;
}

void HITLIL2OptionsPage::apply()
{
    m_config->setIl2Port( m_optionsPage->Il2Port->value());
    m_config->setIl2HostName( m_optionsPage->Il2HostName->text());
    m_config->setIl2ManualControl( m_optionsPage->il2ManualControl->isChecked());
}

void HITLIL2OptionsPage::finish()
{
    delete m_optionsPage;
}
