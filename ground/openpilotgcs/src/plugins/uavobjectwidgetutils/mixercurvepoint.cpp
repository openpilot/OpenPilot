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
#include <QColor>
#include <QDebug>

#include "mixercurveline.h"
#include "mixercurvepoint.h"
#include "mixercurvewidget.h"

MixerNode::MixerNode(MixerCurveWidget *graphWidget)
    : graph(graphWidget)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    vertical = false;
    drawNode = true;
    drawText = true;

    positiveColor = "#609FF2";  //blueish?
    neutralColor = "#14CE24";  //greenish?
    negativeColor = "#EF5F5F";  //redish?
    disabledColor = "#dddddd";
    disabledTextColor = "#aaaaaa";
}

void MixerNode::addEdge(Edge *edge)
{
    edgeList << edge;
    edge->adjust();
}

QList<Edge *> MixerNode::edges() const
{
    return edgeList;
}


QRectF MixerNode::boundingRect() const
{
    return QRectF(-13, -13, 26, 26);
}

QPainterPath MixerNode::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void MixerNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QString text = QString().sprintf("%.2f", value());
    painter->setFont(graph->font());
    if (drawNode) {
        QRadialGradient gradient(-3, -3, 10);

        QColor color;
        if (value() < 0) {
            color = negativeColor;
        }
        else if (value() == 0) {
            color = neutralColor;
        }
        else {
            color = positiveColor;
        }

        if (option->state & QStyle::State_Sunken) {
            gradient.setCenter(3, 3);
            gradient.setFocalPoint(3, 3);

            QColor selColor = color.darker();
            gradient.setColorAt(1, selColor.darker());
            gradient.setColorAt(0, selColor);
        } else {
            gradient.setColorAt(0, graph->isEnabled() ? color : disabledColor);
            gradient.setColorAt(1, graph->isEnabled() ? color.darker() : disabledColor);
        }
        painter->setBrush(gradient);
        painter->setPen(graph->isEnabled() ? QPen(Qt::black, 0) : QPen(disabledTextColor));
        painter->drawEllipse(boundingRect());

        if (!image.isNull()) {
            painter->drawImage(boundingRect().adjusted(1, 1, -1, -1), image);
        }
    }

    if (drawText) {
        if(graph->isEnabled()) {
            painter->setPen(QPen(drawNode ? Qt::white : Qt::black, 0));
        } else {
            painter->setPen(QPen(disabledTextColor));
        }

        painter->drawText( (value() < 0) ? -10 : -8, 3, text);
    }
}

void MixerNode::verticalMove(bool flag){
    vertical = flag;
}

double MixerNode::value() {
    double h = graph->sceneRect().height();
    double ratio = (h - pos().y()) / h;
    return ((graph->getMax() - graph->getMin()) * ratio ) + graph->getMin();
}


QVariant MixerNode::itemChange(GraphicsItemChange change, const QVariant &val)
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

void MixerNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mousePressEvent(event);
}

void MixerNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}
