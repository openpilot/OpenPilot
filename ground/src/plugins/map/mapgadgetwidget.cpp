/*
 * mapgadgetwidget.cpp
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
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

