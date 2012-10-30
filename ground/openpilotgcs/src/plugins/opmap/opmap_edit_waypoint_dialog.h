/**
 ******************************************************************************
 *
 * @file       opmap_edit_waypoint_dialog.h
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

#ifndef OPMAP_EDIT_WAYPOINT_DIALOG_H
#define OPMAP_EDIT_WAYPOINT_DIALOG_H

#include <QDialog>
#include <QDataWidgetMapper>
#include "opmapcontrol/opmapcontrol.h"
#include "flightdatamodel.h"
namespace Ui {
    class opmap_edit_waypoint_dialog;
}
using namespace mapcontrol;

class opmap_edit_waypoint_dialog : public QWidget
{
    Q_OBJECT
public:
    opmap_edit_waypoint_dialog(QWidget *parent,QAbstractItemModel * model,QItemSelectionModel * selection);
    ~opmap_edit_waypoint_dialog();

    /**
    * @brief public functions
    *
    * @param
    */
    void editWaypoint(mapcontrol::WayPointItem *waypoint_item);

private:
    Ui::opmap_edit_waypoint_dialog *ui;
    QDataWidgetMapper *mapper;
    QAbstractItemModel * model;
    QItemSelectionModel * itemSelection;
private slots:

private slots:
    void currentIndexChanged(int index);
    void setupModeWidgets();
    void setupConditionWidgets();
    void pushButtonCancel_clicked();
    void on_pushButtonOK_clicked();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void enableEditWidgets(bool);
    void currentRowChanged(QModelIndex,QModelIndex);
};

#endif // OPMAP_EDIT_WAYPOINT_DIALOG_H
