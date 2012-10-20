/**
 ******************************************************************************
 *
 * @file       mixercurveline.h
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

/*
 * This is inspired by the elastic nodes and edges demo in the Qt examples
 * I have reinventing the wheel!
 */

#ifndef MIXERCURVELINE_H
#define MIXERCURVELINE_H

#include <QGraphicsItem>

class MixerNode;

class Edge : public QGraphicsItem
{
public:
    Edge(MixerNode *sourceNode, MixerNode *destNode);
    ~Edge();

    MixerNode *sourceNode() const;
    void setSourceNode(MixerNode *node);

    MixerNode *destNode() const;
    void setDestNode(MixerNode *node);

    void adjust();

    enum { Type = UserType + 12 };
    int type() const { return Type; }
    
protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    
private:
    MixerNode *source, *dest;

    QPointF sourcePoint;
    QPointF destPoint;
    qreal arrowSize;
};

#endif // MIXERCURVELINE_H

