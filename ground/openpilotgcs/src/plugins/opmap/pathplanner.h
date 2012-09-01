/**
 ******************************************************************************
 *
 * @file       pathplanner.h
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

#ifndef PATHPLANNER_H
#define PATHPLANNER_H

#include <QWidget>
#include "flightdatamodel.h"
#include "opmap_edit_waypoint_dialog.h"
namespace Ui {
class pathPlannerUI;
}

class pathPlanner : public QWidget
{
    Q_OBJECT
    
public:
    explicit pathPlanner(QWidget *parent = 0);
    ~pathPlanner();
    
    void setModel(flightDataModel *model,QItemSelectionModel *selection);
private slots:
        void rowsInserted ( const QModelIndex & parent, int start, int end );

        void on_tbAdd_clicked();

        void on_tbDelete_clicked();

        void on_tbInsert_clicked();

        void on_tbReadFromFile_clicked();

        void on_tbSaveToFile_clicked();

        void on_tbDetails_clicked();

        void on_tbSendToUAV_clicked();

        void on_tbFetchFromUAV_clicked();

private:
    Ui::pathPlannerUI *ui;
    opmap_edit_waypoint_dialog * wid;
    flightDataModel * myModel;
signals:
    void sendPathPlanToUAV();
    void receivePathPlanFromUAV();
};

#endif // PATHPLANNER_H
