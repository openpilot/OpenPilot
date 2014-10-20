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

    QSvgRenderer *renderer = new QSvgRenderer();
    renderer->load(QString(":/gpsgadget/images/gpsEarth.svg"));

    world = new QGraphicsSvgItem();
    world->setSharedRenderer(renderer);
    world->setElementId("map");

    scene = new QGraphicsScene(this);
    scene->addItem(world);
    scene->setSceneRect(world->boundingRect());
    setScene(scene);

    QFontDatabase::addApplicationFont(":/gpsgadget/font/digital-7_mono.ttf");

    // Now create 'maxSatellites' satellite icons which we will move around on the map:
    for (int i = 0; i < MAX_SATTELITES; i++) {
        satellites[i][0] = 0;
        satellites[i][1] = 0;
        satellites[i][2] = 0;
        satellites[i][3] = 0;

        satIcons[i] = new QGraphicsSvgItem(world);
        satIcons[i]->setSharedRenderer(renderer);
        satIcons[i]->setElementId("sat-notSeen");
        satIcons[i]->hide();

        satTexts[i] = new QGraphicsSimpleTextItem("##", satIcons[i]);
        satTexts[i]->setBrush(QColor("Black"));
        satTexts[i]->setFont(QFont("Digital-7"));
    }
}

GpsConstellationWidget::~GpsConstellationWidget()
{
    delete scene;
    scene = 0;

    // delete renderer;
    // renderer = 0;
}

void GpsConstellationWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    fitInView(world, Qt::KeepAspectRatio);
    // Scale, can't use fitInView since that doesn't work until we're shown.
    // qreal factor = height()/world->boundingRect().height();
// world->setScale(factor);
// setSceneRect(world->boundingRect());
}

void GpsConstellationWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    fitInView(world, Qt::KeepAspectRatio);
}

void GpsConstellationWidget::updateSat(int index, int prn, int elevation, int azimuth, int snr)
{
    if (index >= MAX_SATTELITES) {
        // A bit of error checking never hurts.
        return;
    }

    // TODO: add range checking
    satellites[index][0] = prn;
    satellites[index][1] = elevation;
    satellites[index][2] = azimuth;
    satellites[index][3] = snr;

    if (prn && elevation >= 0) {
        QPointF opd = polarToCoord(elevation, azimuth);
        opd += QPointF(-satIcons[index]->boundingRect().center().x(),
                       -satIcons[index]->boundingRect().center().y());
        satIcons[index]->setTransform(QTransform::fromTranslate(opd.x(), opd.y()), false);

        // Show normal GPS or SBAS (120 - 158 range)
        if (prn > 119 && prn < 159) {
            if (snr) {
                satIcons[index]->setElementId("satellite-sbas");
            } else {
                satIcons[index]->setElementId("sat-sbas-notSeen");
            }
        } else {
            if (snr) {
                satIcons[index]->setElementId("satellite");
            } else {
                satIcons[index]->setElementId("sat-notSeen");
            }
        }
        satIcons[index]->show();

        QRectF iconRect   = satIcons[index]->boundingRect();
        QString prnString = QString().number(prn);
        if (prnString.length() == 1) {
            prnString = "0" + prnString;
        }
        satTexts[index]->setText(prnString);
        QRectF textRect = satTexts[index]->boundingRect();

        // Fixed scale, looks better for numbers 01,11,126...
        qreal scale     = 1.40;

        QTransform matrix;
        matrix.translate(iconRect.width() / 2, iconRect.height() / 2);
        matrix.scale(scale, scale);
        matrix.translate(-textRect.width() / 2, -textRect.height() / 2);
        satTexts[index]->setTransform(matrix, false);
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
    double vect_elevation;
    double rad_azimuth;

    // Vector modulus scaled to circle and angle (azimut)
    vect_elevation = 0.79 - (elevation / 90.00f) * 0.79;
    rad_azimuth    = M_PI * azimuth / 180;

    // Cartesian coordinates
    x = ((world->boundingRect().width() * vect_elevation) / 2) * sin(rad_azimuth);
    y = ((world->boundingRect().height() * vect_elevation) / 2) * -cos(rad_azimuth);

    // Start from center
    x = (world->boundingRect().width() / 2) + x;
    y = (world->boundingRect().height() / 2) + y;

    return QPointF(x, y);
}
