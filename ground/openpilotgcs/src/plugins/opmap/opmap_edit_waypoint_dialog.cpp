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
#include "opmapcontrol/opmapcontrol.h"
// *********************************************************************
typedef enum { MODE_FLYENDPOINT=0, MODE_FLYVECTOR=1, MODE_FLYCIRCLERIGHT=2, MODE_FLYCIRCLELEFT=3,
               MODE_DRIVEENDPOINT=4, MODE_DRIVEVECTOR=5, MODE_DRIVECIRCLELEFT=6, MODE_DRIVECIRCLERIGHT=7,
               MODE_FIXEDATTITUDE=8, MODE_SETACCESSORY=9, MODE_DISARMALARM=10 } ModeOptions;
typedef enum { ENDCONDITION_NONE=0, ENDCONDITION_TIMEOUT=1, ENDCONDITION_DISTANCETOTARGET=2,
               ENDCONDITION_LEGREMAINING=3, ENDCONDITION_ABOVEALTITUDE=4, ENDCONDITION_POINTINGTOWARDSNEXT=5,
               ENDCONDITION_PYTHONSCRIPT=6, ENDCONDITION_IMMEDIATE=7 } EndConditionOptions;


// constructor
opmap_edit_waypoint_dialog::opmap_edit_waypoint_dialog(QWidget *parent) :
    QDialog(parent, Qt::Dialog),
    ui(new Ui::opmap_edit_waypoint_dialog)
{
    ui->setupUi(this);
    my_waypoint = NULL;
    connect(ui->rbRelative,SIGNAL(toggled(bool)),this,SLOT(setupPositionWidgets(bool)));
    connect(ui->cbMode,SIGNAL(currentIndexChanged(int)),this,SLOT(setupModeWidgets()));
    connect(ui->cbCondition,SIGNAL(currentIndexChanged(int)),this,SLOT(setupConditionWidgets()));
    ui->cbMode->addItem("Fly Direct",MODE_FLYENDPOINT);
    ui->cbMode->addItem("Fly Vector",MODE_FLYVECTOR);
    ui->cbMode->addItem("Fly Circle Right",MODE_FLYCIRCLERIGHT);
    ui->cbMode->addItem("Fly Circle Left",MODE_FLYCIRCLELEFT);

    ui->cbMode->addItem("Drive Direct",MODE_DRIVEENDPOINT);
    ui->cbMode->addItem("Drive Vector",MODE_DRIVEVECTOR);
    ui->cbMode->addItem("Drive Circle Right",MODE_DRIVECIRCLELEFT);
    ui->cbMode->addItem("Drive Circle Left",MODE_DRIVECIRCLERIGHT);

    ui->cbMode->addItem("Fixed Attitude",MODE_FIXEDATTITUDE);
    ui->cbMode->addItem("Set Accessory",MODE_SETACCESSORY);
    ui->cbMode->addItem("Disarm Alarm",MODE_DISARMALARM);

    ui->cbCondition->addItem("None",ENDCONDITION_NONE);
    ui->cbCondition->addItem("Timeout",ENDCONDITION_TIMEOUT);
    ui->cbCondition->addItem("Distance to tgt",ENDCONDITION_DISTANCETOTARGET);
    ui->cbCondition->addItem("Leg remaining",ENDCONDITION_LEGREMAINING);
    ui->cbCondition->addItem("Above Altitude",ENDCONDITION_ABOVEALTITUDE);
    ui->cbCondition->addItem("Pointing towards next",ENDCONDITION_POINTINGTOWARDSNEXT);
    ui->cbCondition->addItem("Python script",ENDCONDITION_PYTHONSCRIPT);
    ui->cbCondition->addItem("Immediate",ENDCONDITION_IMMEDIATE);

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

    my_waypoint = NULL;

    close();
}

void opmap_edit_waypoint_dialog::on_pushButtonApply_clicked()
{
    saveSettings();
}

void opmap_edit_waypoint_dialog::on_pushButtonRevert_clicked()
{
    loadFromWP(my_waypoint);
}

void opmap_edit_waypoint_dialog::setupPositionWidgets(bool isRelative)
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

