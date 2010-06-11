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
#include "extensionsystem/pluginmanager.h"

MapGadgetWidget::MapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    int size = 256;

    gscButton = 0;	// added by cathy
    uavButton = 0;	// added by cathy

    follow_uav = false;	// added by cathy

    m_mc = new MapControl(QSize(size, size));

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_mc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_mc->setMinimumSize(64, 64);
    m_mc->showScale(true);
    m_mc->showLatLon(true); // added by cathy

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

    addUserControls();

    gscPosition.setX(5.718888888888);	    // longitude
    gscPosition.setY(58.963333333333);	    // latitude

    m_mc->setView(gscPosition);
    m_mc->setZoom(10);
    m_mc->updateRequestNew();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mc);
    setLayout(layout);

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    m_positionActual = PositionActual::GetInstance(objManager);

    // added by cathy .. slot for receiving mouse click Coordinate events
//    connect(m_mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)), this, SLOT(coordinateClicked(const QMouseEvent*, const QPointF)));	// cmoss

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
    if (follow_uav)
    {
	PositionActual::DataFields data = m_positionActual->getData();
	setPosition(QPointF(data.Longitude, data.Latitude));
    }
}

void MapGadgetWidget::setMapProvider(QString provider)
{
    foreach(QString layerName, m_mc->layers())
        m_mc->layer(layerName)->setVisible(layerName == provider);
}

void MapGadgetWidget::resizeEvent(QResizeEvent *event)
{
    m_mc->resize(QSize(width(), height()));
    update();
    QWidget::resizeEvent(event);
}

void MapGadgetWidget::addUserControls()
{   // create the user controls

	// cathy
	gscButton = new QPushButton("GCS");
	gscButton->setMinimumWidth(50);
	gscButton->setMaximumWidth(50);
	gscButton->setToolTip("Jump too ground station control");
	connect(gscButton, SIGNAL(clicked(bool)), this, SLOT(gscButtonClick()));

	// cathy
	uavButton = new QPushButton("UAV");
	uavButton->setMinimumWidth(50);
	uavButton->setMaximumWidth(50);
	uavButton->setCheckable(true);
	uavButton->setToolTip("Stay centered on the UAV");
	connect(uavButton, SIGNAL(clicked(bool)), this, SLOT(uavButtonClick(bool)));

	QPushButton* zoomin = new QPushButton("+");
	zoomin->setMinimumWidth(50);
	zoomin->setMaximumWidth(50);
	zoomin->setToolTip("Zoom in");
	connect(zoomin, SIGNAL(clicked(bool)), m_mc, SLOT(zoomIn()));

	QPushButton* zoomout = new QPushButton("-");
	zoomout->setMinimumWidth(50);
	zoomout->setMaximumWidth(50);
	zoomout->setToolTip("Zoom out");
	connect(zoomout, SIGNAL(clicked(bool)), m_mc, SLOT(zoomOut()));

	// add zoom buttons to the layout of the MapControl
	QVBoxLayout* innerlayout = new QVBoxLayout;
    	innerlayout->setSpacing(3);
	innerlayout->setMargin(2);
	innerlayout->addSpacing(10);
	innerlayout->addWidget(gscButton);
	innerlayout->addWidget(uavButton);
	innerlayout->addSpacing(10);
	innerlayout->addWidget(zoomin);
	innerlayout->addWidget(zoomout);
	innerlayout->addStretch(0);
	m_mc->setLayout(innerlayout);
}

void MapGadgetWidget::gscButtonClick()  // added by cathy
{
    follow_uav = false;
    uavButton->setChecked(follow_uav);

    // jump straight too the GSC location
    setPosition(gscPosition);
}

void MapGadgetWidget::uavButtonClick(bool checked)  // added by cathy
{
    follow_uav = checked;
//    follow_uav = uavButton->isChecked();

    if (follow_uav)
    {
//	PositionActual::DataFields data = m_positionActual->getData();
//	setPosition(QPointF(data.Longitude, data.Latitude));
    }
}

/*
// added by cathy .. comes here when the user mouse clicks on the map
void MapGadgetWidget::coordinateClicked(const QMouseEvent * evnt, const QPointF coordinate) // cmoss
{
    if (evnt->type() == QEvent::MouseButtonPress && evnt->buttons() == Qt::LeftButton)
    {
	QString coord_str = "lat " + QString::number(coordinate.y(), 'f', 6) + ", lon " + QString::number(coordinate.x(), 'f', 6);

	qDebug() << coordinate << ": " << evnt->x() << " / " << evnt->y() << " / " << coord_str;

	QMessageBox::information(this, "Coordinate Clicked", coord_str);
    }
}
*/
// added by cathy .. comes here when the user mouse clicks on an added geometry feature (ie, way points etc)
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

// added by cathy
void MapGadgetWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) // ESC
    {
//	emit(close());
    }
    else
    if (event->key() == Qt::Key_F1) // F1
    {
	gscButtonClick();
    }
    else
    if (event->key() == Qt::Key_F2) // F2
    {
	follow_uav = !follow_uav;
	uavButton->setChecked(follow_uav);
	uavButtonClick(follow_uav);
    }
    else
    if (event->key() == Qt::Key_Up)
    {
    }
    else
    if (event->key() == Qt::Key_Down)
    {
    }
    else
    if (event->key() == Qt::Key_Left)
    {
    }
    else
    if (event->key() == Qt::Key_Right)
    {
    }
    else
    if (event->key() == Qt::Key_PageUp)
    {
	m_mc->zoomIn();
    }
    else
    if (event->key() == Qt::Key_PageDown)
    {
	m_mc->zoomOut();
    }
    else
    {
	qDebug() << event->key() << endl;
    }
}
