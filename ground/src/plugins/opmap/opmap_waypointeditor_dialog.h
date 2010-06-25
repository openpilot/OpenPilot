/**
 ******************************************************************************
 *
 * @file       opmap_waypointeditor_dialog.h
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

#ifndef OPMAP_WAYPOINTEDITOR_DIALOG_H
#define OPMAP_WAYPOINTEDITOR_DIALOG_H

#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QtGui/QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QUndoStack>

#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/positionactual.h"

namespace Ui {
    class opmap_waypointeditor_dialog;
}

// ***************************************************************
// Waypoint object

class WaypointItem : public QObject, public QGraphicsItem
{
    Q_OBJECT

 public:
    WaypointItem(QString name = "", double latitude = 0, double longitude = 0, double height_feet = 0, int time_seconds = 0, int hold_seconds = 0);

    QString waypoint_name;
    double latitude_degress;
    double longitude_degress;
    double height_feet;
    int time_seconds;
    int hold_seconds;

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

 protected:
//    void timerEvent(QTimerEvent *event);

 private:

};

// ***************************************************************

class OurScene : public QGraphicsScene
 {
     Q_OBJECT

 public:
     OurScene(QObject *parent = 0);

 signals:
     void itemMoved(WaypointItem *movedItem, const QPointF &movedFromPosition);

 protected:
     void mousePressEvent(QGraphicsSceneMouseEvent *event);
     void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

 private:
     QGraphicsItem *movingItem;
     QPointF oldPos;
 };

// ***************************************************************
// main dialog widget

class opmap_waypointeditor_dialog : public QDialog
{
    Q_OBJECT
public:
    opmap_waypointeditor_dialog(QWidget *parent = 0);
    ~opmap_waypointeditor_dialog();

 public slots:
     void itemMoved(WaypointItem *movedDiagram, const QPointF &moveStartPosition);

protected:
    void changeEvent(QEvent *e);

private:
    QGraphicsView *view;
    QGraphicsScene *scene;

    QUndoStack *undoStack;

    Ui::opmap_waypointeditor_dialog *ui;
};

// ***************************************************************

#endif // OPMAP_WAYPOINTEDITOR_DIALOG_H
