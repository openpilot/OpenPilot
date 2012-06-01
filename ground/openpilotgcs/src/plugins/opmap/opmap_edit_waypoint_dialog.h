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

#ifndef OPMAP_EDIT_WAYPOINT_DIALOG_H
#define OPMAP_EDIT_WAYPOINT_DIALOG_H

#include <QDialog>

#include "opmapcontrol/opmapcontrol.h"

namespace Ui {
    class opmap_edit_waypoint_dialog;
}

class opmap_edit_waypoint_dialog : public QDialog
{
    Q_OBJECT
public:
    opmap_edit_waypoint_dialog(QWidget *parent = 0);
    ~opmap_edit_waypoint_dialog();

    /**
    * @brief public functions
    *
    * @param
    */
    void editWaypoint(mapcontrol::WayPointItem *waypoint_item);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::opmap_edit_waypoint_dialog *ui;

    int original_number;
    bool original_locked;
    internals::PointLatLng original_coord;
    double original_altitude;
    QString original_description;
    double original_distance;
    double original_bearing;


    mapcontrol::WayPointItem::wptype original_type;

    mapcontrol::WayPointItem *waypoint_item;

    int saveSettings();

private slots:

private slots:
    void setupWidgets(bool isRelative);
    void on_pushButtonCancel_clicked();
    void on_pushButtonRevert_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonOK_clicked();
};

#endif // OPMAP_EDIT_WAYPOINT_DIALOG_H
