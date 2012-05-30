/**
 ******************************************************************************
 *
 * @file       opmap_edit_waypoint_dialog.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin OpenPilot Map Plugin
 * @{
 * @brief The OpenPilot Map plugin 
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
    connect(ui->rbRelative,SIGNAL(toggled(bool)),this,SLOT(setupWidgets(bool)));
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

void opmap_edit_waypoint_dialog::setupWidgets(bool isRelative)
{
    ui->lbLong->setVisible(!isRelative);
    ui->lbDegLong->setVisible(!isRelative);
    ui->doubleSpinBoxLongitude->setVisible(!isRelative);
    ui->lbLat->setVisible(!isRelative);
    ui->lbDegLat->setVisible(!isRelative);
    ui->doubleSpinBoxLatitude->setVisible(!isRelative);
    ui->lbDistance->setVisible(isRelative);
    ui->lbDistanceMeters->setVisible(isRelative);
    ui->lbBearing->setVisible(isRelative);
    ui->lbBearingDeg->setVisible(isRelative);
    ui->spinBoxDistance->setVisible(isRelative);
    ui->doubleSpinBoxBearing->setVisible(isRelative);
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
    if(ui->rbAbsolute->isChecked())
        waypoint_item->setWPType(mapcontrol::WayPointItem::absolute);
    else
        waypoint_item->setWPType(mapcontrol::WayPointItem::relative);
    mapcontrol::distBearing pt;
    pt.distance=ui->spinBoxDistance->value();
    pt.bearing=ui->doubleSpinBoxBearing->value()/180*M_PI;
    this->waypoint_item->setRelativeCoord(pt);
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
    original_type=this->waypoint_item->WPType();
    original_distance=this->waypoint_item->getRelativeCoord().distance;
    original_bearing=this->waypoint_item->getRelativeCoord().bearing*180/M_PI;
    ui->checkBoxLocked->setChecked(original_locked);
    ui->spinBoxNumber->setValue(original_number);
    ui->doubleSpinBoxLatitude->setValue(original_coord.Lat());
    ui->doubleSpinBoxLongitude->setValue(original_coord.Lng());
    ui->doubleSpinBoxAltitude->setValue(original_altitude);
    ui->lineEditDescription->setText(original_description);
    if(original_type==mapcontrol::WayPointItem::absolute)
        ui->rbAbsolute->setChecked(true);
    else
        ui->rbRelative->setChecked(true);
    ui->doubleSpinBoxBearing->setValue(original_bearing);
    ui->spinBoxDistance->setValue(original_distance);
    setupWidgets(ui->rbRelative->isChecked());
    show();
}

// *********************************************************************
