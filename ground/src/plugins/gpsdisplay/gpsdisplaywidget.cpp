/**
 ******************************************************************************
 *
 * @file       gpsdisplaywidget.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @brief      GPS Display widget, does the actual drawing
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   gpsdisplayplugin
 * @{
 *
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

#include "gpsdisplaywidget.h"
#include "ui_gpsdisplaywidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"

#include <iostream>
#include <QtGui/QFileDialog>
#include <QDebug>

/*
 * Initialize the widget
 */
GpsDisplayWidget::GpsDisplayWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(128,128);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
 
    widget = new Ui_GpsDisplayWidget();
    widget->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(this);
    QSvgRenderer *renderer = new QSvgRenderer();
    QGraphicsSvgItem *world = new QGraphicsSvgItem();
    renderer->load(QString(":/gpsgadget/images/gpsEarth.svg"));
    world->setSharedRenderer(renderer);
    world->setElementId("map");
    scene->addItem(world);
    scene->setSceneRect(world->boundingRect());
    widget->gpsWorld->setScene(scene);
    // Somehow fitInView does not work there at all? Makes
    // the 'world' element tiny tiny tiny. anyone knows why??
    //widget->gpsWorld->fitInView(world,Qt::KeepAspectRatio);
    qreal factor = widget->gpsWorld->size().height()/world->boundingRect().height();
    widget->gpsWorld->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    world->setScale(factor);

}

GpsDisplayWidget::~GpsDisplayWidget()
{
   delete widget;
}


void GpsDisplayWidget::connectButtonClicked() {
}



