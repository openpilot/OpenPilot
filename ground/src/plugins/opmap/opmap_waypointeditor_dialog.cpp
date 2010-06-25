/**
 ******************************************************************************
 *
 * @file       opmap_waypointeditor_dialog.cpp
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

#include <QPainter>

#include "opmap_waypointeditor_dialog.h"
#include "ui_opmap_waypointeditor_dialog.h"

#include "extensionsystem/pluginmanager.h"

// ***************************************************************

Waypoint::Waypoint()
{
}

QRectF Waypoint::boundingRect() const
{
    return QRectF(-6, -10, 12, 20);
}

QPainterPath Waypoint::shape() const
{
    QPainterPath path;
    path.addEllipse(QPointF(0, 0), 6, 10);
    return path;
}

 void Waypoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(Qt::black);
    painter->setBrush(QColor(255, 0, 0, 128));
    painter->drawEllipse(QPointF(0, 0), 6, 10);
 }

// ***************************************************************

opmap_waypointeditor_dialog::opmap_waypointeditor_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::opmap_waypointeditor_dialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog);

    view = ui->graphicsViewWaypointHeightAndTimeline;

    scene = new QGraphicsScene(0, 0, 1800, 1000);
    view->setScene(scene);

    view->setRenderHint(QPainter::HighQualityAntialiasing);
    //view->setDragMode(QGraphicsView::ScrollHandDrag);

    // *****
    // test

    Waypoint *waypoint1 = new Waypoint;
    waypoint1->setPos(scene->width() / 2, scene->height() / 2);
    waypoint1->setFlag(QGraphicsItem::ItemIsMovable, true);
    waypoint1->setFlag(QGraphicsItem::ItemIsSelectable, true);
    waypoint1->setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    waypoint1->setToolTip(tr("Waypoint 1"));
    scene->addItem(waypoint1);

    Waypoint *waypoint2 = new Waypoint;
    waypoint2->setPos(scene->width() / 2 + 30, scene->height() / 2);
    waypoint2->setFlag(QGraphicsItem::ItemIsMovable);
    waypoint2->setFlag(QGraphicsItem::ItemIsSelectable, true);
    waypoint2->setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    waypoint2->setToolTip(tr("Waypoint 2"));
    scene->addItem(waypoint2);

    Waypoint *waypoint3 = new Waypoint;
    waypoint3->setPos(scene->width() / 2 + 60, scene->height() / 2);
    waypoint3->setFlag(QGraphicsItem::ItemIsMovable);
    waypoint3->setFlag(QGraphicsItem::ItemIsSelectable, true);
    waypoint3->setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    waypoint3->setToolTip(tr("Waypoint 3"));
    scene->addItem(waypoint3);

    // *****
}

opmap_waypointeditor_dialog::~opmap_waypointeditor_dialog()
{
    delete ui;
}

void opmap_waypointeditor_dialog::changeEvent(QEvent *e)
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
