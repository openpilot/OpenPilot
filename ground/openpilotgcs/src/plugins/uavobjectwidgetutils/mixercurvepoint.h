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

class Edge;
class MixerCurveWidget;
QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
QT_END_NAMESPACE

class MixerNode : public QObject,public QGraphicsItem
{
    Q_OBJECT
public:
    MixerNode(MixerCurveWidget *graphWidget);
    void addEdge(Edge *edge);
    QList<Edge *> edges() const;

    enum { Type = UserType + 10 };
    int type() const { return Type; }

    void setName(QString name) { cmdName = name; }
    const QString& getName() { return cmdName; }

    void verticalMove(bool flag);
    void commandNode(bool enable);
    void commandText(QString text);
    int  getCommandIndex() { return index; }
    void setCommandIndex(int idx) { index = idx; }

    bool getCommandActive() { return cmdActive; }
    void setCommandActive(bool enable) { cmdActive = enable; }
    void setToggle(bool enable) { cmdToggle = enable; }
    bool getToggle() { return cmdToggle; }

    void setPositiveColor(QString color0 = "#00ff00", QString color1 = "#00ff00") { posColor0 = color0;  posColor1 = color1; }
    void setNegativeColor(QString color0 = "#ff0000", QString color1 = "#ff0000") { negColor0 = color0; negColor1 = color1; }
    void setImage(QImage img) { image = img; }
    void setDrawNode(bool draw) { drawNode = draw; }
    void setDrawText(bool draw) { drawText = draw; }

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    double value();

signals:
   void commandActivated(QString text);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &val);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    
private:
    QList<Edge *>       edgeList;
    QPointF             newPos;
    MixerCurveWidget*   graph;
    QString             posColor0;
    QString             posColor1;
    QString             negColor0;
    QString             negColor1;
    QImage              image;

    bool    vertical;
    QString cmdName;
    bool    cmdActive;
    bool    cmdNode;
    bool    cmdToggle;
    QString cmdText;    
    bool    drawNode;
    bool    drawText;
    int     index;

};

#endif  // MIXERCURVEPOINT_H
