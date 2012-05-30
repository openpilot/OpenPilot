/**
 ******************************************************************************
 * @file       pathactioneditorgadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup PathAction Editor GCS Plugins
 * @{
 * @addtogroup PathActionEditorGadgetPlugin PathAction Editor Gadget Plugin
 * @{
 * @brief A gadget to edit a list of pathactions
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

#ifndef PathActionEditorGADGETWIDGET_H_
#define PathActionEditorGADGETWIDGET_H_

#include <QtGui/QLabel>
#include <QtGui/QWidget>
#include <QtGui/QTreeView>
#include "pathaction.h"
#include "waypoint.h"
#include "pathactioneditortreemodel.h"

class Ui_PathActionEditor;

class PathActionEditorGadgetWidget : public QLabel
{
    Q_OBJECT

public:
    PathActionEditorGadgetWidget(QWidget *parent = 0);
    ~PathActionEditorGadgetWidget();

signals:

protected slots:
    void pathactionChanged(UAVObject *);
    void addPathActionInstance();
    void addWaypointInstance();

private:
    Ui_PathActionEditor * m_pathactioneditor;
    PathActionEditorTreeModel *m_model;
    PathAction *pathactionObj;
    Waypoint *waypointObj;
};

#endif /* PathActionEditorGADGETWIDGET_H_ */
