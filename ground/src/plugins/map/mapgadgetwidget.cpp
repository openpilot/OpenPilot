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
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include "extensionsystem/pluginmanager.h"

MapGadgetWidget::MapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    follow_uav = false;	// cmoss

    int size = 256;

    m_mc = new MapControl(QSize(size, size));

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_mc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_mc->setMinimumSize(64, 64);
    m_mc->showScale(true);

    m_osmAdapter = new OSMMapAdapter();
    m_googleSatAdapter = new GoogleSatMapAdapter();
    m_googleAdapter = new GoogleMapAdapter();
    m_yahooAdapter = new YahooMapAdapter();

    m_osmLayer = new MapLayer("OpenStreetMap", m_osmAdapter);
    m_googleLayer = new MapLayer("Google", m_googleAdapter);
    m_googleSatLayer = new MapLayer("Google Sat", m_googleSatAdapter);
    m_yahooLayer = new MapLayer("Yahoo", m_yahooAdapter);

    m_osmLayer->setVisible(true);
    m_googleLayer->setVisible(false);
    m_googleSatLayer->setVisible(false);
    m_yahooLayer->setVisible(false);

    m_mc->addLayer(m_osmLayer);
    m_mc->addLayer(m_googleLayer);
    m_mc->addLayer(m_googleSatLayer);
    m_mc->addLayer(m_yahooLayer);









    // *********************************
    // add a geometry layer .. cmoss

//    m_customLayer = new MapLayer("Custom Layer", m_osmAdapter);	// create layer

    // create a LineString
    QList<Point*> way_points;

    // Image points
//    way_points.append(new ImagePoint(-2.463293, 52.647245, "images/plane.png", "QuadCopter 1", Point::BottomLeft));

    // Circle points
    QPen *pointpen = new QPen(QColor(255, 128, 64)); pointpen->setWidth(2);
    way_points.append(new CirclePoint(-2.463293, 52.647245, 15, "WP1", Point::Middle, pointpen));
    way_points.append(new CirclePoint(-2.465000, 52.647240, 15, "WP2", Point::Middle, pointpen));
    way_points.append(new CirclePoint(-2.468000, 52.649240, 15, "WP3", Point::Middle, pointpen));
    way_points.append(new CirclePoint(-2.470000, 52.649300, 15, "WP4", Point::Middle, pointpen));

    // A QPen also can use transparency
//    QPen *linepen = new QPen(QColor(255, 255, 255, 128)); linepen->setWidth(3);
    QPen *linepen = new QPen(QColor(255, 255, 255)); linepen->setWidth(1);
    // Add the Points and the QPen to a LineString
    LineString *ls = new LineString(way_points, "GSC", linepen);

//    m_customLayer->setVisible(true);	// make it visible
//    m_customLayer->addGeometry(ls);	// Add the LineString to the layer
//    connect(m_customLayer, SIGNAL(geometryClicked(Geometry*, QPoint)), this, SLOT(geometryClicked(Geometry*, QPoint)));	// Connect click events of the layer to this object
//    m_mc->addLayer(m_customLayer);					// add the custom layer into the system

    m_googleSatLayer->addGeometry(ls);	// Add the LineString to the layer

//    m_googleSatLayer->clearGeometries();

    // *********************************













    addZoomButtons();
    m_mc->setView(QPointF(5.718888888888, 58.963333333333));
    m_mc->setZoom(10);
    m_mc->updateRequestNew();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mc);
    setLayout(layout);

//    connect(m_mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)), this, SLOT(coordinateClicked(const QMouseEvent*, const QPointF)));	// cmoss

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    m_positionActual = PositionActual::GetInstance(objManager);

    m_updateTimer = new QTimer();
    m_updateTimer->setInterval(250);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));
    m_updateTimer->start();
}

MapGadgetWidget::~MapGadgetWidget()
{
   // Do nothing
}
void MapGadgetWidget::setZoom(int value)
{
    m_mc->setZoom(value);
    m_mc->updateRequestNew();
}

void MapGadgetWidget::setPosition(QPointF pos)
{
    m_mc->setView(pos);
    m_mc->updateRequestNew();
}

void MapGadgetWidget::updatePosition()
{
    PositionActual::DataFields data = m_positionActual->getData();

    if (follow_uav)
    {
//    setPosition(QPointF(data.Longitude, data.Latitude));  	// cmoss
    }

}

