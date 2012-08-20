/**
 ******************************************************************************
 *
 * @file       widgetdelegates.h
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

#ifndef WIDGETDELEGATES_H
#define WIDGETDELEGATES_H
#include <QItemDelegate>
#include <QComboBox>
#include "flightdatamodel.h"


class MapDataDelegate : public QItemDelegate
 {
        Q_OBJECT

 public:
    typedef enum { MODE_FLYENDPOINT=0, MODE_FLYVECTOR=1, MODE_FLYCIRCLERIGHT=2, MODE_FLYCIRCLELEFT=3,
                   MODE_DRIVEENDPOINT=4, MODE_DRIVEVECTOR=5, MODE_DRIVECIRCLELEFT=6, MODE_DRIVECIRCLERIGHT=7,
                   MODE_FIXEDATTITUDE=8, MODE_SETACCESSORY=9, MODE_DISARMALARM=10 } ModeOptions;
    typedef enum { ENDCONDITION_NONE=0, ENDCONDITION_TIMEOUT=1, ENDCONDITION_DISTANCETOTARGET=2,
                   ENDCONDITION_LEGREMAINING=3, ENDCONDITION_ABOVEALTITUDE=4, ENDCONDITION_POINTINGTOWARDSNEXT=5,
                   ENDCONDITION_PYTHONSCRIPT=6, ENDCONDITION_IMMEDIATE=7 } EndConditionOptions;
    typedef enum { COMMAND_ONCONDITIONNEXTWAYPOINT=0, COMMAND_ONNOTCONDITIONNEXTWAYPOINT=1,
                   COMMAND_ONCONDITIONJUMPWAYPOINT=2, COMMAND_ONNOTCONDITIONJUMPWAYPOINT=3,
                   COMMAND_IFCONDITIONJUMPWAYPOINTELSENEXTWAYPOINT=4 } CommandOptions;

     MapDataDelegate(QObject *parent = 0);

     QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;

     void setEditorData(QWidget *editor, const QModelIndex &index) const;
     void setModelData(QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index) const;

     void updateEditorGeometry(QWidget *editor,
         const QStyleOptionViewItem &option, const QModelIndex &index) const;
     static void loadComboBox(QComboBox * combo,flightDataModel::pathPlanDataEnum type);
 };

#endif // WIDGETDELEGATES_H
