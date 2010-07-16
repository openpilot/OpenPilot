/**
 ******************************************************************************
 *
 * @file       IPconnectionoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup IPConnPlugin IP Telemetry Plugin
 * @{
 * @brief IP Connection Plugin impliment telemetry over TCP/IP and UDP/IP
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

#include "ipconnectionoptionspage.h"
#include "ipconnectionconfiguration.h"
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QRadioButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>

#include "ui_ipconnectionoptionspage.h"


IPconnectionOptionsPage::IPconnectionOptionsPage(IPconnectionConfiguration *config, QObject *parent) :
    IOptionsPage(parent),
    m_config(config)
{

}
IPconnectionOptionsPage::~IPconnectionOptionsPage()
{
}
QWidget *IPconnectionOptionsPage::createPage(QWidget *parent)
{

    m_page = new Ui::IPconnectionOptionsPage();
    QWidget *w = new QWidget(parent);
    m_page->setupUi(w);

    m_page->Port->setValue(m_config->Port());
    m_page->HostName->setText(m_config->HostName());
    m_page->UseTCP->setChecked(m_config->UseTCP()?true:false);
    m_page->UseUDP->setChecked(m_config->UseTCP()?false:true);

    return w;
}

void IPconnectionOptionsPage::apply()
{
    m_config->setPort(m_page->Port->value());
    m_config->setHostName(m_page->HostName->text());
    m_config->setUseTCP(m_page->UseTCP->isChecked()?1:0);
    m_config->savesettings();

    emit availableDevChanged();



}

void IPconnectionOptionsPage::finish()
{
    delete m_page;
}

