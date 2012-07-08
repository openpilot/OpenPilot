/**
 ******************************************************************************
 *
 * @file       mixercurvepoint.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectWidgetUtils Plugin
 * @{
 * @brief Utility plugin for UAVObject to Widget relation management
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

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

#include "mixercurveline.h"
#include "mixercurvepoint.h"
#include "mixercurvewidget.h"

Node::Node(MixerCurveWidget *graphWidget)
    : graph(graphWidget)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    vertical = false;
}

void Node::addEdge(Edge *edge)
{
    edgeList << edge;
    edge->adjust();
}

QList<Edge *> Node::edges() const
{
    return edgeList;
}


QRectF Node::boundingRect() const
{
    return QRectF(-13, -13, 26, 26);
}

QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(-13, -13, 26, 26);
    return path;
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    /*
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(-7, -7, 20, 20);
    */

    QRadialGradient gradient(-3, -3, 10);
    if (option->state & QStyle::State_Sunken) {
        gradient.setCenter(3, 3);
        gradient.setFocalPoint(3, 3);
        gradient.setColorAt(1, QColor("#1c870b").light(120));
        gradient.setColorAt(0, QColor("#116703").light(120));
    } else {
        gradient.setColorAt(0, "#1c870b");
        gradient.setColorAt(1, "#116703");
    }
    painter->setBrush(gradient);
    painter->setPen(QPen(Qt::black, 0));
    painter->drawEllipse(-13, -13, 26, 26);

    if (value() < 0) {
        painter->setPen(QPen(Qt::red, 0));
        painter->drawText(-12, 4, QString().sprintf("%.2f", value()));
    }
    else {
        painter->setPen(QPen(Qt::white, 0));
        painter->drawText(-11, 4, QString().sprintf("%.2f", value()));
    }
}

void Node::verticalMove(bool flag){
    vertical = flag;
}

double Node::value() {
    double h = graph->sceneRect().height();
    double ratio = (h - pos().y()) / h;
    return ((graph->getMax() - graph->getMin()) * ratio ) + graph->getMin();
}


QVariant Node::itemChange(GraphicsItemChange change, const QVariant &val)
{
    QPointF newPos = val.toPointF();
    double h = graph->sceneRect().height();

    switch (change) {
    case ItemPositionChange: {

        if (!vertical)
                break;

        // Force node to move vertically
        newPos.setX(pos().x());

        // Stay inside graph
        if (newPos.y() < 0)
            newPos.setY(0);
        //qDebug() << h << " - " << newPos.y();
        if (newPos.y() > h)
            newPos.setY(h);

          return newPos;
    }
    case ItemPositionHasChanged: {
        foreach (Edge *edge, edgeList)
            edge->adjust();

        update();

        graph->itemMoved(value());
        break;
    }
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, val);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mousePressEvent(event);
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}
