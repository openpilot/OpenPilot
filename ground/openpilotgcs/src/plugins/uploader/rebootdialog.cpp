/**
 ******************************************************************************
 *
 * @file       rebootdialog.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup [Group]
 * @{
 * @addtogroup RebootDialog
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

#include "rebootdialog.h"
#include "ui_rebootdialog.h"

RebootDialog::RebootDialog(UploaderGadgetWidget *uploader) :
    QDialog(uploader),
    ui(new Ui::RebootDialog), m_uploader(uploader)
{
    setWindowFlags(((windowFlags() | Qt::CustomizeWindowHint)
                    & ~Qt::WindowCloseButtonHint & ~Qt::WindowMinMaxButtonsHint));
    ui->setupUi(this);
    connect(this, SIGNAL(reboot()), m_uploader, SLOT(systemReboot()));
    ui->rebootProgressBar->setVisible(true);
    ui->okButton->setVisible(false);
}

RebootDialog::~RebootDialog()
{
    delete ui;
}

void RebootDialog::on_okButton_clicked()
{
    reject();
}

int RebootDialog::exec()
{
    show();
    connect(m_uploader, SIGNAL(progressUpdate(uploader::ProgressStep, QVariant)), this, SLOT(progressUpdate(uploader::ProgressStep, QVariant)));
    emit reboot();
    return result();
}

void RebootDialog::progressUpdate(uploader::ProgressStep progress, QVariant message)
{
    Q_UNUSED(message);
    if (progress == uploader::SUCCESS || progress == uploader::FAILURE) {
        disconnect(m_uploader, SIGNAL(progressUpdate(uploader::ProgressStep, QVariant)), this, SLOT(progressUpdate(uploader::ProgressStep, QVariant)));
        if (progress == uploader::FAILURE) {
            ui->rebootProgressBar->setVisible(false);
            ui->okButton->setVisible(true);
            ui->label->setText(tr("<font color='red'>Reboot failed!</font><p>Please perform a manual reboot by power cycling the board.<br>"
                                  "To power cycle the controller remove all batteries and the USB cable for at least 30 seconds.<br>"
                                  "After 30 seconds, plug in the board again and wait for it to connect, this can take a few seconds.<br>"
                                  "Then press Ok.</p>"));
            QDialog::exec();
        } else {
            accept();
        }
    }
}