void opmap_edit_waypoint_dialog::setupModeWidgets()
{
    ModeOptions mode=(ModeOptions)ui->cbMode->itemData(ui->cbMode->currentIndex()).toInt();
    switch(mode)
    {
    case MODE_FLYENDPOINT:
    case MODE_FLYVECTOR:
    case MODE_FLYCIRCLERIGHT:
    case MODE_FLYCIRCLELEFT:
    case MODE_DRIVEENDPOINT:
    case MODE_DRIVEVECTOR:
    case MODE_DRIVECIRCLELEFT:
    case MODE_DRIVECIRCLERIGHT:
    case MODE_DISARMALARM:
        ui->modeParam1->setVisible(false);
        ui->modeParam2->setVisible(false);
        ui->modeParam3->setVisible(false);
        ui->modeParam4->setVisible(false);
        ui->dsb_modeParam1->setVisible(false);
        ui->dsb_modeParam2->setVisible(false);
        ui->dsb_modeParam3->setVisible(false);
        ui->dsb_modeParam4->setVisible(false);
        break;
    case MODE_FIXEDATTITUDE:
        ui->modeParam1->setText("pitch");
        ui->modeParam2->setText("roll");
        ui->modeParam3->setText("yaw");
        ui->modeParam4->setText("throttle");
        ui->modeParam1->setVisible(true);
        ui->modeParam2->setVisible(true);
        ui->modeParam3->setVisible(true);
        ui->modeParam4->setVisible(true);
        ui->dsb_modeParam1->setVisible(true);
        ui->dsb_modeParam2->setVisible(true);
        ui->dsb_modeParam3->setVisible(true);
        ui->dsb_modeParam4->setVisible(true);
        break;
    case MODE_SETACCESSORY:
        ui->modeParam1->setText("Acc.channel");
        ui->modeParam2->setText("Value");
        ui->modeParam1->setVisible(true);
        ui->modeParam2->setVisible(true);
        ui->modeParam3->setVisible(false);
        ui->modeParam4->setVisible(false);
        ui->dsb_modeParam1->setVisible(true);
        ui->dsb_modeParam2->setVisible(true);
        ui->dsb_modeParam3->setVisible(false);
        ui->dsb_modeParam4->setVisible(false);
         break;
    }
}
void opmap_edit_waypoint_dialog::setupConditionWidgets()
{
    EndConditionOptions mode=(EndConditionOptions)ui->cbCondition->itemData(ui->cbCondition->currentIndex()).toInt();
    switch(mode)
    {
    case ENDCONDITION_NONE:
    case ENDCONDITION_IMMEDIATE:
    case ENDCONDITION_PYTHONSCRIPT:
        ui->condParam1->setVisible(false);
        ui->condParam2->setVisible(false);
        ui->condParam3->setVisible(false);
        ui->condParam4->setVisible(false);
        ui->dsb_condParam1->setVisible(false);
        ui->dsb_condParam2->setVisible(false);
        ui->dsb_condParam3->setVisible(false);
        ui->dsb_condParam4->setVisible(false);
        break;
    case ENDCONDITION_TIMEOUT:
        ui->condParam1->setVisible(true);
        ui->condParam2->setVisible(false);
        ui->condParam3->setVisible(false);
        ui->condParam4->setVisible(false);
        ui->dsb_condParam1->setVisible(true);
        ui->dsb_condParam2->setVisible(false);
        ui->dsb_condParam3->setVisible(false);
        ui->dsb_condParam4->setVisible(false);
        ui->condParam1->setText("Timeout(ms)");
        break;
    case ENDCONDITION_DISTANCETOTARGET:
        ui->condParam1->setVisible(true);
        ui->condParam2->setVisible(true);
        ui->condParam3->setVisible(false);
        ui->condParam4->setVisible(false);
        ui->dsb_condParam1->setVisible(true);
        ui->dsb_condParam2->setVisible(true);
        ui->dsb_condParam3->setVisible(false);
        ui->dsb_condParam4->setVisible(false);
        ui->condParam1->setText("Distance(m)");
        ui->condParam2->setText("Flag(0=2D,1=3D)");//FIXME
        break;
    case ENDCONDITION_LEGREMAINING:
        ui->condParam1->setVisible(true);
        ui->condParam2->setVisible(false);
        ui->condParam3->setVisible(false);
        ui->condParam4->setVisible(false);
        ui->dsb_condParam1->setVisible(true);
        ui->dsb_condParam2->setVisible(false);
        ui->dsb_condParam3->setVisible(false);
        ui->dsb_condParam4->setVisible(false);
        ui->condParam1->setText("Relative Distance(0=complete,1=just starting)");
        break;
    case ENDCONDITION_ABOVEALTITUDE:
        ui->condParam1->setVisible(true);
        ui->condParam2->setVisible(false);
        ui->condParam3->setVisible(false);
        ui->condParam4->setVisible(false);
        ui->dsb_condParam1->setVisible(true);
        ui->dsb_condParam2->setVisible(false);
        ui->dsb_condParam3->setVisible(false);
        ui->dsb_condParam4->setVisible(false);
        ui->condParam1->setText("Altitude in meters (negative)");
        break;
    case ENDCONDITION_POINTINGTOWARDSNEXT:
        ui->condParam1->setVisible(true);
        ui->condParam2->setVisible(false);
        ui->condParam3->setVisible(false);
        ui->condParam4->setVisible(false);
        ui->dsb_condParam1->setVisible(true);
        ui->dsb_condParam2->setVisible(false);
        ui->dsb_condParam3->setVisible(false);
        ui->dsb_condParam4->setVisible(false);
        ui->condParam1->setText("Degrees variation allowed");
        break;
    default:

        break;
    }
}
void opmap_edit_waypoint_dialog::on_pushButtonCancel_clicked()
{
    my_waypoint = NULL;
    close();
}

