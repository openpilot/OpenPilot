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

#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/positionactual.h"

namespace Ui {
    class opmap_waypointeditor_dialog;
}

// ***************************************************************

class Waypoint : public QObject, public QGraphicsItem
{
    Q_OBJECT

 public:
    Waypoint();

    double latitude_degress;
    double longitude_degress;
    double height_feet;
    double relative_time_seconds;

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

 protected:
//    void timerEvent(QTimerEvent *event);

 private:
//    qreal angle;
//    qreal speed;
//    qreal mouseEyeDirection;
//    QColor color;
};

// ***************************************************************

class opmap_waypointeditor_dialog : public QDialog
{
    Q_OBJECT
public:
    opmap_waypointeditor_dialog(QWidget *parent = 0);
    ~opmap_waypointeditor_dialog();

protected:
    void changeEvent(QEvent *e);

private:
    QGraphicsView *view;
    QGraphicsScene *scene;

    Ui::opmap_waypointeditor_dialog *ui;
};

// ***************************************************************

#endif // OPMAP_WAYPOINTEDITOR_DIALOG_H
