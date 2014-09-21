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
    : m_graph(graphWidget)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    m_vertical          = false;
    m_drawNode          = true;
    m_drawText          = true;

    m_alpha             = 0.7;
    m_positiveColor     = "#609FF2";  // blueish?
    m_neutralColor      = "#14CE24";  // greenish?
    m_negativeColor     = "#EF5F5F";  // redish?
    m_disabledColor     = "#dddddd";
    m_disabledTextColor = "#aaaaaa";
}

void MixerNode::addEdge(Edge *edge)
{
    m_edgeList << edge;
    edge->adjust();
}

QList<Edge *> MixerNode::edges() const
{
    return m_edgeList;
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

    painter->setFont(m_graph->font());
    if (m_drawNode) {
        QRadialGradient gradient(-3, -3, 10);

        QColor color;
        if (value() < 0) {
            color = m_negativeColor;
        } else if (value() == 0) {
            color = m_neutralColor;
        } else {
            color = m_positiveColor;
        }
        color.setAlphaF(m_alpha);

        if (option->state & QStyle::State_Sunken) {
            gradient.setCenter(3, 3);
            gradient.setFocalPoint(3, 3);

            QColor selColor = color.darker();
            gradient.setColorAt(1, selColor.darker());
            gradient.setColorAt(0, selColor);
        } else {
            gradient.setColorAt(0, m_graph->isEnabled() ? color : m_disabledColor);
            gradient.setColorAt(1, m_graph->isEnabled() ? color.darker() : m_disabledColor);
        }
        painter->setBrush(gradient);
        painter->setPen(m_graph->isEnabled() ? QPen(Qt::black, 0) : QPen(m_disabledTextColor));
        painter->drawEllipse(boundingRect());

        if (!m_image.isNull()) {
            painter->drawImage(boundingRect().adjusted(1, 1, -1, -1), m_image);
        }
    }

    if (m_drawText) {
        if (m_graph->isEnabled()) {
            painter->setPen(QPen(m_drawNode ? Qt::white : Qt::black, 0));
        } else {
            painter->setPen(QPen(m_disabledTextColor));
        }

        painter->drawText((value() < 0) ? -10 : -8, 3, text);
    }
}

void MixerNode::verticalMove(bool flag)
{
    m_vertical = flag;
}

double MixerNode::value()
{
    double h     = m_graph->sceneRect().height();
    double ratio = (h - pos().y()) / h;

    return ((m_graph->getMax() - m_graph->getMin()) * ratio) + m_graph->getMin();
}


QVariant MixerNode::itemChange(GraphicsItemChange change, const QVariant &val)
{
    QPointF newPos = val.toPointF();
    double h = m_graph->sceneRect().height();

    switch (change) {
    case ItemPositionChange:
    {
        if (!m_vertical) {
            break;
        }

        // Force node to move vertically
        newPos.setX(pos().x());

        // Stay inside graph
        if (newPos.y() < 0) {
            newPos.setY(0);
        }
        // qDebug() << h << " - " << newPos.y();
        if (newPos.y() > h) {
            newPos.setY(h);
        }

        return newPos;
    }
    case ItemPositionHasChanged:
    {
        foreach(Edge * edge, m_edgeList)
        edge->adjust();

        update();

        m_graph->itemMoved(value());
        break;
    }
    default:
        break;
    }
    ;

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
