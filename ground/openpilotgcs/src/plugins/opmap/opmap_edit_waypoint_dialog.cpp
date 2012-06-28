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
#include "widgetdelegates.h"
// *********************************************************************

// constructor
opmap_edit_waypoint_dialog::opmap_edit_waypoint_dialog(QWidget *parent,QAbstractItemModel * model,QItemSelectionModel * selection) :
    QWidget(parent),model(model),itemSelection(selection),
    ui(new Ui::opmap_edit_waypoint_dialog)
{  
    ui->setupUi(this);
    my_waypoint = NULL;
    connect(ui->checkBoxLocked,SIGNAL(toggled(bool)),this,SLOT(enableEditWidgets(bool)));
    connect(ui->cbMode,SIGNAL(currentIndexChanged(int)),this,SLOT(setupModeWidgets()));
    connect(ui->cbCondition,SIGNAL(currentIndexChanged(int)),this,SLOT(setupConditionWidgets()));

    mapDataDelegate::loadComboBox(ui->cbMode,flightDataModel::MODE);
    mapDataDelegate::loadComboBox(ui->cbCondition,flightDataModel::CONDITION);
    mapDataDelegate::loadComboBox(ui->cbCommand,flightDataModel::COMMAND);
    mapper = new QDataWidgetMapper(this);

    mapper->setItemDelegate(new mapDataDelegate(this));
    connect(mapper,SIGNAL(currentIndexChanged(int)),this,SLOT(currentIndexChanged(int)));
    mapper->setModel(model);
    mapper->addMapping(ui->checkBoxLocked,flightDataModel::LOCKED);
    mapper->addMapping(ui->doubleSpinBoxLatitude,flightDataModel::LATPOSITION);
    mapper->addMapping(ui->doubleSpinBoxLongitude,flightDataModel::LNGPOSITION);
    mapper->addMapping(ui->doubleSpinBoxAltitude,flightDataModel::ALTITUDE);
    mapper->addMapping(ui->lineEditDescription,flightDataModel::WPDESCRITPTION);
    mapper->addMapping(ui->checkBoxRelative,flightDataModel::ISRELATIVE);
    mapper->addMapping(ui->doubleSpinBoxBearing,flightDataModel::BEARELATIVE);
    mapper->addMapping(ui->doubleSpinBoxVelocity,flightDataModel::VELOCITY);
    mapper->addMapping(ui->doubleSpinBoxDistance,flightDataModel::DISRELATIVE);
    mapper->addMapping(ui->doubleSpinBoxRelativeAltitude,flightDataModel::ALTITUDERELATIVE);
    mapper->addMapping(ui->cbMode,flightDataModel::MODE);
    mapper->addMapping(ui->dsb_modeParam1,flightDataModel::MODE_PARAMS0);
    mapper->addMapping(ui->dsb_modeParam2,flightDataModel::MODE_PARAMS1);
    mapper->addMapping(ui->dsb_modeParam3,flightDataModel::MODE_PARAMS2);
    mapper->addMapping(ui->dsb_modeParam4,flightDataModel::MODE_PARAMS3);

    mapper->addMapping(ui->cbCondition,flightDataModel::CONDITION);
    mapper->addMapping(ui->dsb_condParam1,flightDataModel::CONDITION_PARAMS0);
    mapper->addMapping(ui->dsb_condParam2,flightDataModel::CONDITION_PARAMS1);
    mapper->addMapping(ui->dsb_condParam3,flightDataModel::CONDITION_PARAMS2);
    mapper->addMapping(ui->dsb_condParam4,flightDataModel::CONDITION_PARAMS0);

    mapper->addMapping(ui->cbCommand,flightDataModel::COMMAND);
    mapper->addMapping(ui->sbJump,flightDataModel::JUMPDESTINATION);
    mapper->addMapping(ui->sbError,flightDataModel::ERRORDESTINATION);
    connect(itemSelection,SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(currentRowChanged(QModelIndex,QModelIndex)));
}
void opmap_edit_waypoint_dialog::currentIndexChanged(int index)
{
    ui->lbNumber->setText(QString::number(index));
    QModelIndex idx=mapper->model()->index(index,0);
    if(index==itemSelection->currentIndex().row())
        return;
    itemSelection->clear();
    itemSelection->setCurrentIndex(idx,QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

opmap_edit_waypoint_dialog::~opmap_edit_waypoint_dialog()
{
    delete ui;
}

void opmap_edit_waypoint_dialog::on_pushButtonOK_clicked()
{
    close();
}

void opmap_edit_waypoint_dialog::setupModeWidgets()
{
    mapDataDelegate::ModeOptions mode=(mapDataDelegate::ModeOptions)ui->cbMode->itemData(ui->cbMode->currentIndex()).toInt();
    switch(mode)
    {
    case mapDataDelegate::MODE_FLYENDPOINT:
    case mapDataDelegate::MODE_FLYVECTOR:
    case mapDataDelegate::MODE_FLYCIRCLERIGHT:
    case mapDataDelegate::MODE_FLYCIRCLELEFT:
    case mapDataDelegate::MODE_DRIVEENDPOINT:
    case mapDataDelegate::MODE_DRIVEVECTOR:
    case mapDataDelegate::MODE_DRIVECIRCLELEFT:
    case mapDataDelegate::MODE_DRIVECIRCLERIGHT:
    case mapDataDelegate::MODE_DISARMALARM:
        ui->modeParam1->setVisible(false);
        ui->modeParam2->setVisible(false);
        ui->modeParam3->setVisible(false);
        ui->modeParam4->setVisible(false);
        ui->dsb_modeParam1->setVisible(false);
        ui->dsb_modeParam2->setVisible(false);
        ui->dsb_modeParam3->setVisible(false);
        ui->dsb_modeParam4->setVisible(false);
        break;
    case mapDataDelegate::MODE_FIXEDATTITUDE:
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
    case mapDataDelegate::MODE_SETACCESSORY:
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
    mapDataDelegate::EndConditionOptions mode=(mapDataDelegate::EndConditionOptions)ui->cbCondition->itemData(ui->cbCondition->currentIndex()).toInt();
    switch(mode)
    {
    case mapDataDelegate::ENDCONDITION_NONE:
    case mapDataDelegate::ENDCONDITION_IMMEDIATE:
    case mapDataDelegate::ENDCONDITION_PYTHONSCRIPT:
        ui->condParam1->setVisible(false);
        ui->condParam2->setVisible(false);
        ui->condParam3->setVisible(false);
        ui->condParam4->setVisible(false);
        ui->dsb_condParam1->setVisible(false);
        ui->dsb_condParam2->setVisible(false);
        ui->dsb_condParam3->setVisible(false);
        ui->dsb_condParam4->setVisible(false);
        break;
    case mapDataDelegate::ENDCONDITION_TIMEOUT:
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
    case mapDataDelegate::ENDCONDITION_DISTANCETOTARGET:
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
    case mapDataDelegate::ENDCONDITION_LEGREMAINING:
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
    case mapDataDelegate::ENDCONDITION_ABOVEALTITUDE:
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
    case mapDataDelegate::ENDCONDITION_POINTINGTOWARDSNEXT:
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
void opmap_edit_waypoint_dialog::pushButtonCancel_clicked()
{
    my_waypoint = NULL;
    close();
}

void opmap_edit_waypoint_dialog::editWaypoint(mapcontrol::WayPointItem *waypoint_item)
{
    if (!waypoint_item) return;
    show();
    mapper->setCurrentIndex(waypoint_item->Number());
}

void opmap_edit_waypoint_dialog::on_pushButton_clicked()
{
    mapper->toPrevious();
}

void opmap_edit_waypoint_dialog::on_pushButton_2_clicked()
{
    mapper->toNext();
}

void opmap_edit_waypoint_dialog::enableEditWidgets(bool value)
{
    QWidget * w;
    foreach(QWidget * obj,this->findChildren<QWidget *>())
    {
        w=qobject_cast<QComboBox*>(obj);
        if(w)
            w->setEnabled(!value);
        w=qobject_cast<QLineEdit*>(obj);
        if(w)
            w->setEnabled(!value);
        w=qobject_cast<QDoubleSpinBox*>(obj);
        if(w)
            w->setEnabled(!value);
        w=qobject_cast<QCheckBox*>(obj);
        if(w && w!=ui->checkBoxLocked)
            w->setEnabled(!value);
        w=qobject_cast<QSpinBox*>(obj);
        if(w)
            w->setEnabled(!value);
    }
}

void opmap_edit_waypoint_dialog::currentRowChanged(QModelIndex current, QModelIndex previous)
{
    mapper->setCurrentIndex(current.row());
}
