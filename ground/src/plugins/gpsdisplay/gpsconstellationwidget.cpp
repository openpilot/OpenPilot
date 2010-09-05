/**
 ******************************************************************************
 *
 * @file       gpsconstellationwidget.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GPSGadgetPlugin GPS Gadget Plugin
 * @{
 * @brief A widget which displays the GPS constellation
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

#include "gpsconstellationwidget.h"

#include <QtGui>
#include <QDebug>



/*
 * Initialize the widget
 */
GpsConstellationWidget::GpsConstellationWidget(QWidget *parent) : QGraphicsView(parent)
{

    // Create a layout, add a QGraphicsView and put the SVG inside.
    // The constellation widget looks like this:
    // |--------------------|
    // |                    |
    // |                    |
    // |     Constellation  |
    // |                    |
    // |                    |
    // |                    |
    // |--------------------|


    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsScene *scene = new QGraphicsScene(this);
    QSvgRenderer *renderer = new QSvgRenderer();
    world = new QGraphicsSvgItem();
    renderer->load(QString(":/gpsgadget/images/gpsEarth.svg"));
    world->setSharedRenderer(renderer);
    world->setElementId("map");
    scene->addItem(world);
    scene->setSceneRect(world->boundingRect());
    setScene(scene);

    // Now create 16 satellite icons which we will move around on the map:
    for (int i=0; i<16;i++) {
        satIcons[i] = new QGraphicsSvgItem();
        satIcons[i]->setSharedRenderer(renderer);
        satIcons[i]->setElementId("sat-notSeen");
        satIcons[i]->setParentItem(world);
        satIcons[i]->hide();
    }

}

GpsConstellationWidget::~GpsConstellationWidget()
{

}

void GpsConstellationWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    fitInView(world, Qt::KeepAspectRatio);
    // Scale, can't use fitInView since that doesn't work until we're shown.
 //   qreal factor = height()/world->boundingRect().height();
//    world->setScale(factor);
 //   setSceneRect(world->boundingRect());

}

void GpsConstellationWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    fitInView(world, Qt::KeepAspectRatio);
}

void GpsConstellationWidget::updateSat(int index, int prn, int elevation, int azimuth, int snr)
{
    if (index >= 16) {
        return; // A bit of error checking never hurts.
    }

    // TODO: add range checking
    satellites[index][0] = prn;
    satellites[index][1] = elevation;
    satellites[index][2] = azimuth;
    satellites[index][3] = snr;

    if (prn) {
        QPointF opd = polarToCoord(elevation,azimuth);
        opd += QPointF(-satIcons[index]->boundingRect().center().x(),
                       -satIcons[index]->boundingRect().center().y());
        satIcons[index]->setTransform(QTransform::fromTranslate(opd.x(),opd.y()), false);
        if (snr)
            satIcons[index]->setElementId("satellite");
        else
            satIcons[index]->setElementId("sat-notSeen");
        satIcons[index]->show();

    } else {
        satIcons[index]->hide();
    }

}

/**
  Converts the elevation/azimuth to X/Y coordinates on the map

  */
QPointF GpsConstellationWidget::polarToCoord(int elevation, int azimuth)
{
    double x;
    double y;
    double rad_elevation;
    double rad_azimuth;


    rad_elevation = M_PI*elevation/180;
    rad_azimuth = M_PI*azimuth/180;

    x = cos(rad_elevation)*sin(rad_azimuth);
    y = -cos(rad_elevation)*cos(rad_azimuth);

    x = world->boundingRect().width()/2 * x;
    y = world->boundingRect().height()/2 * y;

    x = (world->boundingRect().width() / 2) + x;
    y = (world->boundingRect().height() / 2) + y;

    return QPointF(x,y);

}
