/**
 ******************************************************************************
 *
 * @file       escstartpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup [Group]
 * @{
 * @addtogroup ESCStartPage
 * @{
 * @brief [Brief]
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

#include "escstartpage.h"
#include "ui_escstartpage.h"
#include <extensionsystem/pluginmanager.h>


ESCStartPage::ESCStartPage(ESCWizard* wizard, QWidget *parent) :
    AbstractWizardPage<ESCWizard>(wizard, parent),
    ui(new Ui::ESCStartPage), m_telemtryManager(0), m_connectionManager(0)
{
    m_connectionManager = getWizard()->getConnectionManager();
    Q_ASSERT(m_connectionManager);

    ExtensionSystem::PluginManager *pluginManager = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pluginManager);
    m_telemtryManager = pluginManager->getObject<TelemetryManager>();
    Q_ASSERT(m_telemtryManager);
    connect(m_telemtryManager, SIGNAL(connected()), this, SLOT(connectionStatusChanged()));
    connect(m_telemtryManager, SIGNAL(disconnected()), this, SLOT(connectionStatusChanged()));

    ui->setupUi(this);    
}

ESCStartPage::~ESCStartPage()
{
    delete ui;
}

void ESCStartPage::initializePage()
{
}

bool ESCStartPage::isComplete() const
{
    bool result = m_telemtryManager->isConnected() &&
            m_connectionManager->getCurrentDevice().getConName().startsWith("USB:", Qt::CaseInsensitive);
    ui->connectLabel->setVisible(!result);
    return result;
}

void ESCStartPage::connectionStatusChanged()
{
    emit completeChanged();
}