void MapGadgetWidget::setMapProvider(QString provider)
{
//    for (int i = 0; i < m_mc->layers().size(); i++)
//    {
//	Layer *layer = m_mc->layers().at(i);
//	QString layerName = layer->layername();
//
//	if (layerName == "Custom Layer") continue;    // cmoss .. leave the custom layer visible
//	layer->setVisible(layerName == provider);
//    }

    foreach (QString layerName, m_mc->layers())
    {
	m_mc->layer(layerName)->setVisible(layerName == provider);

//	QMessageBox::information(this, provider, layerName);
//	m_mc->layer(layerName)->setVisible(layerName == provider || layerName == QString("Custom Layer"));   // cmoss
    }
}


void MapGadgetWidget::resizeEvent(QResizeEvent *event)
{
    m_mc->resize(QSize(width(), height()));
    update();
    QWidget::resizeEvent(event);
}

void MapGadgetWidget::addZoomButtons()
{
	// create buttons as controls for zoom
	QPushButton *zoomin = new QPushButton("+");
	QPushButton *zoomout = new QPushButton("-");
	QPushButton *gcs = new QPushButton("GCS");	// cmoss
	QPushButton *uav = new QPushButton("UAV");	// cmoss

	// cmoss
	gcs->setMaximumWidth(50);
	gcs->setToolTip("Go to the ground station");

	uav->setMaximumWidth(50);
	uav->setCheckable(true);
	uav->setToolTip("Follow the UAV");

	zoomin->setMinimumSize(50, 50);
	zoomin->setMaximumSize(50, 50);
//	zoomin->setMaximumWidth(50);

	zoomout->setMinimumSize(50, 50);
	zoomout->setMaximumSize(50, 50);
//	zoomout->setMaximumWidth(50);

	connect(zoomin, SIGNAL(clicked(bool)), m_mc, SLOT(zoomIn()));
	connect(zoomout, SIGNAL(clicked(bool)), m_mc, SLOT(zoomOut()));

	// add zoom buttons to the layout of the MapControl
	QVBoxLayout* innerlayout = new QVBoxLayout;
//	QHBoxLayout *innerlayout = new QHBoxLayout;

    	innerlayout->setSpacing(3);
	innerlayout->setMargin(2);

	innerlayout->addSpacing(10);
	innerlayout->addWidget(gcs);
	innerlayout->addWidget(uav);
	innerlayout->addSpacing(10);
	innerlayout->addWidget(zoomin);
	innerlayout->addWidget(zoomout);

	innerlayout->addStretch(0);

	m_mc->setLayout(innerlayout);
}

void MapGadgetWidget::coordinateClicked(const QMouseEvent * evnt, const QPointF coordinate) // cmoss
{
    if (evnt->type() == QEvent::MouseButtonPress && evnt->buttons() == Qt::LeftButton)
    {
	QString coord_str = "lat " + QString::number(coordinate.y(), 'f', 6) + ", lon " + QString::number(coordinate.x(), 'f', 6);

	qDebug() << coordinate << ": " << evnt->x() << " / " << evnt->y() << " / " << coord_str;

	QMessageBox::information(this, "Coordinate Clicked", coord_str);
    }
}

void MapGadgetWidget::geometryClicked(Geometry* geom, QPoint)	// cmoss
{
	qDebug() << "parent: " << geom->parentGeometry();
	qDebug() << "Element clicked: " << geom->name();
	if (geom->hasClickedPoints())
	{
		QList<Geometry*> pp = geom->clickedPoints();
		qDebug() << "number of child elements: " << pp.size();
		for (int i=0; i<pp.size(); i++)
		{
			QMessageBox::information(this, geom->name(), pp.at(i)->name());
		}
	}
	else
	if (geom->GeometryType == "Point")
	{
		QMessageBox::information(this, geom->name(), "just a point");
	}
}

void MapGadgetWidget::keyPressEvent(QKeyEvent* evnt)	// cmoss
{
	if (evnt->key() == 49 || evnt->key() == 17825792)  // tastatur '1'
	{
		m_mc->zoomIn();
	}
	else
	if (evnt->key() == 50)
	{
		m_mc->moveTo(QPointF(8.25, 60));
	}
	else
	if (evnt->key() == 51 || evnt->key() == 16777313)     // tastatur '3'
	{
		m_mc->zoomOut();
	}
	else
	if (evnt->key() == 54) // 6
	{
		m_mc->setView(QPointF(8,50));
	}
	else
	if (evnt->key() == 16777234) // left
	{
		m_mc->scrollLeft();
	}
	else
	if (evnt->key() == 16777236) // right
	{
		m_mc->scrollRight();
	}
	else
	if (evnt->key() == 16777235 ) // up
	{
		m_mc->scrollUp();
	}
	else
	if (evnt->key() == 16777237) // down
	{
		m_mc->scrollDown();
	}
	else
	if (evnt->key() == Qt::Key_Escape) // ESC
	{
		emit(close());
	}
	else
	{
		qDebug() << evnt->key() << endl;
	}
}
