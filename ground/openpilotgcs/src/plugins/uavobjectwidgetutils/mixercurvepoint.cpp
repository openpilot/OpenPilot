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
    cmdActive = false;
    vertical = false;
    cmdNode = false;
    cmdToggle = true;    
    drawNode = true;
    drawText = true;

    posColor0 = "#1c870b";  //greenish?
    posColor1 = "#116703";  //greenish?
    negColor0 = "#aa0000";  //red
    negColor1 = "#aa0000";  //red
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
    return cmdNode ? QRectF(-4, -4, 15, 10) : QRectF(-13, -13, 26, 26);
}

QPainterPath MixerNode::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void MixerNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QString text = cmdNode ? cmdText : QString().sprintf("%.2f", value());
    painter->setFont(graph->font());
    if (drawNode) {
        QRadialGradient gradient(-3, -3, 10);
        if (option->state & QStyle::State_Sunken) {
            gradient.setCenter(3, 3);
            gradient.setFocalPoint(3, 3);

            gradient.setColorAt(1, Qt::darkBlue);
            gradient.setColorAt(0, Qt::darkBlue);
        } else {
            if (cmdNode) {
                gradient.setColorAt(0, cmdActive ? posColor0 : negColor0);
                gradient.setColorAt(1, cmdActive ? posColor1 : negColor1);
            }
            else {
                if (value() < 0) {
                    gradient.setColorAt(0, negColor0);
                    gradient.setColorAt(1, negColor1);
                }
                else {
                    gradient.setColorAt(0, posColor0);
                    gradient.setColorAt(1, posColor1);
                }
            }
        }
        painter->setBrush(gradient);
        painter->setPen(QPen(Qt::black, 0));
        painter->drawEllipse(boundingRect());

        if (!image.isNull())
            painter->drawImage(boundingRect().adjusted(1,1,-1,-1), image);
    }

    if (drawText) {
        painter->setPen(QPen(drawNode ? Qt::white : Qt::black, 0));
        if (cmdNode) {
            painter->drawText(0,4,text);
        }
        else {
            painter->drawText( (value() < 0) ? -13 : -11, 4, text);
        }
    }
}

void MixerNode::verticalMove(bool flag){
    vertical = flag;
}

void MixerNode::commandNode(bool enable){
    cmdNode = enable;
}
void MixerNode::commandText(QString text){
    cmdText = text;
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
    if (cmdNode) {
        graph->cmdActivated(this);
        //return;
    }
    update();
    QGraphicsItem::mousePressEvent(event);
}

void MixerNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}
