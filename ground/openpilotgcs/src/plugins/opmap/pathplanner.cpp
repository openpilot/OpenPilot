/**
 ******************************************************************************
 *
 * @file       pathplanner.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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

#include "pathplanner.h"
#include "ui_pathplanner.h"
#include "widgetdelegates.h"
#include <QAbstractItemModel>
#include <QFileDialog>

pathPlanner::pathPlanner(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::pathPlannerUI),wid(NULL),myModel(NULL)
{
    ui->setupUi(this);
}

pathPlanner::~pathPlanner()
{
    delete ui;
    if(wid)
        delete wid;
}
void pathPlanner::setModel(flightDataModel *model,QItemSelectionModel *selection)
{
    myModel=model;
    ui->tableView->setModel(model);
    ui->tableView->setSelectionModel(selection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setItemDelegate(new MapDataDelegate(this));
    connect(model,SIGNAL(rowsInserted(const QModelIndex&,int,int)),this,SLOT(rowsInserted(const QModelIndex&,int,int)));
    wid=new opmap_edit_waypoint_dialog(NULL,model,selection);
    ui->tableView->resizeColumnsToContents();
}

void pathPlanner::rowsInserted ( const QModelIndex & parent, int start, int end )
{
    Q_UNUSED(parent);
    for(int x=start;x<end+1;x++)
    {
        QModelIndex index=ui->tableView->model()->index(x,flightDataModel::MODE);
        ui->tableView->openPersistentEditor(index);
        index=ui->tableView->model()->index(x,flightDataModel::CONDITION);
        ui->tableView->openPersistentEditor(index);
        index=ui->tableView->model()->index(x,flightDataModel::COMMAND);
        ui->tableView->openPersistentEditor(index);
        ui->tableView->size().setHeight(10);
    }
}

void pathPlanner::on_tbAdd_clicked()
{
    ui->tableView->model()->insertRow(ui->tableView->model()->rowCount());
}

void pathPlanner::on_tbDelete_clicked()
{
    ui->tableView->model()->removeRow(ui->tableView->selectionModel()->currentIndex().row());
}

void pathPlanner::on_tbInsert_clicked()
{
    ui->tableView->model()->insertRow(ui->tableView->selectionModel()->currentIndex().row());
}

void pathPlanner::on_tbReadFromFile_clicked()
{
    if(!myModel)
        return;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
    myModel->readFromFile(fileName);
}

void pathPlanner::on_tbSaveToFile_clicked()
{
    if(!myModel)
        return;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));
    myModel->writeToFile(fileName);
}

void pathPlanner::on_tbDetails_clicked()
{
    if(wid)
       wid->show();
}

void pathPlanner::on_tbSendToUAV_clicked()
{
    emit sendPathPlanToUAV();
}

void pathPlanner::on_tbFetchFromUAV_clicked()
{
    emit receivePathPlanFromUAV();
}
