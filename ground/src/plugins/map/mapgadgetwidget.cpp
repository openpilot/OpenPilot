/**
 ******************************************************************************
 *
 * @file       mapgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   map
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
#include "mapgadgetwidget.h"
#include <QStringList>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

MapGadgetWidget::MapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    int size = 256;
    mc = new MapControl(QSize(size, size));
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mc->setMinimumSize(64, 64);
    mc->showScale(true);
    mapadapter = new OSMMapAdapter();
    mainlayer = new MapLayer("OpenStreetMap-Layer", mapadapter);
    mc->addLayer(mainlayer);

    addZoomButtons();
    mc->setView(QPointF(5.718888888888, 58.963333333333));
    mc->setZoom(10);
    mc->updateRequestNew();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mc);
    setLayout(layout);
}

MapGadgetWidget::~MapGadgetWidget()
{
   // Do nothing
}
void MapGadgetWidget::setZoom(int value)
{
    mc->setZoom(value);
    mc->updateRequestNew();
}

void MapGadgetWidget::setPosition(QPointF pos)
{
    mc->setView(pos);
    mc->updateRequestNew();
}


void MapGadgetWidget::resizeEvent(QResizeEvent *event)
{
    mc->resize(QSize(width(), height()));
    update();
    QWidget::resizeEvent(event);
}

void MapGadgetWidget::addZoomButtons()
{
	// create buttons as controls for zoom
	QPushButton* zoomin = new QPushButton("+");
	QPushButton* zoomout = new QPushButton("-");
	zoomin->setMaximumWidth(50);
	zoomout->setMaximumWidth(50);

	connect(zoomin, SIGNAL(clicked(bool)),
			mc, SLOT(zoomIn()));
	connect(zoomout, SIGNAL(clicked(bool)),
			mc, SLOT(zoomOut()));

	// add zoom buttons to the layout of the MapControl
	QVBoxLayout* innerlayout = new QVBoxLayout;
	innerlayout->addWidget(zoomin);
	innerlayout->addWidget(zoomout);
	mc->setLayout(innerlayout);
}

