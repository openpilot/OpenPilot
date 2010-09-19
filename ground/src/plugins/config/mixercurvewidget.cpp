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

    Node *node1 = new Node(this);
    Node *node2 = new Node(this);
    Node *node3 = new Node(this);
    Node *node4 = new Node(this);
    Node *node5 = new Node(this);
    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(node3);
    scene->addItem(node4);
    scene->addItem(node5);

    scene->addItem(new Edge(node1, node2));
    scene->addItem(new Edge(node2, node3));
    scene->addItem(new Edge(node3, node4));
    scene->addItem(new Edge(node4, node5));

    qreal w = plot->boundingRect().width()/4;

    node1->setPos(0,50);
    node2->setPos(w, 50);
    node3->setPos(2*w, 50);
    node4->setPos(3*w, 50);
    node5->setPos(4*w, 50);

    node1->verticalMove(true);
    node2->verticalMove(true);
    node3->verticalMove(true);
    node4->verticalMove(true);
    node5->verticalMove(true);

}

MixerCurveWidget::~MixerCurveWidget()
{

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



void MixerCurveWidget::itemMoved()
{
 qDebug() << "Moved ball";
}
