/**
 ******************************************************************************
 *
 * @file       mixercurvewidget.cpp
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

#include "mixercurvewidget.h"
#include "mixercurveline.h"
#include "mixercurvepoint.h"

#include <QObject>
#include <QtGui>
#include <QDebug>

/*
 * Initialize the widget
 */
MixerCurveWidget::MixerCurveWidget(QWidget *parent) : QGraphicsView(parent)
{
    // Create a layout, add a QGraphicsView and put the SVG inside.
    // The Mixer Curve widget looks like this:
    // |--------------------|
    // |                    |
    // |                    |
    // |     Graph  |
    // |                    |
    // |                    |
    // |                    |
    // |--------------------|


    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);

    curveMin = 0.0;
    curveMax = 1.0;

    setFrameStyle(QFrame::NoFrame);
    setStyleSheet("background:transparent");

    QGraphicsScene *scene  = new QGraphicsScene(this);
    QSvgRenderer *renderer = new QSvgRenderer();
    plot = new QGraphicsSvgItem();
    renderer->load(QString(":/uavobjectwidgetutils/images/curve-bg.svg"));
    plot->setSharedRenderer(renderer);

    scene->addItem(plot);
    plot->setZValue(-1);

    scene->setSceneRect(plot->boundingRect());
    setScene(scene);

    initNodes(MixerCurveWidget::NODE_NUMELEM);
}

MixerCurveWidget::~MixerCurveWidget()
{
    while (!nodeList.isEmpty()) {
        delete nodeList.takeFirst();
    }

    while (!edgeList.isEmpty()) {
        delete edgeList.takeFirst();
    }
}

void MixerCurveWidget::setPositiveColor(QString color)
{
    for (int i = 0; i < nodeList.count(); i++) {
        MixerNode *node = nodeList.at(i);
        node->setPositiveColor(color);
    }
}

void MixerCurveWidget::setNegativeColor(QString color)
{
    for (int i = 0; i < nodeList.count(); i++) {
        MixerNode *node = nodeList.at(i);
        node->setNegativeColor(color);
    }
}

/**
   Init curve: create a (flat) curve with a specified number of points.

   If a curve exists already, resets it.
   Points should be between 0 and 1.
 */
void MixerCurveWidget::initCurve(const QList<double> *points)
{
    if (points->length() < 2) {
        return; // We need at least 2 points on a curve!
    }
    // finally, set node positions
    setCurve(points);
}

void MixerCurveWidget::initNodes(int numPoints)
{
    // First of all, clear any existing list
    if (nodeList.count()) {
        foreach(MixerNode * node, nodeList) {
            foreach(Edge * edge, node->edges()) {
                if (edge->sourceNode() == node) {
                    scene()->removeItem(edge);
                    delete edge;
                }
            }
            scene()->removeItem(node);
            delete node;
        }

        nodeList.clear();
    }

    // Create the nodes and edges
    MixerNode *prevNode = 0;
    for (int i = 0; i < numPoints; i++) {
        MixerNode *node = new MixerNode(this);

        nodeList.append(node);
        scene()->addItem(node);

        node->setPos(0, 0);

        if (prevNode) {
            scene()->addItem(new Edge(prevNode, node));
        }

        prevNode = node;
    }
}

/**
   Returns the current curve settings
 */
QList<double> MixerCurveWidget::getCurve()
{
    QList<double> list;

    foreach(MixerNode * node, nodeList) {
        list.append(node->value());
    }

    return list;
}
/**
   Sets a linear graph
 */
void MixerCurveWidget::initLinearCurve(int numPoints, double maxValue, double minValue)
{
    double range = maxValue - minValue; // setRange(minValue, maxValue);

    QList<double> points;
    for (double i = 0; i < (double)numPoints; i++) {
        double val = (range * (i / (double)(numPoints - 1))) + minValue;
        points.append(val);
    }
    initCurve(&points);
}
/**
   Setd the current curve settings
 */
void MixerCurveWidget::setCurve(const QList<double> *points)
{
    curveUpdating = true;

    int ptCnt = points->count();
    if (nodeList.count() != ptCnt) {
        initNodes(ptCnt);
    }

    double range = curveMax - curveMin;

    qreal w = plot->boundingRect().width() / (ptCnt - 1);
    qreal h = plot->boundingRect().height();
    for (int i = 0; i < ptCnt; i++) {
        double val = (points->at(i) < curveMin) ? curveMin : (points->at(i) > curveMax) ? curveMax : points->at(i);

        val += range;
        val -= (curveMin + range);
        val /= range;

        MixerNode *node = nodeList.at(i);
        node->setPos(w * i, h - (val * h));
        node->verticalMove(true);

        node->update();
    }
    curveUpdating = false;

    update();

    emit curveUpdated();
}


void MixerCurveWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.

    QRectF rect = plot->boundingRect();
    fitInView(rect.adjusted(-12, -12, 12, 12), Qt::KeepAspectRatio);
}

void MixerCurveWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    QRectF rect = plot->boundingRect();
    fitInView(rect.adjusted(-12, -12, 12, 12), Qt::KeepAspectRatio);
}

void MixerCurveWidget::changeEvent(QEvent *event)
{
    QGraphicsView::changeEvent(event);

    if (event->type() == QEvent::EnabledChange) {
        foreach(MixerNode * node, nodeList) {
            node->update();
        }
    }
}

void MixerCurveWidget::itemMoved(double itemValue)
{
    Q_UNUSED(itemValue);

    if (!curveUpdating) {
        emit curveUpdated();
    }
}

void MixerCurveWidget::setMin(double value)
{
    if (curveMin != value) {
        emit curveMinChanged(value);
    }

    curveMin = value;
}

void MixerCurveWidget::setMax(double value)
{
    if (curveMax != value) {
        emit curveMaxChanged(value);
    }

    curveMax = value;
}

double MixerCurveWidget::getMin()
{
    return curveMin;
}
double MixerCurveWidget::getMax()
{
    return curveMax;
}
double MixerCurveWidget::setRange(double min, double max)
{
    curveMin = min;
    curveMax = max;
    return curveMax - curveMin;
}
