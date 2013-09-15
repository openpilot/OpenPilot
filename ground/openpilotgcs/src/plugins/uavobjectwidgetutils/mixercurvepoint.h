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

#ifndef MIXERCURVEPOINT_H
#define MIXERCURVEPOINT_H

#include <QGraphicsItem>
#include <QColor>
#include <QList>
#include <QFont>
#include "uavobjectwidgetutils_global.h"

class Edge;
class MixerCurveWidget;
QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
QT_END_NAMESPACE


class UAVOBJECTWIDGETUTILS_EXPORT MixerNode : public QObject, public QGraphicsItem {
    Q_OBJECT
                                  Q_INTERFACES(QGraphicsItem)
public:
    MixerNode(MixerCurveWidget *graphWidget);
    void addEdge(Edge *edge);
    QList<Edge *> edges() const;

    enum { Type = UserType + 10 };
    int type() const
    {
        return Type;
    }

    void verticalMove(bool flag);

    void setPositiveColor(QColor color = "#609FF2")
    {
        positiveColor = color;
    }

    void setNegativeColor(QColor color = "#EF5F5F")
    {
        negativeColor = color;
    }

    void setImage(QImage img)
    {
        image = img;
    }

    void setDrawNode(bool draw)
    {
        drawNode = draw;
    }

    void setDrawText(bool draw)
    {
        drawText = draw;
    }

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    double value();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &val);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QList<Edge *> edgeList;
    QPointF newPos;
    MixerCurveWidget *graph;

    QColor positiveColor;
    QColor neutralColor;
    QColor negativeColor;
    QColor disabledColor;
    QColor disabledTextColor;

    QImage image;

    bool vertical;
    bool drawNode;
    bool drawText;
    int index;
};

#endif // MIXERCURVEPOINT_H
