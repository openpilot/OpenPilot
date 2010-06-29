/**
 ******************************************************************************
 *
 * @file       opmap_edit_waypoint_dialog.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   opmap
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

#ifndef OPMAP_EDIT_WAYPOINT_DIALOG_H
#define OPMAP_EDIT_WAYPOINT_DIALOG_H

#include <QDialog>

namespace Ui {
    class opmap_edit_waypoint_dialog;
}

class opmap_edit_waypoint_dialog : public QDialog {
    Q_OBJECT
public:
    opmap_edit_waypoint_dialog(QWidget *parent = 0);
    ~opmap_edit_waypoint_dialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::opmap_edit_waypoint_dialog *ui;

private slots:

private slots:
    void on_pushButtonCancel_clicked();
    void on_pushButtonRevert_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonOK_clicked();
};

#endif // OPMAP_EDIT_WAYPOINT_DIALOG_H
