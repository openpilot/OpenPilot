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
#include <QGraphicsSceneDragDropEvent>

#include "opmap_waypointeditor_dialog.h"
#include "ui_opmap_waypointeditor_dialog.h"

#include "extensionsystem/pluginmanager.h"

// ***************************************************************
// Waypoint object

WaypointItem::WaypointItem(QString name, double latitude, double longitude, double height, int time, int hold) :
    waypoint_name(name),
    latitude_degress(latitude),
    longitude_degress(longitude),
    height_feet(height),
    time_seconds(time),
    hold_seconds(hold)
{
    setToolTip(waypoint_name);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

    pixmap.load(QString::fromUtf8(":/opmap/images/waypoint_marker1.png"));
}

QRectF WaypointItem::boundingRect() const
{
//  return QRectF(-6, -10, 12, 20);
    return QRectF(-pixmap.width() / 2, -pixmap.height(), pixmap.width(), pixmap.height());
}
/*
QPainterPath WaypointItem::shape() const
{
    QPainterPath path;
//    path.addEllipse(QPointF(0, 0), 6, 10);
    path.addRect(pixmap.rect());
    return path;
}
*/
 void WaypointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
//  painter->setPen(Qt::black);
//  painter->setBrush(QColor(255, 0, 0, 128));
//  painter->drawEllipse(QPointF(0, 0), 6, 10);

    painter->drawPixmap(-pixmap.width() / 2, -pixmap.height(), pixmap);
}

void WaypointItem::setPixmap(QPixmap pixmap)
{
    this->pixmap = pixmap.copy(pixmap.rect());
}

// ***************************************************************
// Scene object

OurScene::OurScene(QObject *parent) : QGraphicsScene(parent)
{
    movingItem = 0;
}

void OurScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF mousePos(event->buttonDownScenePos(Qt::LeftButton).x(), event->buttonDownScenePos(Qt::LeftButton).y());

    movingItem = itemAt(mousePos.x(), mousePos.y());

    if (movingItem != 0 && event->button() == Qt::LeftButton)
    {
	oldPos = movingItem->pos();
    }

    clearSelection();

    QGraphicsScene::mousePressEvent(event);
 }

 void OurScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
 {
    if (movingItem != 0 && event->button() == Qt::LeftButton)
    {
	if (oldPos != movingItem->pos())
	    emit itemMoved(qgraphicsitem_cast<WaypointItem *>(movingItem), oldPos);

	movingItem = 0;
    }

    QGraphicsScene::mouseReleaseEvent(event);
 }

// ***************************************************************
// main dialogue

opmap_waypointeditor_dialog::opmap_waypointeditor_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::opmap_waypointeditor_dialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog);

    view = ui->graphicsViewWaypointHeightAndTimeline;

    scene = new OurScene();
    scene->setSceneRect(QRect(0, 0, 500, 500));
    view->setScene(scene);

    waypoint_pixmap1.load(QString::fromUtf8(":/opmap/images/waypoint_marker1.png"));
    waypoint_pixmap2.load(QString::fromUtf8(":/opmap/images/waypoint_marker2.png"));

    undoStack = new QUndoStack();

    connect(scene, SIGNAL(itemMoved(WaypointItem *, const QPointF &)), this, SLOT(itemMoved(WaypointItem *, const QPointF &)));

    // *****
    // test

    WaypointItem *waypoint1 = new WaypointItem(tr("Waypoint 1"), 0, 0, 10, 5, 10);
    waypoint1->setPos(scene->width() / 2, scene->height() / 2);
    scene->addItem(waypoint1);

    WaypointItem *waypoint2 = new WaypointItem(tr("Waypoint 2"), 0, 0, 50, 8, 5);
    waypoint2->setPos(scene->width() / 2 + 30, scene->height() / 2);
    scene->addItem(waypoint2);

    WaypointItem *waypoint3 = new WaypointItem(tr("Waypoint 3"), 0, 0, 100, 8, 5);
    waypoint3->setPixmap(waypoint_pixmap2);
    waypoint3->setPos(scene->width() / 2 + 60, scene->height() / 2);
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

void opmap_waypointeditor_dialog::itemMoved(WaypointItem *movedItem, const QPointF &oldPosition)
{
//     undoStack->push(new MoveCommand(movedItem, oldPosition));
}

// ***************************************************************
