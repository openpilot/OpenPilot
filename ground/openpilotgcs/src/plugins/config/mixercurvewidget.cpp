/**
 ******************************************************************************
 *
 * @file       mixercurvewidget.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief A widget which displays an adjustable mixer curve
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

    curveMin=0.0;
    curveMax=1.0;


    setFrameStyle(QFrame::NoFrame);
    setStyleSheet("background:transparent");

    QGraphicsScene *scene = new QGraphicsScene(this);
    QSvgRenderer *renderer = new QSvgRenderer();
    plot = new QGraphicsSvgItem();
    renderer->load(QString(":/configgadget/images/curve-bg.svg"));
    plot->setSharedRenderer(renderer);
    //plot->setElementId("map");
    scene->addItem(plot);
    plot->setZValue(-1);
    scene->setSceneRect(plot->boundingRect());
    setScene(scene);

}

MixerCurveWidget::~MixerCurveWidget()
{

}

/**
  Init curve: create a (flat) curve with a specified number of points.

  If a curve exists already, resets it.
  Points should be between 0 and 1.
  */
void MixerCurveWidget::initCurve(QList<double> points)
{

    if (points.length() < 2)
        return; // We need at least 2 points on a curve!

    // First of all, reset the list
    // TODO: one edge might not get deleted properly, small mem leak maybe...
        foreach (Node *node, nodeList ) {
            QList<Edge*> edges = node->edges();
            foreach(Edge *edge, edges) {
                if (scene()->items().contains(edge))
                        scene()->removeItem(edge);
                else
                    delete edge;
            }
        scene()->removeItem(node);
        delete node;
    }
    nodeList.clear();

    // Create the nodes
    qreal w = plot->boundingRect().width()/(points.length()-1);
    qreal h = plot->boundingRect().height();
    for (int i=0; i<points.length(); i++) {
        Node *node = new Node(this);
        scene()->addItem(node);
        nodeList.append(node);
        double val = points.at(i);
        if (val>curveMax)
                val=curveMax;
        if (val<curveMin)
                val=curveMin;
        val+=curveMin;
        val/=(curveMax-curveMin);
        node->setPos(w*i,h-val*h);
        node->verticalMove(true);
    }

    // ... and link them together:
    for (int i=0; i<(points.length()-1); i++) {
        scene()->addItem(new Edge(nodeList.at(i),nodeList.at(i+1)));
    }

}


/**
  Returns the current curve settings
  */
QList<double> MixerCurveWidget::getCurve() {
    QList<double> list;

    qreal h = plot->boundingRect().height();
    foreach(Node *node, nodeList) {
        list.append(((curveMax-curveMin)*(h-node->pos().y())/h)+curveMin);
    }

    return list;
}

/**
  Setd the current curve settings
  */
void MixerCurveWidget::setCurve(QList<double> points)
{
    if (nodeList.length()<1)
    {
        initCurve(points);
    }
    else
    {
        qreal w = plot->boundingRect().width()/(points.length()-1);
        qreal h = plot->boundingRect().height();
        for (int i=0; i<points.length(); i++) {
            double val = points.at(i);
            if (val>curveMax)
                    val=curveMax;
            if (val<curveMin)
                    val=curveMin;
            val-=curveMin;
            val/=(curveMax-curveMin);
            nodeList.at(i)->setPos(w*i,h-val*h);
        }
    }
}


void MixerCurveWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    fitInView(plot, Qt::KeepAspectRatio);

}

void MixerCurveWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    fitInView(plot, Qt::KeepAspectRatio);
}



void MixerCurveWidget::itemMoved(double itemValue)
{
    QList<double> list = getCurve();
    emit curveUpdated(list, itemValue);
}

void MixerCurveWidget::setMin(double value)
{
    curveMin = value;
}
void MixerCurveWidget::setMax(double value)
{
    curveMax = value;
}
void MixerCurveWidget::setRange(double min, double max)
{
    curveMin = min;
    curveMax = max;
}
