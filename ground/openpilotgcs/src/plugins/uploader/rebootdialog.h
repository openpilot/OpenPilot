/**
 ******************************************************************************
 *
 * @file       rebootdialog.h
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

#ifndef REBOOTDIALOG_H
#define REBOOTDIALOG_H

#include <QDialog>
#include "uploadergadgetwidget.h"

namespace Ui {
class RebootDialog;
}

class RebootDialog : public QDialog {
    Q_OBJECT

public:
    explicit RebootDialog(UploaderGadgetWidget *uploader);
    ~RebootDialog();

signals:
    void reboot();

private slots:
    void on_okButton_clicked();
    void progressUpdate(uploader::ProgressStep progress, QVariant message);

private:
    Ui::RebootDialog *ui;
    UploaderGadgetWidget *m_uploader;

public slots:
    int exec();
};

#endif // REBOOTDIALOG_H
