/**
 ******************************************************************************
 *
 * @file       rebootpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup RebootPage
 * @{
 * @brief
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

#include "rebootpage.h"
#include "ui_rebootpage.h"

RebootPage::RebootPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::RebootPage), m_toggl(false)
{
    ui->setupUi(this);
    ui->yellowLabel->setVisible(false);
    ui->redLabel->setVisible(true);
}

RebootPage::~RebootPage()
{
    disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(toggleLabel()));
    m_timer.stop();
    delete ui;
}

void RebootPage::initializePage()
{
    if(!m_timer.isActive()) {
        connect(&m_timer, SIGNAL(timeout()), this, SLOT(toggleLabel()));
        m_timer.setInterval(500);
        m_timer.setSingleShot(false);
        m_timer.start();
    }
}

bool RebootPage::validatePage()
{
    return true;
}

void RebootPage::toggleLabel()
{
    m_toggl = !m_toggl;
    ui->yellowLabel->setVisible(m_toggl);
    ui->redLabel->setVisible(!m_toggl);
}
