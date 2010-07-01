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

#include "opmap_edit_waypoint_dialog.h"
#include "ui_opmap_edit_waypoint_dialog.h"

// *********************************************************************

// constructor
opmap_edit_waypoint_dialog::opmap_edit_waypoint_dialog(QWidget *parent) :
    QDialog(parent, Qt::Dialog),
    ui(new Ui::opmap_edit_waypoint_dialog)
{
    ui->setupUi(this);

    waypoint_item = NULL;
}

// destrutor
opmap_edit_waypoint_dialog::~opmap_edit_waypoint_dialog()
{
    delete ui;
}

// *********************************************************************

void opmap_edit_waypoint_dialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void opmap_edit_waypoint_dialog::on_pushButtonOK_clicked()
{
    int res = saveSettings();
    if (res < 0) return;

    waypoint_item = NULL;

    close();
}

void opmap_edit_waypoint_dialog::on_pushButtonApply_clicked()
{
    saveSettings();
}

void opmap_edit_waypoint_dialog::on_pushButtonRevert_clicked()
{
    ui->checkBoxLocked->setChecked(original_locked);
    ui->spinBoxNumber->setValue(original_number);
    ui->doubleSpinBoxLatitude->setValue(original_coord.Lat());
    ui->doubleSpinBoxLongitude->setValue(original_coord.Lng());
    ui->doubleSpinBoxAltitude->setValue(original_altitude);
    ui->lineEditDescription->setText(original_description);

    saveSettings();
}

void opmap_edit_waypoint_dialog::on_pushButtonCancel_clicked()
{
    waypoint_item = NULL;
    close();
}

// *********************************************************************

int opmap_edit_waypoint_dialog::saveSettings()
{
    // ********************
    // fetch the various ui item values

    bool locked = ui->checkBoxLocked->isChecked();

    int number = ui->spinBoxNumber->value();
    if (number < 0)
    {
	return -1;
    }

    double latitude = ui->doubleSpinBoxLatitude->value();
    double longitude = ui->doubleSpinBoxLongitude->value();

    double altitude = ui->doubleSpinBoxAltitude->value();

    QString description = ui->lineEditDescription->displayText().simplified();

    // ********************
    // transfer the settings to the actual waypoint

    waypoint_item->SetNumber(number);
    waypoint_item->SetCoord(internals::PointLatLng(latitude, longitude));
    waypoint_item->SetAltitude(altitude);
    waypoint_item->SetDescription(description);
    waypoint_item->setFlag(QGraphicsItem::ItemIsMovable, !locked);

    // ********************

    return 0;	// all ok
}

// *********************************************************************
// public functions

void opmap_edit_waypoint_dialog::editWaypoint(mapcontrol::WayPointItem *waypoint_item)
{
    if (!waypoint_item) return;

    this->waypoint_item = waypoint_item;

    original_number = this->waypoint_item->Number();
    original_locked = (this->waypoint_item->flags() & QGraphicsItem::ItemIsMovable) == 0;
    original_coord = this->waypoint_item->Coord();
    original_altitude = this->waypoint_item->Altitude();
    original_description = this->waypoint_item->Description().simplified();

    ui->checkBoxLocked->setChecked(original_locked);
    ui->spinBoxNumber->setValue(original_number);
    ui->doubleSpinBoxLatitude->setValue(original_coord.Lat());
    ui->doubleSpinBoxLongitude->setValue(original_coord.Lng());
    ui->doubleSpinBoxAltitude->setValue(original_altitude);
    ui->lineEditDescription->setText(original_description);

    show();
}

// *********************************************************************
