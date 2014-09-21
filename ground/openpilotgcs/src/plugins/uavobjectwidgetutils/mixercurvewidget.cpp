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
MixerCurveWidget::MixerCurveWidget(QWidget *parent) :
    QGraphicsView(parent), m_xAxisTextItem(0), m_yAxisTextItem(0)
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

    m_curveMin = 0.0;
    m_curveMax = 1.0;

    setFrameStyle(QFrame::NoFrame);
    setStyleSheet("background:transparent");
    setRenderHint(QPainter::HighQualityAntialiasing, true);
    QGraphicsScene *scene  = new QGraphicsScene(this);
    QSvgRenderer *renderer = new QSvgRenderer();
    m_plot = new QGraphicsSvgItem();
    renderer->load(QString(":/uavobjectwidgetutils/images/curve-bg.svg"));
    m_plot->setSharedRenderer(renderer);

    scene->addItem(m_plot);
    m_plot->setZValue(-1);

    scene->setSceneRect(m_plot->boundingRect());
    setScene(scene);

    setupXAxisLabel();
    setupYAxisLabel();
    initNodes(MixerCurveWidget::NODE_NUMELEM);
}

MixerCurveWidget::~MixerCurveWidget()
{
    while (!m_nodeList.isEmpty()) {
        delete m_nodeList.takeFirst();
    }

    while (!m_edgeList.isEmpty()) {
        delete m_edgeList.takeFirst();
    }
    if (m_xAxisTextItem) {
        delete m_xAxisTextItem;
        m_xAxisTextItem = NULL;
    }
    if (m_yAxisTextItem) {
        delete m_yAxisTextItem;
        m_yAxisTextItem = NULL;
    }
}

void MixerCurveWidget::setPositiveColor(QString color)
{
    for (int i = 0; i < m_nodeList.count(); i++) {
        MixerNode *node = m_nodeList.at(i);
        node->setPositiveColor(color);
    }
}

void MixerCurveWidget::setNegativeColor(QString color)
{
    for (int i = 0; i < m_nodeList.count(); i++) {
        MixerNode *node = m_nodeList.at(i);
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
    if (m_nodeList.count()) {
        foreach(MixerNode * node, m_nodeList) {
            foreach(Edge * edge, node->edges()) {
                if (edge->sourceNode() == node) {
                    scene()->removeItem(edge);
                    delete edge;
                }
            }
            scene()->removeItem(node);
            delete node;
        }

        m_nodeList.clear();
    }

    // Create the nodes and edges
    MixerNode *prevNode = 0;
    for (int i = 0; i < numPoints; i++) {
        MixerNode *node = new MixerNode(this, m_plot);

        m_nodeList.append(node);
        scene()->addItem(node);

        node->setPos(0, 0);

        if (prevNode) {
            scene()->addItem(new Edge(prevNode, node));
        }

        prevNode = node;
    }
}

void MixerCurveWidget::setupXAxisLabel()
{
    if (!m_xAxisString.isEmpty()) {
        if (m_xAxisTextItem) {
            m_xAxisTextItem->setPlainText(m_xAxisString);
        } else {
            m_xAxisTextItem = new QGraphicsTextItem(m_xAxisString, m_plot);
            scene()->addItem(m_xAxisTextItem);
        }
    }
}

void MixerCurveWidget::setupYAxisLabel()
{
    if (!m_yAxisString.isEmpty()) {
        if (m_yAxisTextItem) {
            m_yAxisTextItem->setPlainText(m_yAxisString);
        } else {
            m_yAxisTextItem = new QGraphicsTextItem(m_yAxisString, m_plot);
            m_yAxisTextItem->setRotation(270);
            scene()->addItem(m_yAxisTextItem);
        }
    }
}

/**
   Returns the current curve settings
 */
QList<double> MixerCurveWidget::getCurve()
{
    QList<double> list;

    foreach(MixerNode * node, m_nodeList) {
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
    m_curveUpdating = true;

    int ptCnt = points->count();
    if (m_nodeList.count() != ptCnt) {
        initNodes(ptCnt);
    }

    double range = m_curveMax - m_curveMin;

    qreal w = m_plot->boundingRect().width() / (ptCnt - 1);
    qreal h = m_plot->boundingRect().height();
    for (int i = 0; i < ptCnt; i++) {
        double val = (points->at(i) < m_curveMin) ? m_curveMin : (points->at(i) > m_curveMax) ? m_curveMax : points->at(i);

        val += range;
        val -= (m_curveMin + range);
        val /= range;

        MixerNode *node = m_nodeList.at(i);
        node->setPos(w * i, h - (val * h));
        node->verticalMove(true);

        node->update();
    }
    m_curveUpdating = false;

    update();

    emit curveUpdated();
}

void MixerCurveWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    positionAxisLabels();
    setSceneRect(scene()->itemsBoundingRect());
    fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void MixerCurveWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    positionAxisLabels();
    setSceneRect(scene()->itemsBoundingRect());
    fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void MixerCurveWidget::changeEvent(QEvent *event)
{
    QGraphicsView::changeEvent(event);

    if (event->type() == QEvent::EnabledChange) {
        foreach(MixerNode * node, m_nodeList) {
            node->update();
        }
    }
}

void MixerCurveWidget::positionAxisLabels()
{
    QRectF rect = m_plot->boundingRect();

    if (m_xAxisTextItem) {
        m_xAxisTextItem->setPos(rect.right() -
                                m_xAxisTextItem->boundingRect().width(), rect.bottom() - 4);
    }

    if (m_yAxisTextItem) {
        m_yAxisTextItem->setPos(rect.left() -
                                m_yAxisTextItem->boundingRect().height(), m_yAxisTextItem->boundingRect().width());
    }
}

void MixerCurveWidget::itemMoved(double itemValue)
{
    Q_UNUSED(itemValue);

    if (!m_curveUpdating) {
        emit curveUpdated();
    }
}

void MixerCurveWidget::setMin(double value)
{
    if (m_curveMin != value) {
        emit curveMinChanged(value);
    }

    m_curveMin = value;
}

void MixerCurveWidget::setMax(double value)
{
    if (m_curveMax != value) {
        emit curveMaxChanged(value);
    }

    m_curveMax = value;
}

double MixerCurveWidget::getMin()
{
    return m_curveMin;
}
double MixerCurveWidget::getMax()
{
    return m_curveMax;
}
double MixerCurveWidget::setRange(double min, double max)
{
    m_curveMin = min;
    m_curveMax = max;
    return m_curveMax - m_curveMin;
}

void MixerCurveWidget::setXAxisLabel(QString label)
{
    m_xAxisString = label;
    setupXAxisLabel();
}

void MixerCurveWidget::setYAxisLabel(QString label)
{
    m_yAxisString = label;
    setupYAxisLabel();
}