int opmap_edit_waypoint_dialog::saveSettings()
{
    int number = ui->spinBoxNumber->value();
    if (number < 0)
    {
	return -1;
    }
    customData data;
    data.mode=ui->cbMode->itemData(ui->cbMode->currentIndex()).toInt();
    data.mode_params[0]=ui->dsb_modeParam1->value();
    data.mode_params[1]=ui->dsb_modeParam2->value();
    data.mode_params[2]=ui->dsb_modeParam3->value();
    data.mode_params[3]=ui->dsb_modeParam4->value();

    data.condition=ui->cbCondition->itemData(ui->cbCondition->currentIndex()).toInt();
    data.condition_params[0]=ui->dsb_condParam1->value();
    data.condition_params[1]=ui->dsb_condParam2->value();
    data.condition_params[2]=ui->dsb_condParam3->value();
    data.condition_params[3]=ui->dsb_condParam4->value();

    QVariant var;
    var.setValue(data);
    my_waypoint->setData(0,var);

    my_waypoint->SetNumber(ui->spinBoxNumber->value());
    my_waypoint->SetCoord(internals::PointLatLng(ui->doubleSpinBoxLatitude->value(), ui->doubleSpinBoxLongitude->value()));
    my_waypoint->SetAltitude(ui->doubleSpinBoxAltitude->value());
    my_waypoint->SetDescription(ui->lineEditDescription->displayText().simplified());
    my_waypoint->setFlag(QGraphicsItem::ItemIsMovable, !ui->checkBoxLocked->isChecked());
    if(ui->rbAbsolute->isChecked())
        my_waypoint->setWPType(mapcontrol::WayPointItem::absolute);
    else
        my_waypoint->setWPType(mapcontrol::WayPointItem::relative);
    mapcontrol::distBearing pt;
    pt.distance=ui->spinBoxDistance->value();
    pt.bearing=ui->doubleSpinBoxBearing->value()/180*M_PI;
    my_waypoint->setRelativeCoord(pt);
    return 0;	// all ok
}

void opmap_edit_waypoint_dialog::loadFromWP(mapcontrol::WayPointItem *waypoint_item)
{
    customData data=waypoint_item->data(0).value<customData>();
    ui->spinBoxNumber->setValue(waypoint_item->Number());
    ui->doubleSpinBoxLatitude->setValue(waypoint_item->Coord().Lat());
    ui->doubleSpinBoxLongitude->setValue(waypoint_item->Coord().Lng());
    ui->doubleSpinBoxAltitude->setValue(waypoint_item->Altitude());
    ui->lineEditDescription->setText(waypoint_item->Description());
    if(waypoint_item->WPType()==mapcontrol::WayPointItem::absolute)
        ui->rbAbsolute->setChecked(true);
    else
        ui->rbRelative->setChecked(true);
    ui->doubleSpinBoxBearing->setValue(waypoint_item->getRelativeCoord().bearing*180/M_PI);
    ui->spinBoxDistance->setValue(waypoint_item->getRelativeCoord().distance);

    ui->cbMode->setCurrentIndex(ui->cbMode->findData(data.mode));
    ui->dsb_modeParam1->setValue(data.mode_params[0]);
    ui->dsb_modeParam2->setValue(data.mode_params[1]);
    ui->dsb_modeParam3->setValue(data.mode_params[2]);
    ui->dsb_modeParam4->setValue(data.mode_params[3]);

    ui->cbCondition->setCurrentIndex(ui->cbCondition->findData(data.condition));
    ui->dsb_condParam1->setValue(data.condition_params[0]);
    ui->dsb_condParam2->setValue(data.condition_params[1]);
    ui->dsb_condParam3->setValue(data.condition_params[2]);
    ui->dsb_condParam4->setValue(data.condition_params[3]);
}

void opmap_edit_waypoint_dialog::editWaypoint(mapcontrol::WayPointItem *waypoint_item)
{
    if (!waypoint_item) return;

    this->my_waypoint = waypoint_item;
    loadFromWP(waypoint_item);
    setupPositionWidgets(ui->rbRelative->isChecked());
    show();
}

// *********************************************************************
