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
    ui(new Ui::RebootPage), m_toggl(false), m_isRebooting(false)
{
    ui->setupUi(this);
    ui->yellowLabel->setVisible(false);
    ui->redLabel->setVisible(true);
    connect(ui->rebootButton, SIGNAL(clicked()), this, SLOT(reboot()));
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    m_uploader = pm->getObject<UploaderGadgetFactory>();
    Q_ASSERT(m_uploader);
}

RebootPage::~RebootPage()
{
    disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(toggleLabel()));
    m_timer.stop();
    delete ui;
}

void RebootPage::initializePage()
{
    ui->messageLabel->setText("");
    ui->rebootProgress->setValue(0);
    if (!m_timer.isActive()) {
        connect(&m_timer, SIGNAL(timeout()), this, SLOT(toggleLabel()));
        m_timer.setInterval(500);
        m_timer.setSingleShot(false);
        m_timer.start();
    }
}

bool RebootPage::validatePage()
{
    return !m_isRebooting;
}

void RebootPage::toggleLabel()
{
    m_toggl = !m_toggl;
    ui->yellowLabel->setVisible(m_toggl);
    ui->redLabel->setVisible(!m_toggl);
}

void RebootPage::enableButtons(bool enable)
{
    getWizard()->button(QWizard::NextButton)->setEnabled(enable);
    getWizard()->button(QWizard::CancelButton)->setEnabled(enable);
    getWizard()->button(QWizard::BackButton)->setEnabled(enable);
    getWizard()->button(QWizard::CustomButton1)->setEnabled(enable);
    ui->rebootButton->setEnabled(enable);
}

void RebootPage::reboot()
{
    enableButtons(false);
    ui->rebootProgress->setValue(0);
    QApplication::processEvents();
    connect(m_uploader, SIGNAL(progressUpdate(uploader::ProgressStep, QVariant)), this, SLOT(progressUpdate(uploader::ProgressStep, QVariant)));
    ui->messageLabel->setText(tr("Reboot in progress..."));
    m_isRebooting = true;
    m_uploader->reboot();
}

void RebootPage::progressUpdate(uploader::ProgressStep progress, QVariant message)
{
    Q_UNUSED(message);
    if (progress == uploader::SUCCESS || progress == uploader::FAILURE) {
        disconnect(m_uploader, SIGNAL(progressUpdate(uploader::ProgressStep, QVariant)), this, SLOT(progressUpdate(uploader::ProgressStep, QVariant)));
        if (progress == uploader::FAILURE) {
            ui->messageLabel->setText(tr("<font color='red'>Software reboot failed!</font><p> Please perform a manual reboot by power cycling the board. "
                                         "To power cycle the controller remove all batteries and the USB cable for at least 30 seconds. "
                                         "After 30 seconds, plug in the board again and wait for it to connect, this can take a few seconds. Then press Next."));
        } else {
            ui->messageLabel->setText(tr("<font color='green'>Reboot complete!</font>"));
        }
        m_isRebooting= false;
        enableButtons(true);
    } else {
        ui->rebootProgress->setValue(ui->rebootProgress->value() + 1);
    }
}


bool RebootPage::isComplete() const
{
    return !m_isRebooting;
}
